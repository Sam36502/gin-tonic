#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_thread.h>
#include "util.h"

#define SERVER_PORT 8779 // 'W' 'O' in ASCII
#define THREAD_NAME_SRV "WO Server"
#define THREAD_NAME_CLI "WO Client"
#define SOCKET_CHECK_TIMEOUT 100
#define MAX_NET_ENTITIES 256
#define MAX_NET_CONNECTIONS 256
#define MAX_NET_BLOCK_SIZE (UINT16_MAX+1)

// Netblock Type-Bytes
#define NETBLOCK_ACK		0x00
#define NETBLOCK_PING		0x01
#define NETBLOCK_ENT_ADD	0x10
#define NETBLOCK_ENT_UPDATE	0x11
#define NETBLOCK_ENT_REMOVE	0x1F
#define NETBLOCK_INFOMSG	0xFE
#define NETBLOCK_TERMINATE	0xFF

#define NULL_IP (IPaddress){0,0}
#define PASS_DATA_THROUGH() \
				conn->last_data = data-4; \
				conn->last_data_len = block_len+4; \
				SDL_CondSignal(conn->cond_passthrough); \

typedef struct {
	IPaddress ip;
	TCPsocket sock;
	SDL_mutex *mutex;
	SDL_cond *cond_passthrough;
	SDL_Thread *thread;
	Uint8 *last_data;
	int last_data_len;
	bool thread_running;
} Connection;

typedef Uint16 Network_Entity_ID;

typedef enum {
	NETENTITY_WORM
} Network_Entity_Type;

typedef struct {
	Network_Entity_Type type;
	Network_Entity_ID id; // Net-id for server-side entities
	void *entity;
	size_t entity_size;
} Network_Entity;


extern IPaddress g_localhost;
extern Connection *g_server_connection;
extern char *g_server_name;
extern Network_Entity g_net_entity_list[MAX_NET_ENTITIES];
extern int g_net_entity_count;
//extern SDL_mutex *g_server_mutex;
//extern SDL_Thread *g_server_thread;


void Net_Init();
void Net_Term();
void Net_FormatIP(char *buf, size_t len, IPaddress ip);
void Net_Log(char *msg, IPaddress ip);
void Net_StartServer(char *name, Uint16 capacity);
void Net_StopServer();

bool Net_PingServer(IPaddress ip, Uint16 *players, Uint16 *capacity, char *name, size_t name_len); // Pass NULL to any param you don't need
Connection *Net_ConnectToServer(IPaddress ip);
void Net_RegisterEntityOnServer(Connection *conn, Network_Entity_Type entity_type, Uint8 *entity_data, size_t entity_size);
void Net_UpdateEntityOnServer(Connection *conn, Network_Entity *ent);
void Net_RemoveEntityOnServer(Connection *conn, Network_Entity *ent);
void Net_Disconnect(Connection *conn);
void Net_SendInfoMessage(Connection *conn, char *str, size_t len);

void NetEntity_Register(Network_Entity *ent);
void NetEntity_Remove(Network_Entity_ID ent_id);
void Net_LogEntities();
void Net_GetClientData(Connection *conn, Uint8 **data, int *len);

#endif