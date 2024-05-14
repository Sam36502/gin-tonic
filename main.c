// SDL2 Hello, World!

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define GT_ENVIRONMENT_DEV
#include "include/log.h"
#include "include/screen.h"

int main(int argc, char* args[]) {
	// Set Logging to Develop Mode
	Log_SetPrintLevel(LOG_DEBUG);
	Log_SetPopupLevel(LOG_WARNING);

	// Initialisation
	if (SDL_Init(SDL_INIT_VIDEO) < 0) Log_SDLMessage(LOG_FATAL, "Failed to initialise SDL");
	Log_Message(LOG_DEBUG, "Hello, world!");
	Log_Message(LOG_INFO, "Hello, world!");

	Screen_Init("GinTonic Test Program", 1000, 128);

	// Main Loop
	bool isRunning = true;
	bool redraw = true;
	SDL_Event curr_event;
	while (isRunning) {

		// Handle events
		int scode = SDL_WaitEvent(&curr_event);

		if (scode != 0) {
			switch (curr_event.type) {
				case SDL_QUIT:
					isRunning = false;
				continue;

				case SDL_KEYDOWN: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						default: break;
					};
				} break;

				case SDL_KEYUP: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						default: break;
					};
					redraw = true;
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
			SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(g_renderer);

			SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
			SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){
				50, 50,
				50, 50
			});

			SDL_RenderPresent(g_renderer);
		}

	}

	// Termination
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return 0;
}