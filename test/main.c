// SDL2 Hello, World!

#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "../include/log.h"
#include "../include/util.h"
#include "../include/text.h"
#include "../include/synth.h"
#include "../include/music.h"
#include "../include/screen.h"
#include "../include/events.h"
#include "../include/button.h"
#include "../include/datablock.h"
#include "../include/binding.h"

#include "menu_scene.c"
#include "test_scene.c"

#define GREEN (SDL_Colour){ 0x20, 0xFF, 0x40, 0xFF }

Music_File *g_music = NULL;
bool is_green = false;
Orientation g_orientation = ORIENT_TOP;
Spritesheet *g_spsh_wave_btns = NULL;

void say_hello(int id, void *_) {
	Log_Message(LOG_INFO, "Hello, button!");
	Music_File_Play(g_music, 0);
	is_green = !is_green;
	//Synth_TriggerChannelEnvelope(g_synth, 0);
}

void end_note(int id, void *_) {
	//Music_File_End(g_music);
	//Synth_ReleaseChannelEnvelope(g_synth, 0);
}

int main(int argc, char* args[]) {
	// Set Logging to Develop Mode
	Log_SetPrintLevel(LOG_DEBUG);
	Log_SetPopupLevel(LOG_WARNING);

	// Initialisation
	if (SDL_Init(SDL_INIT_VIDEO) < 0) Log_SDLMessage(LOG_FATAL, "Failed to initialise SDL");
	Log_Message(LOG_DEBUG, "Hello, world!");
	Log_Message(LOG_INFO, "Hello, world!");

	// Test fs functions
	char *dirname = Util_FS_Dirname(args[0]);
	char *basename = Util_FS_Basename(args[0]);
	char *sdlbase = SDL_GetBasePath();
	printf("Testing FS Utilities!\n  ARG 0: %s\n    DIR: %s\n,   BASE: %s\n", args[0], dirname, basename);
	printf("SDL BaseName = '%s'\n", sdlbase);
	SDL_free(dirname);
	SDL_free(basename);
	SDL_free(sdlbase);
	fflush(stdout);

	Screen_Init("GinTonic Test Program", 128, 128);
	Sprite_Init();
	//Text_Init("../assets/text_sprites.png");
	Text_Init(NULL);
	Events_Init(50);

	//g_spsh_wave_btns = Sprite_LoadSheetGrid("../assets/wave_buttons.png", 16, 16);

	Music_Init(16);
	g_music = Music_File_Open("bemused/music/test_music.dbf");
	if (g_music == NULL) {
		Log_Message(LOG_FATAL, "Oh, fiddlesticks....");
		return 1;
	}
	Synth_CreateChannelOscilloscope(g_synth, 0,
		g_renderer,
		&(struct SDL_Rect){
			0, 75,
			128, 50
		},
		(SDL_Colour){ 0x20, 0xFF, 0x40, 0xFF },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF },
		(SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
	);

	Datablock_File *player_prefs = Datablock_File_Open("settings.dbf");
	Binding_InitFromFile(player_prefs, 0xBEEF);
//	Binding_Init();
//	Binding_Add(INPUT_UP, SDLK_UP);
//	Binding_Add(INPUT_UP, SDLK_w);
//	Binding_Add(INPUT_UP, SDLK_k);
//	Binding_Add(INPUT_DOWN, SDLK_DOWN);
//	Binding_Add(INPUT_DOWN, SDLK_s);
//	Binding_Add(INPUT_DOWN, SDLK_j);
//	Binding_Add(INPUT_LEFT, SDLK_LEFT);
//	Binding_Add(INPUT_LEFT, SDLK_a);
//	Binding_Add(INPUT_LEFT, SDLK_h);
//	Binding_Add(INPUT_RIGHT, SDLK_RIGHT);
//	Binding_Add(INPUT_RIGHT, SDLK_d);
//	Binding_Add(INPUT_RIGHT, SDLK_l);
//
//	Binding_Add(INPUT_BACK, SDLK_ESCAPE);
//	Binding_Add(INPUT_BACK, SDLK_BACKSPACE);
//	Binding_Add(INPUT_SELECT, SDLK_SPACE);
//	Binding_Add(INPUT_SELECT, SDLK_RETURN);
//	Binding_WriteToFile(player_prefs, 0xBEEF);

//	Synth_SetChannelEnvelope(g_synth, 0, (Synth_Envelope){
//		0.01,
//		0.01,
//		0.80,
//		0.50
//	});

	puts("Music File parsed successfully!");
	printf("        Num. Songs: %i\n", g_music->song_count);
	printf("  Num. Instruments: %i\n", g_music->ins_count);
	printf("  Songs:\n");
	for (int i=0; i<g_music->song_count; i++) {
		printf("    [%i] Tick: %ims, Channels: %i, Data: 0x%02X\n", i,
			g_music->songs[i].tick_ms,
			g_music->songs[i].num_channels,
			g_music->songs[i].channels[0].data[0]
		);
	}
	fflush(stdout);
	g_music->loop_current = true;

	Button_Create(
		(struct SDL_Rect){
			50, 50,
			50, 25
		},
		false,
		&say_hello,
		&end_note,
		NULL,
		(SDL_Colour){ 0x40, 0x00, 0x00, 0xFF },
		(SDL_Colour){ 0x80, 0x00, 0x00, 0xFF },
		(SDL_Colour){ 0xFF, 0x00, 0x00, 0xFF }
	);

	Textbox *txt = Text_Box_Create(
		"\nSphinx of " \
		TEXT_ESC_CLR "\x33\x33\x33" \
		"black" \
		TEXT_ESC_RESET \
		" quartz, judge my vow's appraisal!" \
		TEXT_ESC_CLR "\xFF\x80\x40" \
		"!" \
		TEXT_ESC_RESET \
		"\n\x83",
	68, 10, 10);

	Scene_Register(scn_menu, "menu");
	Scene_Register(scn_test, "test");
	Scene_Set("menu");
	Scene_Execute();
	return 0;

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
						} break;
					}
				} break;

				case SDL_KEYDOWN: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						default: break;
					};
				} break;

				case SDL_KEYUP: {
					SDL_KeyCode kc = curr_event.key.keysym.sym;
					switch (kc) {
						case SDLK_RIGHT: g_orientation++; g_orientation &= 0b11; break;
						case SDLK_LEFT: g_orientation--; g_orientation &= 0b11; break;
						default: break;
					};
					printf("---> New Orientation: 0b%i%i\n", g_orientation>>1, g_orientation&0b01);
					fflush(stdout);
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
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x20, 0x80, 0xFF);
			SDL_RenderClear(g_renderer);

			//Text_Draw("A sphinx's black quartz for judging my vowel!", 37, 10, 20);
			//Text_PrintEncoding(4, 4);
			
			//if (is_green) Text_SetColour(GREEN);
			Text_Box_Draw(txt, 5, 5);
			//if (is_green) Text_ResetColour();
			SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x40, 0x20, 0xFF);
			SDL_RenderDrawRect(g_renderer, &(struct SDL_Rect){
				30, 30, 16, 16,
			});
			Sprite_DrawRot(g_spsh_wave_btns, 30, 30, 0x00, g_orientation);

			//Text_Draw("Hello, \x27\x01\x20\x40\xFFworld\x27\x00!", 21, 10, 10);

			Button_DrawAll();
			Synth_DrawOscilloscopes(g_synth, g_renderer);
			SDL_RenderPresent(g_renderer);
		}

	}

	Text_Box_Destroy(txt);
	Binding_Term();
	Datablock_File_Close(player_prefs);
	Music_File_Close(g_music);
	Sprite_FreeSheet(g_spsh_wave_btns);

	// Termination
	Music_Term();
	Button_Term();
	Sprite_Term();
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return 0;
}