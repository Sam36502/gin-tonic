#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>
#include <stdbool.h>

#include "util.h"
#include "worm.h"
#include "sprite.h"
#include "world.h"
#include "txtbox.h"
#include "network.h"

// OBSOLETE?
//#define CLR_WHITE (struct SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF }
//#define CLR_BLACK (struct SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
//#define CLR_ORANGE (struct SDL_Colour){ 0xFF, 0x80, 0x40, 0xFF }
//#define CLR_GREEN (struct SDL_Colour){ 0x40, 0xFF, 0x80, 0xFF }
//#define CLR_BLUE (struct SDL_Colour){ 0x00, 0x74, 0xD9, 0xFF }
//#define CLR_PINK (struct SDL_Colour){ 0xF0, 0x12, 0xBE, 0xFF }
#define CLR_LIGHT_GREY (struct SDL_Colour){ 0xDD, 0xDD, 0xDD, 0xFF }
//#define CLR_DARK_GREY (struct SDL_Colour){ 0xAA, 0xAA, 0xAA, 0xFF }

void Initialise();
void Terminate();


int main(int argc, char* args[]) {
	Initialise();

	// Main Loop
	bool redraw = true;
	SDL_Event curr_event;
	while (g_isRunning) {

		// Handle events
		int scode = SDL_WaitEventTimeout(&curr_event, 10);

		if (scode != 0) {
			switch (curr_event.type) {
				case SDL_USEREVENT: {
					if (curr_event.user.code == UEVENT_CODE_CLOCK) {
						// Clock tick
						Worm_Tick(g_player_worm, true);
						for (int i=0; i<g_net_entity_count; i++) {
							if (g_net_entity_list[i].type == NETENTITY_WORM) continue;
							Worm_Tick(WORM_ENTITY(g_net_entity_list[i]), false);
						}

						redraw = true;
					}
				} break;

				case SDL_QUIT:
					g_isRunning = false;
				continue;

				case SDL_KEYDOWN: {
					Worm_HandleInputs(g_player_worm, curr_event.key, true);
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						default: break;
					};
				} break;

				case SDL_KEYUP: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						case SDLK_F3: g_debug = !g_debug; break;
						case SDLK_F11: Net_StartServer("Sam's Super Server", 24); break;
						case SDLK_F12: {
							if (g_server_connection == NULL) {
								g_server_connection = Net_ConnectToServer(g_localhost);
							} else {
								Net_Disconnect(g_server_connection);
								g_server_connection = NULL;
							}
						} break;
						case SDLK_F10: {
							char name[256];
							Uint16 online, capacity;
							puts("Pinging Server...");
							bool is_valid = Net_PingServer(g_localhost, &online, &capacity, name, 256);
							if (is_valid) {
								puts("`g_localhost` is valid!");
								printf("	%s, %02i/%02i\n", name, online, capacity);
							} else {
								puts("`g_localhost` is not a valid Worm-Odyssey Server");
							}
							fflush(stdout);
						} break;
						case SDLK_F9: Net_SendInfoMessage(g_server_connection, "Hello, world!", 14); break;
						case SDLK_F8: Worm_JoinServer(g_server_connection, g_player_worm);
						case SDLK_F7:  break;
						default: break;
					};
					Worm_HandleInputs(g_player_worm, curr_event.key, false);
				} break;

				case SDL_MOUSEBUTTONDOWN: 
				case SDL_MOUSEBUTTONUP:
				break;

				case SDL_MOUSEMOTION:
				break;

				case SDL_MOUSEWHEEL: {
				} break;
			}
		}

		if (redraw) {
			Util_ClearScreen(CLR_LIGHT_GREY);

			World_Draw();

			for (int i=0; i<g_net_entity_count; i++) {
				if (g_net_entity_list[i].type == NETENTITY_WORM) continue;
				Worm_DrawBody(WORM_ENTITY(g_net_entity_list[i]));
			}
			Worm_DrawBody(g_player_worm);

			Txtbox_DrawTextbox(g_spsh_text, "Hello,\nworld!");
			//Sprite_DrawTextbox(g_spsh_text, "This is a much\nlonger piece of text\nthat will probably be cut off", 66);

			SDL_RenderPresent(g_renderer);
			redraw = false;
		}

	}

	Terminate();
	return 0;
}


// Initialise all the required parts of the engine in the requisite order
void Initialise() {
	
	// Init SDL
	Uint32 init_systems = SDL_INIT_VIDEO
		| SDL_INIT_TIMER
		;
	if (SDL_Init(init_systems) < 0)
		Util_ErrMsg("Failed to initialise SDL");

	// Initialise my subsystems
	Util_Init();
	Sprite_Init();
	Worm_Init();
	Net_Init();
	World_Init();
}

// Terminate / Destroy all the parts of the engine that need it
void Terminate() {
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	Net_Term();
	SDL_Quit();
}