#include "worm.h"

Worm *g_player_worm = NULL;

void Worm_Init() {
	g_player_worm = Worm_Create(
		SCREEN_WIDTH/2,
		SCREEN_HEIGHT/2,
		WORM_START_LEN
	);
}

Worm *Worm_Create(int spawn_x, int spawn_y, int length) {
	Worm *new_worm = SDL_malloc(sizeof(Worm));
	new_worm->body_clr = WORM_BODY_CLR;
	new_worm->head_clr = WORM_HEAD_CLR;
	new_worm->body[0][0] = spawn_x;
	new_worm->body[0][1] = spawn_y;
	new_worm->speed = WORM_START_SPEED;
	new_worm->input_mask = 0;
	new_worm->velocity = 0.0;
	new_worm->len = length;
	for (int i=0; i<new_worm->len; i++) {
		new_worm->body[i][0] = new_worm->body[0][0];
		new_worm->body[i][1] = new_worm->body[0][1] + i;
	}
	return new_worm;
}

void Worm_Destroy(Worm *wrm) {
	SDL_free(wrm);
}

void Worm_Tick(Worm *wrm, bool track_camera) {
	if (wrm->input_mask == 0) return;

	float worm_speed = wrm->speed;
	if (wrm->input_mask & INPUT_RUN) worm_speed *= 2;
	if (wrm->velocity < 1.0) {
		wrm->velocity += worm_speed;
		return;
	}

	while (wrm->velocity >= 1.0) {
		wrm->velocity -= 1.0;

		int head_x = wrm->body[0][0];
		int head_y = wrm->body[0][1];

		if (track_camera) {
			if (wrm->input_mask & INPUT_N) head_y--;
			if (wrm->input_mask & INPUT_S) head_y++;
			if (wrm->input_mask & INPUT_W) head_x--;
			if (wrm->input_mask & INPUT_E) head_x++;
		}

		if (wrm->body[0][0] == head_x && wrm->body[0][1] == head_y) continue;

		// Move the camera when the worm crosses the margin
		if (head_x < g_camera_x + MOVEMENT_MARGIN) g_camera_x--;
		if (head_x > g_camera_x + VIEW_WIDTH - MOVEMENT_MARGIN) g_camera_x++;
		if (head_y < g_camera_y + MOVEMENT_MARGIN) g_camera_y--;
		if (head_y > g_camera_y + VIEW_HEIGHT - MOVEMENT_MARGIN) g_camera_y++;

		for (int i=wrm->len-1; i>0; i--) {
			wrm->body[i][0] = wrm->body[i-1][0];
			wrm->body[i][1] = wrm->body[i-1][1];
		}
		wrm->body[0][0] = head_x;
		wrm->body[0][1] = head_y;
	}
}

void Worm_HandleInputs(Worm *wrm, SDL_KeyboardEvent evt, bool is_down) {
	switch (evt.keysym.sym) {

		case SDLK_UP:
		case SDLK_w: {
			if (is_down) wrm->input_mask |= INPUT_N;
			else wrm->input_mask &= ~INPUT_N;
		} break;

		case SDLK_DOWN:
		case SDLK_s: {
			if (is_down) wrm->input_mask |= INPUT_S;
			else wrm->input_mask &= ~INPUT_S;
		} break;

		case SDLK_RIGHT:
		case SDLK_d: {
			if (is_down) wrm->input_mask |= INPUT_E;
			else wrm->input_mask &= ~INPUT_E;
		} break;

		case SDLK_LEFT:
		case SDLK_a: {
			if (is_down) wrm->input_mask |= INPUT_W;
			else wrm->input_mask &= ~INPUT_W;
		} break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT: {
			if (is_down) wrm->input_mask |= INPUT_RUN;
			else wrm->input_mask &= ~INPUT_RUN;
		} break;
	}
}

void Worm_DrawBody(Worm *wrm) {
	if (g_debug) {
		// Orange Square: Movement Margin
		SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x80, 0x40, 0xFF);
		SDL_RenderDrawRect(g_renderer, &(struct SDL_Rect){
			MOVEMENT_MARGIN,
			MOVEMENT_MARGIN,
			VIEW_WIDTH - (2*MOVEMENT_MARGIN) + 1,
			VIEW_HEIGHT - (2*MOVEMENT_MARGIN) + 1,
		});
	}

	// Worm is outside the screen; don't draw
	if (wrm->body[0][0] < g_camera_x - wrm->len
		|| wrm->body[0][0] > g_camera_x + wrm->len + VIEW_WIDTH
		|| wrm->body[0][1] < g_camera_y - wrm->len
		|| wrm->body[0][1] > g_camera_y + wrm->len + VIEW_HEIGHT
	) return;

	// Draw the worm
	SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(wrm->body_clr));
	for (int i=1; i<wrm->len; i++) {
		SDL_RenderDrawPoint(g_renderer, wrm->body[i][0] - g_camera_x, wrm->body[i][1] - g_camera_y);
	}
	SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(wrm->head_clr));
	SDL_RenderDrawPoint(g_renderer, wrm->body[0][0] - g_camera_x, wrm->body[0][1] - g_camera_y);
}

void Worm_JoinServer(Connection *conn, Worm *worm) {
	// Send Join Request with worm data
	Uint8 data[sizeof(Worm)];
	SDL_memcpy(data, worm, sizeof(Worm));
	Net_RegisterEntityOnServer(conn, NETENTITY_WORM, data, sizeof(Worm));

	// Get Net-ID response
	Uint8 *resp;
	int len;
	Net_GetClientData(conn, &resp, &len);
	if (len != 2) { Net_Log("Invalid Network ID received", conn->ip); return; }
	U8_TO_U16(worm->net_id, resp[0], resp[1]);
}

// Fuck, man. I should have just gone for peer-to-peer.
// ...Or just used an off-the-shelf netcode library.

//Uint8 *Worm_Serialise(Worm *wrm) {
//	Uint8 *data = SDL_malloc(sizeof(Worm));
//	SDL_memcpy(data, wrm, sizeof(wrm));
//	return data;
//}
//
//Worm *Worm_Deserialise(Uint8 *data) {
//	Uint8 *wrm = SDL_malloc(sizeof(Worm));
//	SDL_memcpy(wrm, data, sizeof(Worm));
//	return wrm;
//}

void Worm_LeaveServer(Connection *conn, Worm *worm) {

}
