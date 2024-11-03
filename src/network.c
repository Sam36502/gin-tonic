#include "../include/network.h"


// Internal Globals
static char __ip_stringbuf[32];


void Net_Init() {
	if (SDLNet_Init() != 0) {
		Net_Log_Message(LOG_ERROR, "Failed to initialise Networking");
		return;
	}
}

void Net_Term() {
	SDLNet_Quit();
}

char *Net_FormatIP(IPaddress ip) {
	Uint8 bytes[4];
	for (int i=0; i<4; i++)
		bytes[i] = ip.host >> (i*8) & 0xFF;
	int flipped_port = (ip.port & 0xFF);
	flipped_port <<= 8;
	flipped_port |= (ip.port>>8);
	
	SDL_snprintf(__ip_stringbuf, 32, "%i.%i.%i.%i:%i", bytes[0], bytes[1], bytes[2], bytes[3], flipped_port);

	return __ip_stringbuf;
}

void Net_Log_Message(Log_Level lvl, char *msg) {
	char buf[256];
	SDL_snprintf(buf, 256, "%s: %s", msg, SDLNet_GetError());
	Log_Message(lvl, buf);
}

void Net_SendBlock(TCPsocket sock, Datablock *data) {
	if (sock == NULL || data == NULL) return;
	
	size_t buflen = data->block_length + 8;
	Uint8 buf[buflen];
	Datablock_WriteToBytes(buf, buflen, data);
	SDLNet_TCP_Send(sock, buf, buflen);
}


//	
//		Server functions
//	

int __ServerThread(void *_srv) {
	Net_Server *srv = (Net_Server *) _srv;
	Uint8 *data_buffer = SDL_malloc(NET_BUFFER_SIZE);

	srv->state = NET_STATE_GOOD;

	while (srv->state == NET_STATE_GOOD) {
		SDL_LockMutex(srv->mutex);

		if (srv->sock == NULL) {
			srv->state = NET_STATE_TERM;
			SDL_UnlockMutex(srv->mutex);
			break;
		}

		// Check if anyone wants to connect
		TCPsocket client_sock = NULL;
		client_sock = SDLNet_TCP_Accept(srv->sock);
		if (client_sock != NULL) {
			srv->client_socks[srv->client_count++] = client_sock;
			SDLNet_TCP_AddSocket(srv->socketset, client_sock);

			Events_GTEvent_Push(UEVENT_NET_CONNECT, client_sock, NULL);
		}

		// Receive data from clients
		if (srv->client_count > 0) {
			int ready_socks = SDLNet_CheckSockets(srv->socketset, NET_SOCKET_TIMEOUT);
			if (ready_socks < 0) {
				Net_Log_Message(LOG_WARNING, "Failed to check client sockets");
				continue;
			}

			if (ready_socks > 0) {
				for (int i=0; i<srv->client_count; i++) {
					TCPsocket sock = srv->client_socks[i];
					if (SDLNet_SocketReady(sock) == 0) continue;

					int bytes_received = SDLNet_TCP_Recv(sock, data_buffer, NET_BUFFER_SIZE);
					if (bytes_received < 0) continue;

					// Client Disconnected
					if (bytes_received == 0) {
						IPaddress *addr = SDLNet_TCP_GetPeerAddress(sock);
						Events_GTEvent_Push(UEVENT_NET_DISCONNECT, addr, NULL);

						Net_Server_DisconnectClient(srv, i);
						i--; // Compensate for removed client
						continue;
					}

					// TODO: This can return multiple blocks?
					Datablock *db = Datablock_ParseFromBytes(data_buffer, bytes_received);
					Events_GTEvent_Push(UEVENT_NET_RECEIVE, sock, db);
				}
			}

		}

		SDL_UnlockMutex(srv->mutex);
	}

	srv->state = NET_STATE_OFFL;
	SDL_free(data_buffer);
	return 0;
}

Net_Server *Net_Server_Start(Uint16 port, Uint16 capacity) {
	Net_Server *srv = SDL_malloc(sizeof(Net_Server));

	SDLNet_ResolveHost(&srv->ip, NULL, port);
	srv->capacity = capacity;
	srv->client_count = 0;
	srv->state = NET_STATE_INIT;
	srv->client_socks = SDL_malloc(sizeof(TCPsocket) * capacity);
	srv->socketset = SDLNet_AllocSocketSet(capacity);
	if (srv->socketset == NULL) {
		Net_Log_Message(LOG_ERROR, "Failed to allocate Socket-Set");
		Net_Server_Stop(srv);
		return NULL;
	}

	srv->mutex = SDL_CreateMutex();
	if (srv->mutex == NULL) {
		Log_SDLMessage(LOG_ERROR, "Failed to Create Server Mutex");
		Net_Server_Stop(srv);
		return NULL;
	}

	srv->sock = SDLNet_TCP_Open(&srv->ip);
	if (srv->sock == NULL) {
		Net_Log_Message(LOG_ERROR, "Failed to open Server Socket");
		Net_Server_Stop(srv);
		return NULL;
	}

	srv->thread = SDL_CreateThread(__ServerThread, NET_THREAD_NAME_SERVER, srv);
	if (srv->thread == NULL) {
		Log_SDLMessage(LOG_ERROR, "Failed to Create Server Thread");
		Net_Server_Stop(srv);
		return NULL;
	}

	return srv;
}

void Net_Server_Stop(Net_Server *srv) {
	if (srv == NULL) return;

	if (srv->mutex != NULL) {
		SDL_LockMutex(srv->mutex);

		// Disconnect all clients
		Net_Server_DisconnectClient(srv, -1);

		SDLNet_TCP_Close(srv->sock);
		srv->sock = NULL;
		srv->state = NET_STATE_TERM;

		SDL_UnlockMutex(srv->mutex);
		SDL_WaitThread(srv->thread, NULL);

		SDL_DestroyMutex(srv->mutex);
	}

	SDLNet_FreeSocketSet(srv->socketset);

	SDL_free(srv);
}

int Net_Server_GetClientIndex(Net_Server *srv, TCPsocket client_sock) {
	if (srv == NULL || client_sock == NULL) return -1;

	// Find client index
	for (int i=0; i<srv->client_count; i++) {
		if (client_sock == srv->client_socks[i]) {
			return i;
		}
	}

	return -1;
}

void Net_Server_DisconnectClient(Net_Server *srv, int client_index) {
	if (srv == NULL || client_index >= srv->client_count) return;

	// Remove client from list & Disconnect
	if (client_index >= 0) {
		TCPsocket client_sock = srv->client_socks[client_index];

		SDLNet_TCP_DelSocket(srv->socketset, client_sock);
		SDLNet_TCP_Close(client_sock);

		int last_id = srv->client_count - 1;
		srv->client_socks[client_index] = srv->client_socks[last_id];
		srv->client_socks[last_id] = NULL;

		srv->client_count--;
		return;
	}

	// index <0; Disconnect all
	for (int i=0; i<srv->client_count; i++) {
		TCPsocket client_sock = srv->client_socks[i];

		SDLNet_TCP_DelSocket(srv->socketset, client_sock);
		SDLNet_TCP_Close(client_sock);
		srv->client_socks[i] = NULL;
	}
	srv->client_count = 0;
}

void Net_Server_Send(Net_Server *srv, int client_index, Datablock *data) {
	if (srv == NULL || data == NULL) return;
	if (client_index >= srv->client_count) return;

	if (client_index >= 0) {
		Net_SendBlock(srv->client_socks[client_index], data);
		return;
	}

	// index <0; Broadcasat
	for (int i=0; i<srv->client_count; i++) {
		Net_SendBlock(srv->client_socks[i], data);
	}
}


//	
//		Client functions
//	

int __Connection_Thread(void *_conn) {
	Net_Conn *conn = (Net_Conn *) _conn;
	Uint8 *data = SDL_malloc(NET_BUFFER_SIZE);

	// Connect to server
	SDL_LockMutex(conn->mutex);
	SDLNet_SocketSet sockset = SDLNet_AllocSocketSet(1);
	conn->sock = SDLNet_TCP_Open(&conn->ip);
	if (conn->sock == NULL) {
		// TODO: Let user decide what to log; set error code and fail silently
		conn->state = NET_STATE_TERM;
		Net_Log_Message(LOG_WARNING, "Failed to connect to server");
	} else {
		SDLNet_TCP_AddSocket(sockset, conn->sock);
		Events_GTEvent_Push(UEVENT_NET_CONNECT, conn->sock, NULL);
		conn->state = NET_STATE_GOOD;
	}
	SDL_UnlockMutex(conn->mutex);

	while (conn->state == NET_STATE_GOOD) {
		SDL_LockMutex(conn->mutex);

		if (conn->sock == NULL) {
			SDL_UnlockMutex(conn->mutex);
			conn->state = NET_STATE_TERM;
			break;
		}

		int chk = SDLNet_CheckSockets(sockset, NET_SOCKET_TIMEOUT);
		if (chk < 0) {
			Net_Log_Message(LOG_WARNING, "Failed to check client sockets");
			SDL_UnlockMutex(conn->mutex);
			continue;
		}

		// Check for any packets from the server
		if (chk > 0 && SDLNet_SocketReady(conn->sock)) {
			int resp_len = SDLNet_TCP_Recv(conn->sock, data, NET_BUFFER_SIZE);
			if (resp_len < 0) Net_Log_Message(LOG_WARNING, "Failed to receive data from connection");

			// Connection Terminated
			if (resp_len == 0) {
				IPaddress *addr = SDLNet_TCP_GetPeerAddress(conn->sock);
				Events_GTEvent_Push(UEVENT_NET_DISCONNECT, addr, NULL);
				SDLNet_TCP_Close(conn->sock);
				conn->sock = NULL;

				SDL_UnlockMutex(conn->mutex);
				break;
			}

			Datablock *db = Datablock_ParseFromBytes(data, resp_len);
			Events_GTEvent_Push(UEVENT_NET_RECEIVE, conn->sock, db);
		}

		SDL_UnlockMutex(conn->mutex);
	}

	SDLNet_FreeSocketSet(sockset);

	SDL_free(data);

	conn->state = NET_STATE_OFFL;
	return 0;
}

Net_Conn *Net_Connect(IPaddress ip) {
	Net_Conn *conn = SDL_malloc(sizeof(Net_Conn));
	conn->ip = ip;
	conn->state = NET_STATE_INIT;
	conn->mutex = SDL_CreateMutex();
	if (conn->mutex == NULL) {
		Log_SDLMessage(LOG_ERROR, "Failed to Create Client Mutex");
		Net_Disconnect(conn);
		return NULL;
	}

	conn->thread = SDL_CreateThread(__Connection_Thread, NET_THREAD_NAME_CONNECTION, (void *) conn);
	if (conn->thread == NULL) {
		Log_SDLMessage(LOG_ERROR, "Failed to Create Client Thread");
		Net_Disconnect(conn);
		return NULL;
	}

	return conn;
}

void Net_Disconnect(Net_Conn *conn) {
	if (conn == NULL) return;
	
	// Stop Client thread
	if (conn->mutex != NULL) {
		
		if (conn->sock != NULL) {
			SDL_LockMutex(conn->mutex);

			conn->state = NET_STATE_TERM;
			SDLNet_TCP_Close(conn->sock);
			conn->sock = NULL;

			SDL_UnlockMutex(conn->mutex);
		}

		SDL_WaitThread(conn->thread, NULL);
		SDL_DestroyMutex(conn->mutex);
	}

	SDL_free(conn);
}

void Net_Conn_Send(Net_Conn *conn, Datablock *data) {
	if (conn == NULL) return;
	Net_SendBlock(conn->sock, data);
}