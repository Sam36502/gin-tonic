#ifndef WORM_H
#define WORM_H

#include "util.h"
#include "world.h"
#include "network.h"
#include <stddef.h>
#include <stdbool.h>

#define WORM_MAX_LEN 150
#define WORM_START_LEN 6
#define WORM_START_SPEED 1.0
#define WORM_BODY_CLR (struct SDL_Colour){ 0xFB, 0xC3, 0xA7, 0xFF }
#define WORM_HEAD_CLR (struct SDL_Colour){ 0xD7, 0x85, 0x5B, 0xFF }
#define WORM_DARK_CLR (struct SDL_Colour){ 0x81, 0x4A, 0x3C, 0xFF }
#define WORM_ENTITY(e) (Worm *)(e.entity)

// Define Worm Control-Mask
#define INPUT_N		0b00000001
#define INPUT_W		0b00000010
#define INPUT_E		0b00000100
#define INPUT_S		0b00001000
#define INPUT_DIR	INPUT_N | INPUT_S | INPUT_E | INPUT_W
#define INPUT_RUN	0b00010000

typedef struct Worm {
	int len;
	float speed;	// Worm's possible speed
	float velocity;	// Worm's current speed
	Uint32 input_mask;
	SDL_Color head_clr;
	SDL_Color body_clr;
	int body[WORM_MAX_LEN][2];
	Network_Entity_ID net_id; // Net ID for client-side worms
} Worm;

extern Worm *g_player_worm;

void Worm_Init();
Worm *Worm_Create(int spawn_x, int spawn_y, int length);
void Worm_Destroy(Worm *wrm);
void Worm_Tick(Worm *wrm, bool track_camera);
void Worm_HandleInputs(Worm *wrm, SDL_KeyboardEvent evt, bool is_down);
void Worm_DrawBody(Worm *wrm);

void Worm_JoinServer(Connection *conn, Worm *worm);
//Uint8 *Worm_Serialise(Worm *wrm);
//Worm *Worm_Deserialise(Uint8 *data, size_t len);
void Worm_LeaveServer(Connection *conn, Worm *worm);

#endif