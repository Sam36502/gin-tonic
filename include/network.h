#ifndef GT_NETWORK_H
#define GT_NETWORK_H
//	
//				Network Utilities
//	
//		Handles creating and connecting to servers with TCP/IP.
//		Also includes basic IP utils.
//	
//	To-Do:
//		- Separate StartServer into Create & Start (Destroy & Stop) ?
//		- Get Datablock parsing to correctly handle partial blocks (que-buffer; periodically for valid full blocks?)

#include <SDL2/SDL_net.h>
#include <SDL2/SDL_thread.h>

#include "../include/events.h"
#include "../include/datablock.h"


//	
//		Constant Definitions
//	

#define NET_SOCKET_TIMEOUT 0
#define NET_THREAD_NAME_SERVER "gt_thread_srv"
#define NET_THREAD_NAME_CONNECTION "gt_thread_conn"
#define NET_BUFFER_SIZE 0x400

#define NULL_IP (IPaddress){0,0}

#define UEVENT_NET_RECEIVE		0x80
#define UEVENT_NET_CONNECT		0x81
#define UEVENT_NET_DISCONNECT	0x82


//	
//		Type Definitions
//	

typedef enum {
	NET_STATE_INIT,	// Starting / Connecting (transition)
	NET_STATE_GOOD,	// Server / Connection established and running
	NET_STATE_TERM,	// Stopping / Disconnecting (transition)
	NET_STATE_OFFL,	// Stopped / Closed (Offline)
} Net_State;

typedef struct {
	IPaddress ip;
	TCPsocket sock;
	SDL_mutex *mutex;
	SDL_Thread *thread;
	Net_State state;
} Net_Conn;

typedef struct {
	IPaddress ip;
	Uint16 capacity;
	Uint16 client_count;
	TCPsocket sock;
	SDLNet_SocketSet socketset;
	TCPsocket *client_socks;
	SDL_mutex *mutex;
	SDL_Thread *thread;
	Net_State state;
} Net_Server;


//	
//		Function Declarations
//	

//	Initialise Networking Features
//	
//	Registers new user-events for network events:
//	 - UEVENT_NET_RECEIVE: Data was received from an active connection; data1 = TCPsocket socket; data2 = Datablock *packet_data
//	 - UEVENT_NET_CONNECT: A connection was successfully established; data1 = TCPsocket socket; data2 = NULL
//	 - UEVENT_NET_DISCONNECT: A connection was terminated; data1 = IPaddress *addr; data2 = NULL
void Net_Init();

//	Terminate Networking System
//	
void Net_Term();

//	Format an IP-address into a human-readable string
//	
//	Returned pointer should not be freed; it is allocated internally.
//	If you need the resulting string between calls, you must copy it
char *Net_FormatIP(IPaddress ip);

//	Wraps Log_Message to include SDL_net error info
//	
void Net_Log_Message(Log_Level lvl, char *msg);

//	Sends a data-block over a TCP-Socket
//	
void Net_SendBlock(TCPsocket sock, Datablock *data);


//// Server Functions ////

//	Starts a server on the given port
//	
//	Capacity defines how many simultaneous connections the server can handle
//	Returns NULL on failure
Net_Server *Net_Server_Start(Uint16 port, Uint16 capacity);

//	Stops a server that was previously running
//	
void Net_Server_Stop(Net_Server *srv);

//	Gets the client-index of a client's TCP-Socket
//	
//	NOTE: The index is NOT an ID!
//	If a client disconnects, the others' IDs may be rearranged!
//	
//	Returns client index or -1 if the arguments are invalid,
//	or if that socket couldn't be found on the server
int Net_Server_GetClientIndex(Net_Server *srv, TCPsocket client_sock);

//	Disconnects a client from the server
//	
//	if `client_index` is <0, this disconnects all clients
void Net_Server_DisconnectClient(Net_Server *srv, int client_index);

//	Sends a data-block to a client of the given server
//	
//	Does nothing if `client_index` is invalid
//	Broadcast to all connected clients if `client_index` < 0
//	
//	(Wrapper for `Net_SendBlock()`)
void Net_Server_Send(Net_Server *srv, int client_index, Datablock *data);


//// Client Functions ////

//	Opens a connection to a server
//	
//	Returns NULL if the connection fails
Net_Conn *Net_Connect(IPaddress ip);

//	Disconnects from a server
//	
void Net_Disconnect(Net_Conn *conn);

//	Sends a data-block to the server over a Connection
//	
//	(Wrapper for `Net_SendBlock()`)
void Net_Conn_Send(Net_Conn *conn, Datablock *data);

#endif