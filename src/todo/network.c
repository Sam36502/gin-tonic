#include "network.h"


// Actually extern
IPaddress g_localhost;
Connection *g_server_connection = NULL;
char *g_server_name = NULL;
Network_Entity g_net_entity_list[MAX_NET_ENTITIES];
int g_net_entity_count = 0;

// Probably not needed outside
SDL_Thread *g_server_thread = NULL;
SDL_mutex *g_server_mutex = NULL;
SDLNet_SocketSet g_socketset;
TCPsocket g_clients[MAX_NET_CONNECTIONS];
int g_client_count = 0;

Uint16 g_server_capacity;
bool g_server_running = false;


void Net_Init() {
	if (SDLNet_Init() != 0)
		Util_ErrMsg("Failed to initialise Networking");

	g_socketset = SDLNet_AllocSocketSet(MAX_NET_CONNECTIONS);

	// Create local reference to `localhost` for debugging
	if (SDLNet_ResolveHost(&g_localhost, "localhost", SERVER_PORT) != 0)
		Util_ErrMsg("Failed to resolve 'localhost'");
	
	g_server_mutex = SDL_CreateMutex();
}

void Net_Term() {
	Net_StopServer();

	SDL_DestroyMutex(g_server_mutex);
	SDLNet_FreeSocketSet(g_socketset);
	SDLNet_Quit();
}

void Net_FormatIP(char *buf, size_t len, IPaddress ip) {
	Uint8 bytes[4];
	for (int i=0; i<4; i++)
		bytes[i] = ip.host >> (i*8) & 0xFF;
	
	sprintf_s(buf, 256, "%i.%i.%i.%i:%i", bytes[0], bytes[1], bytes[2], bytes[3], ip.port);
}

void Net_Log(char *msg, IPaddress ip) {
	if (ip.host == 0) {
		printf("[INFO]: %s\n", msg);
	} else {
		char ipstr[256];
		Net_FormatIP(ipstr, 256, ip);
		printf("[INFO](%s): %s\n", ipstr, msg);
	}
	fflush(stdout);
}

void __DisconnectClient(TCPsocket client) {
	// Find client index
	int index = -1;
	for (int i=0; i<g_client_count; i++) {
		if (client == g_clients[i]) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		IPaddress ip = *SDLNet_TCP_GetPeerAddress(client);
		Net_Log("Couldn't find client's index! Aborting disconnect", ip);
		return;
	}

	// Remove client from list & Disconnect
	SDLNet_TCP_DelSocket(g_socketset, client);
	g_client_count--;
	TCPsocket last_client = g_clients[g_client_count];
	g_clients[index] = last_client;
	SDLNet_TCP_Close(client);
}

void __HandleServerData(Uint8 *data, int len, TCPsocket socket) {
	IPaddress ip = *SDLNet_TCP_GetPeerAddress(socket);
	if (len < 4) {
		Net_Log("Invalid block received (too small)", ip);
		return;
	}

	int bytes_left = len;
	while (bytes_left > 0) {
		Uint8 block_type = data[0];
		//Uint8 blocks_left = data[1]; // How many blocks left to read if in a chain
		Uint16 block_size;
		U8_TO_U16(block_size, data[2], data[3]);
		data += 4;
		bytes_left -= 4;

		switch (block_type) {

			case NETBLOCK_ACK: {
				if (block_size != 0) {
					Net_Log("Invalid ACK-block received", ip);
					return;
				}
			} break;

			case NETBLOCK_PING: {
				Net_Log("Ping Received", ip);
				int name_len = SDL_strlen(g_server_name);
				int worm_count = 0;
				for (int i=0; i<g_net_entity_count; i++) {
					if (g_net_entity_list[i].type == NETENTITY_WORM) worm_count++;
				}
				Uint8 *resp = SDL_malloc(sizeof(Uint8) * (4+16+name_len));
				Uint8 top[] = {
					NETBLOCK_PING, 0x00, 
					(name_len+16) >> 8,
					(name_len+16) & 0xFF,
					0x57, 0x4F, 0x52, 0x4D, 0x2D, 0x4F, 0x44, 0x59, 0x53, 0x53, 0x45, 0x59, // 'WORM-ODYSSEY' Magic Bytes
					worm_count >> 8,
					worm_count  & 0xFF,
					g_server_capacity >> 8,
					g_server_capacity & 0xFF,
				};
				SDL_memcpy(resp, top, 4+16);
				SDL_memcpy(resp+4+16, g_server_name, name_len);
				SDLNet_TCP_Send(socket, resp, 4 + 16 + name_len);
			} break;

			case NETBLOCK_ENT_ADD: {
				// TODO: Parse received worm and add it to the list of tracked worms
				// TODO: Also, forward new worm info to all connected users
				//       Actually, we'll eventually need a universal function
				//       to forward all updates to the net entity list to clients...
				Net_Log("New worm wants to join the server", ip);
				Network_Entity *ent = SDL_malloc(sizeof(Network_Entity));
				SDL_memcpy(&ent->type, data, sizeof(Network_Entity_Type));
				data += sizeof(Network_Entity_Type);
				bytes_left -= sizeof(Network_Entity_Type);
				ent->entity_size = bytes_left;
				SDL_memcpy(ent->entity, data, ent->entity_size);
			}

			case NETBLOCK_INFOMSG: {
				Net_Log("Info Message Received:", ip);
				printf("\t\t");
				for (int i=0; i<block_size; i++) {
					char c = data[i];
					if (SDL_isprint(c)) putchar(c);
					if (c == '\n') printf("\n\t\t");
				}
				putchar('\n');
				fflush(stdout);
			} break;

			case NETBLOCK_TERMINATE: {
				__DisconnectClient(socket);
				Net_Log("Disconnected", ip);
				return; // ignore any further packets from this socket
			} break;

			default: {
				Net_Log("Unknown Block Type received", ip);
			} break;

		}

		data += block_size;
		bytes_left -= block_size;
	}
}

int __ServerThread(void *data) {
	IPaddress serv_addr = {
		.host = INADDR_ANY,
		.port = SERVER_PORT,
	};
	TCPsocket server_socket = SDLNet_TCP_Open(&serv_addr);
	if (server_socket  == NULL)
		Util_ErrMsg("Failed to open Server Socket");

	Uint8 *__data_buffer = SDL_malloc(sizeof(Uint8) * MAX_NET_BLOCK_SIZE);

	while (g_server_running) {
		SDL_UnlockMutex(g_server_mutex);

		// Check if anyone wants to connect // TODO: Put in separate thread?
		TCPsocket client_sock = NULL;
		client_sock = SDLNet_TCP_Accept(server_socket);
		if (client_sock != NULL) {
			g_clients[g_client_count++] = client_sock;
			IPaddress *client_addr = SDLNet_TCP_GetPeerAddress(client_sock);
			SDLNet_TCP_AddSocket(g_socketset, client_sock);
			Net_Log("Connection received!", *client_addr);
		}

		// Receive data from clients
		if (g_client_count > 0) {
			int ready_socks = SDLNet_CheckSockets(g_socketset, SOCKET_CHECK_TIMEOUT);
			if (ready_socks < 0) Util_ErrMsg("Failed to check sockets");
			if (ready_socks > 0) {
				int had_data = 0;
				for (int i=0; had_data<ready_socks && i<g_client_count; i++) {
					TCPsocket sock = g_clients[i];
					int bytes_received = SDLNet_TCP_Recv(sock, __data_buffer, MAX_NET_BLOCK_SIZE);
					if (bytes_received <= 0) continue;

					had_data++;
					__HandleServerData(__data_buffer, bytes_received, sock);
				}
			}
		}

		SDL_LockMutex(g_server_mutex);
	}

	SDL_free(__data_buffer);
	return 0;
}

void Net_StartServer(char *name, Uint16 capacity) {
	if (g_server_mutex == NULL) Util_ErrMsg("Tried to start the server before initialising networking");

	g_server_name = name;
	g_server_capacity = capacity;
	g_server_running = true;
	g_server_thread = SDL_CreateThread(__ServerThread, THREAD_NAME_SRV, NULL);

	Net_Log("Server Started!", NULL_IP);
}

void Net_StopServer() {
	if (!g_server_running || g_server_mutex == NULL) return;

	Net_Log("Stopping Server...", NULL_IP);
	SDL_LockMutex(g_server_mutex);
	g_server_running = false;
	SDL_UnlockMutex(g_server_mutex);

	SDL_WaitThread(g_server_thread, NULL);
	Net_Log("Server Stopped!", NULL_IP);
}


// Client functions

// Pass NULL to any param you don't need
//
// Pings a server and returns true if the server seems to be a valid Worm Odyssey server.
// Also returns server info into any pointers if provided
bool Net_PingServer(IPaddress ip, Uint16 *players, Uint16 *capacity, char *name, size_t name_len) {
	Connection *conn = Net_ConnectToServer(ip);
	if (conn == NULL) return false;

	// Ping Server
	Uint8 data[4] = { 0x01, 0x00, 0x00, 0x00 };
	SDLNet_TCP_Send(conn->sock, data, 4);
	Uint8 *resp;
	int resp_len;
	Net_GetClientData(conn, &resp, &resp_len);
	if (resp_len < 20) return false;

	// Check if response matches worm-odyssey protocol
	if (resp[0] != 0x01 || resp[1] != 0x00) return false;
	Uint16 resp_block_len;
	U8_TO_U16(resp_block_len, data[2], data[3]);
	if (resp_block_len != resp_len-4) return false;
	if (players != NULL) U8_TO_U16(*players, resp[16], resp[17]);
	if (capacity != NULL) U8_TO_U16(*capacity, resp[18], resp[19]);
	resp[16] = '\0';
	if (SDL_strcmp("WORM-ODYSSEY", (char *)(resp+4)) != 0) return false;

	// Read server name
	if (name != NULL || name_len <= 0) {
		SDL_memcpy(name, resp+20, name_len);
	}

	Net_Disconnect(conn);
	return true;
}

void __HandleClientData(Uint8 *data, int len, Connection *conn) {
	int bytes_left = len;
	while (bytes_left > 0) {
		Uint8 block_type = data[0];
		Uint8 block_index = data[1];
		Uint16 block_len;
		U8_TO_U16(block_len, data[2], data[3]);
		data += 4;
		bytes_left -= 4;

		switch (block_type) {

			case NETBLOCK_PING: { // Pass-through for ping function to read
				PASS_DATA_THROUGH()
			} break;

			case NETBLOCK_ENT_ADD: {
				if (block_index == 0x01) { // This is a response to our client registering the entity; passthrough
					PASS_DATA_THROUGH()
				}

				// This is a message forwarded from the server to inform us a network entity has joined
				//Network_Entity *ent = SDL_malloc(sizeof(Network_Entity));
				////void *entity_data = SDL_malloc(sizeof());
				//NetEntity_Register(ent);
			} break;

			default: Net_Log("Unknown Block type received", conn->ip); break;
		}

		data += block_len;
		bytes_left -= block_len;
	}
}

int __ClientThread(void *_conn) {
	Connection *conn = (Connection *) _conn;
	Uint8 *data = SDL_malloc(sizeof(Uint8) * MAX_NET_BLOCK_SIZE);

	SDL_LockMutex(conn->mutex);
	SDLNet_SocketSet sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(sockset, conn->sock);

	while (conn->thread_running) {
		SDL_UnlockMutex(conn->mutex);
		
		// Check for any packets from the server
		int ready = SDLNet_CheckSockets(sockset, SOCKET_CHECK_TIMEOUT);
		if (ready < 0) Util_ErrMsg("Failed to check server socket from client!");

		int resp_len = SDLNet_TCP_Recv(conn->sock, data, MAX_NET_BLOCK_SIZE);
		if (resp_len < 0) Util_ErrMsg("Failed to check server socket from client!");
		__HandleClientData(data, resp_len, conn);

		SDL_LockMutex(conn->mutex);
	}

	SDL_free(data);
	return 0;
}

Connection *Net_ConnectToServer(IPaddress ip) {
	ip.port = SERVER_PORT;
	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if (sock == NULL) return NULL;

	Connection *conn = SDL_malloc(sizeof(Connection));
	conn->ip = ip;
	conn->sock = sock;
	conn->mutex = SDL_CreateMutex();
	conn->cond_passthrough = SDL_CreateCond();
	conn->thread_running = true;
	conn->thread = SDL_CreateThread(__ClientThread, THREAD_NAME_CLI, (void *) conn);
	conn->last_data = NULL;
	conn->last_data_len = 0;
	Net_Log("Connected!", ip);
	return conn;
}

void Net_RegisterEntityOnServer(Connection *conn, Network_Entity_Type entity_type, Uint8 *entity_data, size_t entity_size) {
	// Serialise Network Entitiy
	int block_len = sizeof(Network_Entity_Type) + entity_size;
	Uint8 *data = SDL_malloc(4 + block_len);
	data[0] = NETBLOCK_ENT_ADD;
	data[1] = 0x00;
	U16_TO_U8(block_len, data[2], data[3]);
	for (int i=0; i<sizeof(Network_Entity_Type); i++) data[4+i] = (entity_type >> (8*i)) & 0xFF;
	SDL_memcpy(data+4+sizeof(Network_Entity_Type), entity_data, entity_size);

	// Sent serialised data to server
	SDLNet_TCP_Send(conn->sock, data, 4 + block_len);
}

void Net_UpdateEntityOnServer(Connection *conn, Network_Entity *ent) {

}

void Net_RemoveEntityOnServer(Connection *conn, Network_Entity *ent);

void Net_Disconnect(Connection *conn) {
	if (conn == NULL) return;
	
	// Send disconnect message to server
	Uint8 buf[4] = { NETBLOCK_TERMINATE, 0x00, 0x00, 0x00 };
	SDLNet_TCP_Send(conn->sock, buf, 4);

	// Stop Client thread
	SDL_LockMutex(conn->mutex);
	conn->thread_running = false;
	SDL_UnlockMutex(conn->mutex);
	SDL_WaitThread(conn->thread, NULL);

	// Disconnect
	SDLNet_TCP_Close(conn->sock);
	SDL_DestroyCond(conn->cond_passthrough);
	SDL_DestroyMutex(conn->mutex);
	SDL_free(conn);
	Net_Log("Disconnected", conn->ip);
}

void Net_SendInfoMessage(Connection *conn, char *str, size_t len) {
	if (conn == NULL) return;
	Uint8 buf[4+len];
	buf[0] = NETBLOCK_INFOMSG;
	buf[1] = 0x00;
	buf[2] = len >> 8;
	buf[3] = len & 0xFF;
	SDL_memmove(buf+4, str, len);
	SDLNet_TCP_Send(conn->sock, buf, 4+len);
}

void NetEntity_Register(Network_Entity *ent) {
	// Find new unique ID
	Network_Entity_ID new_id = 0;
	for (int i=0; i<g_net_entity_count; i++) {
		if (new_id <= g_net_entity_list[i].id) new_id = g_net_entity_list[i].id + 1;
	}

	ent->id = new_id;
	g_net_entity_list[g_net_entity_count] = *ent;
	g_net_entity_count++;
}

void NetEntity_Remove(Network_Entity_ID ent_id) {
	int index = -1;
	for (int i=0; i<g_net_entity_count; i++) {
		if (g_net_entity_list[i].id == ent_id) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		g_net_entity_count--;
		g_net_entity_list[index] = g_net_entity_list[g_net_entity_count];
	}
}

void Net_LogEntities() {
	puts("Currently Registered Network Entities:");
	for (int i=0; i<g_net_entity_count; i++) {
		printf("	[%04i](0x%02X) 0x%p\n", i, g_net_entity_list[i].type, g_net_entity_list[i].entity);
	}
	fflush(stdout);
}

void Net_GetClientData(Connection *conn, Uint8 **data, int *len) {
	if (conn == NULL) return;
	SDL_LockMutex(conn->mutex);
	SDL_CondWait(conn->cond_passthrough, conn->mutex);
	if (data != NULL) *data = conn->last_data;
	if (len != NULL) *len = conn->last_data_len;
	SDL_UnlockMutex(conn->mutex);
}