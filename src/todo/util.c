#include "util.h"

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
bool g_isRunning = true;
bool g_debug = false;


static SDL_EventType __uevent_type_clock;
static Uint32 __cb_clock(Uint32 interval, void *param) {
	SDL_UserEvent event = {
		.type = __uevent_type_clock,
		.code = UEVENT_CODE_CLOCK,
	};
	SDL_PushEvent((SDL_Event *) &event);

	if (g_isRunning) return interval;
	else return 0;
}


void Util_Init() {
	// Set up main clock timer
	__uevent_type_clock = SDL_RegisterEvents(1);
	SDL_AddTimer(TICK_MS, __cb_clock, NULL);
}

void Util_ErrMsg(const char *msg) {
	printf("[ERROR] %s: %s\n", msg, SDL_GetError());
	
	exit(1);
}

void Util_ClearScreen(SDL_Color clr) {
	SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(clr));
	SDL_RenderClear(g_renderer);
}

//void Util_DrawText(int x, int y, char *str, size_t len) {
//	int char_x = x;
//	for (int i=0; i<len && str[i] != '\0'; i++) {
//		SpriteID spid = str[i];
//		if (SDL_isprint(spid)) spid -= ' '; // Calculate Sprite ID
//		else continue;
//		Sprite_Draw(g_spsh_text, char_x, y, spid);
//		char_x += g_spsh_text->sprite_width + TEXT_KERN;
//	}
//}

void Util_DrawBinary(int x, int y, Uint32 num) {
	for (int i=32-1; i>=0; i--) {
		if (num & (0b1 << i))
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
		else
			SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderDrawPoint(g_renderer, x + i, y);
	}
}

//void Util_DrawVector(int x, int y, vector vec) {
//	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
//	SDL_RenderDrawLine(g_renderer,
//		CHUNK_SIZE * x, CHUNK_SIZE * y,
//		CHUNK_SIZE * (x+vec[0]), CHUNK_SIZE * (y+vec[1])
//	);
//}

void Util_FormatIP(char *buf, size_t buflen, IPaddress ip) {
	Uint8 bytes[4];
	for (int i=0; i<4; i++)
		bytes[i] = ip.host >> (i*8) & 0xFF;
	
	sprintf_s(buf, buflen, "%i.%i.%i.%i:%i", bytes[0], bytes[1], bytes[2], bytes[3], ip.port);
}