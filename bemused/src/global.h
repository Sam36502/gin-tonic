#ifndef GLOBAL
#define GLOBAL

//	Gin-Tonic includes
#include <log.h>
#include <screen.h>
#include <synth.h>
#include <music.h>
#include <button.h>
#include <slider.h>
#include <sprite.h>
#include <text.h>
#include <events.h>


//	
//	Global Variables
//	

Music_File *g_music = NULL;
int g_curr_song = 0;
Instrument_ID g_curr_ins = 0;
int g_channel_id = -1;
bool g_loop_song = false;
bool g_new_file = false;

char *g_music_file_name = NULL;
char *g_midi_file_name = NULL;
char *g_asm_file_name = NULL;
enum {
	OPEN_EDITOR,
	SHOW_INFO,
	IMPORT_MIDI,
	IMPORT_ASM,
} g_prog_function = OPEN_EDITOR;

char *note_name(int midi_note_num);

#endif