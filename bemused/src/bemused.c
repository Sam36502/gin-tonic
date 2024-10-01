// SDL2 Hello, World!

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "global.h"
#include "args.c"
#include "interface.c"
#include "commands.c"
#include "midiconvert.c"
#include "compiler.c"

void Terminate();

int main(int argc, char* argv[]) {
	// Set Logging to Develop Mode
	Log_SetPrintLevel(LOG_DEBUG);
	Log_SetPopupLevel(LOG_WARNING);

	// Get filename
	if (!parse_args(argc, argv)) {
		return EXIT_FAILURE;
	}

	Music_Init(16);

	// Initialise Test channel for playing simple notes on each instrument
	g_channel_id = Music_ChannelAlloc();
	Synth_SetChannelFrequency(g_synth, g_channel_id, 440.0f); // Set test channel to A4

	// Load music file provided
	if (g_new_file) {
		g_music = Music_File_Create(g_music_file_name);
		if (g_music == NULL) {
			printf("Failed to create music file '%s'. Exiting....\n", g_music_file_name);
			Terminate();
			return EXIT_FAILURE;
		}
	} else {
		g_music = Music_File_Open(g_music_file_name);
		if (g_music == NULL) {
			printf("Failed to load music file '%s'. Exiting....\n", g_music_file_name);
			Terminate();
			return EXIT_FAILURE;
		}
	}

	// Compile song file
	if (g_prog_function == IMPORT_ASM) {
		compile_song_data(g_music, g_asm_file_name);
	}

	// Select curr song
	if (g_curr_song >= g_music->song_count) {
		printf("Invalid song nr. %i: There are only %i songs in '%s'!\n",
			g_curr_song, g_music->song_count,
			g_music_file_name
		);
		Terminate();
		return EXIT_FAILURE;
	}

	// If the program is just supposed to output info, do so and exit
	if (g_prog_function == SHOW_INFO) {
		printf("\n  Music File '%s' contains:\n", g_music_file_name);
		puts("-------------------------------------");
		printf("  - %i Instrument(s):\n", g_music->ins_count);
		for (int i=0; i<g_music->ins_count; i++) {
			printf("      [0x%04X] %s-Wave\n",
				g_music->instruments[i].id,
				Synth_GetWaveformName(g_music->instruments[i].wave)
			);
		}
		printf("  - %i Song(s):\n", g_music->song_count);
		for (int i=0; i<g_music->song_count; i++) {
			printf("      [0x%04X] Tick: %ims, Channels: %i\n",
				g_music->songs[i].id,
				g_music->songs[i].tick_ms,
				g_music->songs[i].num_channels
			);
		}
		fflush(stdout);
		Terminate();
		return EXIT_SUCCESS;
	}

	// Import MIDI if required
	if (g_prog_function == IMPORT_MIDI) {
		bool success = convert_midi(g_music, g_midi_file_name);
		if (!success) {
			Terminate();
			return EXIT_FAILURE;
		}
	}

	// Initialise SDL & GinTonic
	if (SDL_Init(SDL_INIT_VIDEO) < 0) Log_SDLMessage(LOG_FATAL, "Failed to initialise SDL");
	Screen_Init("Bemused - GinTonic Music Editor", 512, 512);
	Sprite_Init();
	Text_Init("assets/text_sprites.png");
	Events_Init(50);

	UI_setup();

	g_music->loop_current = g_loop_song;
	if (g_music->loop_current) {
		puts("Looping Song..."); fflush(stdout);
	}

	// Main Loop
	bool isRunning = true;
	bool redraw = true;
	SDL_Event curr_event;
	while (isRunning) {

		// Handle events
		int scode = SDL_WaitEvent(&curr_event);

		if (scode != 0) {
			Events_HandleInternal(curr_event);

			switch (curr_event.type) {
				case SDL_QUIT:
					isRunning = false;
				continue;

				case SDL_USEREVENT: {
					switch (curr_event.user.code) {
						case UEVENT_CLOCK_TICK: {
							redraw = true;
							Music_Instrument ins = g_music->instruments[g_curr_ins];
							Synth_SetChannelEnvelope(g_synth, g_channel_id, ins.env);
							Synth_SetChannelWaveform(g_synth, g_channel_id, ins.wave, ins.wave_mod);
						} break;
					}
				} break;

				case SDL_KEYDOWN: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						case SDLK_SPACE: cb_note_down(-1, NULL); break;
						default: break;
					};
				} break;

				case SDL_KEYUP: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						case SDLK_SPACE: cb_note_up(-1, NULL); break;
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

				case SDL_TEXTINPUT: {

				} break;
			}
		}

		if (redraw) {
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(g_renderer);

			UI_draw();
			draw_commands();

			SDL_RenderPresent(g_renderer);
		}

	}
	
	Terminate();
	return 0;
}

void Terminate() {
	if (g_music != NULL) {
		Music_File_Stop(g_music);
		Music_ChannelFree(g_channel_id);
		Music_File_Close(g_music);
	}

	// Termination
	UI_cleanup();
	Music_Term();
	Button_Term();
	Sprite_Term();
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
}