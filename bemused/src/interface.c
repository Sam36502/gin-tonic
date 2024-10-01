//	
//	Handles drawing and processing UI
//	

#define INS_BOX_WIDTH 100
#define INS_BOX_HEIGHT 130

#define OSC_WIDTH 200
#define OSC_HEIGHT 100

#include "global.h"


static bool __is_space_held = false;
static Spritesheet *wave_buttons_spsh;
static SDL_Rect *__osc_rect;
static Synth_Wave __wave_IDs[5];
static int __new_channels = 1;
static int __song_down = -1;
static int __song_up = +1;


void UI_setup();


void cb_select_ins(int id, void *_ins) {
	Music_Instrument *ins= (Music_Instrument *) _ins;
	g_curr_ins = ins->id;
}

void cb_save(int id, void *_) {
	Log_Message(LOG_INFO, "Saving Music File...");
	Music_File_Save(g_music);
}

void cb_play(int id, void *_) {
	Log_Message(LOG_INFO, "Playing Music File...");
	Music_File_Play(g_music, g_curr_song);
}

void cb_stop(int id, void *_) {
	Log_Message(LOG_INFO, "Stopping Music File...");
	Music_File_Stop(g_music);
}

void cb_new_song(int id, void *_ch_count) {
	int ch_count = *(int *) _ch_count;
	Log_Message(LOG_INFO, "Adding new song to file");
	g_curr_song = Music_File_CreateSong(g_music, ch_count);
}

void cb_del_song(int id, void *_) {
	for (int i=g_curr_song; i<g_music->song_count-1; i++) {
		g_music->songs[i] = g_music->songs[i+1];
	}
	g_music->song_count--;
	g_curr_song = g_music->song_count-1;
}

void cb_new_ins(int id, void *_) {
	Log_Message(LOG_INFO, "Adding new instrument to file");
	g_music->ins_count++;
	g_music->instruments = SDL_realloc(g_music->instruments, sizeof(Music_Instrument) * g_music->ins_count);
	Music_Instrument *ins = &g_music->instruments[g_music->ins_count-1];
	ins->id = g_music->ins_count-1;
	ins->wave = SYNTH_WAVE_SINE;
	ins->wave_mod = 0.0f;
	ins->env = (Synth_Envelope){
		.attack = 0.0f,
		.decay = 0.0f,
		.sustain = 1.0f,
		.release = 0.0f,
	};
	Button_Term();
	Slider_Term();
	UI_setup();
}

void cb_del_ins(int id, void *_) {
	Log_Message(LOG_INFO, "Removing selected instrument from file");
	for (int i=g_curr_ins; i<g_music->ins_count-1; i++) {
		g_music->instruments[i] = g_music->instruments[i+1];
	}
	g_music->ins_count--;
	g_curr_ins = g_music->ins_count-1;
	Button_Term();
	Slider_Term();
	UI_setup();
}

void cb_select_wave(int id, void *_wave) {
	Synth_Wave wave = *(Synth_Wave *) _wave;
	g_music->instruments[g_curr_ins].wave = wave;
}

void cb_note_down(int id, void *_) {
	if (__is_space_held) return;
	__is_space_held = true;
	Synth_TriggerChannelEnvelope(g_synth, g_channel_id);
}

void cb_note_up(int id, void *_) {
	__is_space_held = false;
	Synth_ReleaseChannelEnvelope(g_synth, g_channel_id);
}

void cb_set_song(int id, void *_nr) {
	int nr = *(int *) _nr;
	if (nr == -1 && g_curr_song == 0) return;
	if (nr == +1 && g_curr_song == g_music->song_count-1) return;
	g_curr_song += nr;
}

void UI_setup() {
	wave_buttons_spsh = Sprite_LoadSheetGrid("assets/wave_buttons.png", 16, 16);

	// Set sliders' look
	Slider_SetKnobStyle(2, 10, (SDL_Colour){ 0x80, 0x80, 0x80, 0xFF });
	Slider_SetSliderStyle(4, (SDL_Colour){ 0x40, 0x40, 0x40, 0xFF });
	
	// Right-Side Save Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10-1,
			5*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_save,
		NULL, NULL,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side Play Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 1*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			5*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_play,
		NULL, NULL,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side Stop Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 2*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			9*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_stop,
		NULL, &__new_channels,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side New Song Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 3*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			9*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_new_song,
		NULL, &__new_channels,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side New Song Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 4*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			9*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_del_song,
		NULL, NULL,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side New Instrument Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 5*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			9*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_new_ins,
		NULL, NULL,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Right-Side Del Instrument Button
	Button_Create(
		(struct SDL_Rect){
			g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH)-1, 10 + 6*(TEXT_KERN + TEXT_SPRITE_HEIGHT)-1,
			9*(TEXT_KERN+TEXT_SPRITE_WIDTH)+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_del_ins,
		NULL, NULL,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);


	// Song selector buttons
	Button_Create(
		(struct SDL_Rect){
			5 + INS_BOX_WIDTH + 5 - 1, 5 - 1,
			TEXT_SPRITE_WIDTH+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_set_song,
		NULL, &__song_up,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);
	Button_Create(
		(struct SDL_Rect){
			5 + INS_BOX_WIDTH + 5 + 2*(TEXT_SPRITE_WIDTH+TEXT_KERN)-1, 5-1,
			TEXT_SPRITE_WIDTH+2, TEXT_SPRITE_HEIGHT+2
		},
		false,
		&cb_set_song,
		NULL, &__song_down,
		(SDL_Colour){ 0x00, 0x80, 0x00, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
	);

	// Left-Side Instrument Buttons
	for (int i=0; i<g_music->ins_count; i++) {
		Music_Instrument *ins = &g_music->instruments[i];
		Button_Create(
			(struct SDL_Rect){
				5, 5 + INS_BOX_HEIGHT*i,
				100, (TEXT_SPRITE_HEIGHT + TEXT_KERN),
			},
			false,
			&cb_select_ins,
			NULL, ins,
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x00 },
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
		);

		// Create envelope sliders
		Slider_Create(5 + 20, 5 + INS_BOX_HEIGHT*i + 18, 100-20, SLIDER_HORIZONTAL, &g_music->instruments[i].wave_mod, 1.0f);
		Slider_Create(5, 5 + INS_BOX_HEIGHT*i + 28 + 35, 100, SLIDER_HORIZONTAL, &g_music->instruments[i].env.attack, 1.0f);
		Slider_Create(5, 5 + INS_BOX_HEIGHT*i + 28 + 50, 100, SLIDER_HORIZONTAL, &g_music->instruments[i].env.decay, 1.0f);
		Slider_Create(5, 5 + INS_BOX_HEIGHT*i + 28 + 65, 100, SLIDER_HORIZONTAL, &g_music->instruments[i].env.sustain, 1.0f);
		Slider_Create(5, 5 + INS_BOX_HEIGHT*i + 28 + 80, 100, SLIDER_HORIZONTAL, &g_music->instruments[i].env.release, 1.0f);
	}

	// Right-Side Wave selector Buttons
	for (int i=0; i<5; i++) {
		__wave_IDs[i] = i;
		Button_Create(
			(struct SDL_Rect){
				g_screen_width-10-13*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 9*(TEXT_KERN+TEXT_SPRITE_HEIGHT) + i*16,
				16 + 10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 16
			},
			false,
			&cb_select_wave,
			NULL, &__wave_IDs[i],
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x00 },
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x20 },
			(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0x40 }
		);
	}

	// Create an oscilloscope for the test channel
	__osc_rect = SDL_malloc(sizeof(SDL_Rect));
	*__osc_rect = (struct SDL_Rect){
		0,
		g_screen_height - OSC_HEIGHT,
		OSC_WIDTH, OSC_HEIGHT
	};
	Synth_CreateChannelOscilloscope(g_synth, g_channel_id, g_renderer,
		__osc_rect,
		(SDL_Colour){ 0x20, 0xFF, 0x40, 0xFF },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF },
		(SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
	);

	// Create an oscilloscope for the mix output
	__osc_rect = SDL_malloc(sizeof(SDL_Rect));
	*__osc_rect = (struct SDL_Rect){
		g_screen_width - OSC_WIDTH,
		g_screen_height - OSC_HEIGHT,
		OSC_WIDTH, OSC_HEIGHT
	};
	Synth_CreateMixOscilloscope(g_synth, g_renderer,
		__osc_rect,
		(SDL_Colour){ 0x20, 0xFF, 0x40, 0xFF },
		(SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF },
		(SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
	);
}



void UI_draw() {
	char buf[256];

	// Print Current Song Number
	SDL_snprintf(buf, 256, "\x80 \x81 Song Nr. %i", g_curr_song);
	Text_Draw(buf, 256, 5 + INS_BOX_WIDTH + 5, 5);

	// Draw Instrument Info
	for (int i=0; i<g_music->ins_count; i++) {
		Music_Instrument ins = g_music->instruments[i];
		SDL_snprintf(buf, 256, "Instrument %i:", i);
		Text_Draw(buf, 256, 5+1, 5 + INS_BOX_HEIGHT*i+1);
		Sprite_Draw(wave_buttons_spsh, 5, 5 + INS_BOX_HEIGHT*i + TEXT_KERN + TEXT_SPRITE_HEIGHT, ins.wave);
		
		// Draw Envelope
		int att_w = ins.env.attack * 25.0f;
		int dec_w = ins.env.decay * 25.0f;
		int sus_h = ins.env.sustain * 25.0f;
		int rel_w = ins.env.release * 25.0f;
		SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderDrawLine(g_renderer, 5, 35+25 + INS_BOX_HEIGHT*i, 5 + att_w, 35+25 + INS_BOX_HEIGHT*i - 25);
		SDL_RenderDrawLine(g_renderer, 5 + att_w, 35+25 + INS_BOX_HEIGHT*i - 25, 5 + att_w + dec_w, 35+25 + INS_BOX_HEIGHT*i - sus_h);
		SDL_RenderDrawLine(g_renderer, 5 + att_w + dec_w, 35+25 + INS_BOX_HEIGHT*i - sus_h, 5 + att_w + dec_w + 25, 35+25 + INS_BOX_HEIGHT*i - sus_h);
		SDL_RenderDrawLine(g_renderer, 5 + att_w + dec_w + 25, 35+25 + INS_BOX_HEIGHT*i - sus_h, 5 + att_w + dec_w + 25 + rel_w, 35+25 + INS_BOX_HEIGHT*i);
	}

	// Draw interface
	Text_Draw("Save", 5, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10);
	Text_Draw("Play", 5, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 1*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	Text_Draw("Stop", 5, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 2*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	Text_Draw("New Song", 9, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 3*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	Text_Draw("Del. Song", 9, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 4*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	Text_Draw("New Ins.", 9, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 5*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	Text_Draw("Del. Ins.", 9, g_screen_width-10-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 6*(TEXT_KERN+TEXT_SPRITE_HEIGHT));

	Text_Draw("Select Wave:", 13, g_screen_width-10-13*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 8*(TEXT_KERN+TEXT_SPRITE_HEIGHT));
	for (int i=0; i<5; i++) {
		char *name = (char *) Synth_GetWaveformName(i);
		Text_Draw(name, 10, g_screen_width-10-16-10*(TEXT_KERN+TEXT_SPRITE_WIDTH), 10 + 9*(TEXT_KERN+TEXT_SPRITE_HEIGHT) + i*16 + 3);
		Sprite_Draw(wave_buttons_spsh, g_screen_width-10-16, 10 + 9*(TEXT_KERN+TEXT_SPRITE_HEIGHT) + i*16, i);
	}


	Button_DrawAll();
	Slider_DrawAll();
	Synth_DrawOscilloscopes(g_synth, g_renderer);
}

void UI_cleanup() {
	SDL_free(__osc_rect);
}