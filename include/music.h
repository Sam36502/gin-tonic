#ifndef GT_MUSIC_H
#define GT_MUSIC_H
//	
//				Music Utilities
//	
//		Builds on top of my synth library and includes
//		loading instruments and music sequences from a file,
//		as well as playing them back
//	
//		(Hierarchy-Level: Top)
//	

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "synth.h"
#include "datablock.h"


//	
//		Constant Definitions
//	

#define MIDI_TUNING 440.0   // Tuning of A4 in Hertz
#define MIDI_A0 21          // MIDI Note Number of A0 (lowest supported note)
#define MIDI_SEMITONE 1.05946309436

// Song Data Instructions
#define SONG_END 	0x00	// END(); End of the song data
#define SONG_WAIT	0x01	// WAIT(t); Wait `t` many ticks before continuing
#define SONG_NOTE	0x02	// NOTE(n); Set channel frequency to note `n`
#define SONG_VOL	0x03	// VOL(v); Set channel volume to value `v`
#define SONG_TRIG	0x04	// TRIG(); Trigger the envelope
#define SONG_REL	0x05	// REL(); Release the envelope
#define SONG_TICK	0x06	// TICK(t); Set tick duration in ms
#define SONG_INS	0x07	// INS(ih, il); Set instrument for this channel to the id provided (high byte, low byte)
#define SONG_MOD	0x08	// MOD(val); Set current instruments mod value
//#define SONG_	0x08	// MOD(val); Set current instruments mod value

// Datablock Block-Types
#define SONG_DB_HEADER		0x0000
#define SONG_DB_INSTRUMENT	0x0001
#define SONG_DB_SONG_HEAD	0x0002
#define SONG_DB_SONG_DATA	0x0003
#define SONG_DB_SONG_META	0x0004


//	
//		Type Definitions
//	

typedef int Instrument_ID;
typedef int Song_ID;

typedef struct {
	Instrument_ID id;
	Synth_Wave wave;
	float wave_mod;
	Synth_Envelope env;
} Music_Instrument;

typedef struct {
	Instrument_ID ins_id;
	Uint16 timer;
	Uint16 data_offset;
	Uint8 *data;
	size_t data_len;
	int channel_id;
} Song_Channel;

typedef struct {
	Song_ID id;
	Uint8 tick_ms;	// tick length in ms
	int num_channels;
	Song_Channel *channels;
} Music_Song;

typedef struct {
	Music_Instrument *instruments;
	Uint16 ins_count;
	Music_Song *songs;
	Uint16 song_count;
	Song_ID curr_song_id;
	Song_ID next_song_id;
	SDL_TimerID timer_id;
	bool is_playing;
	bool loop_current;
	Datablock_File *dbf;
} Music_File;

//	
//		Global Variable Declarations
//	

extern Synth *g_synth;

//	
//		Function Declarations
//	

//	Initialises the music system
//	
//	Sets up a `synth.h` Synth on the default device and gets ready to
//	be able to load `Music_File`s to play.
//	
//	The `num_channels` parameter sets how many channels the synth can
//	have and will limit what can be played on it. If a song being played
//	requests more channels than are available, it will simply play without
//	those channels, which will therefore be silent.
void Music_Init(int num_channels);

//	Terminates the music system
//	
void Music_Term();

//	Allocates a free synth channel and returns its ID
//	
//	Note: Make sure to free the channel again with `Music_ChannelFree()`
//	if you don't want a channel-leak. (lol)
int Music_ChannelAlloc();

//	Frees an allocated synth channel
//	
void Music_ChannelFree(int ch);

//	Opens a music file
//	
//	Music files contain a list of songs and instruments. You can use
//	the various `Music_File` functions to then play and queue songs.
Music_File *Music_File_Open(char *music_filename);

//	Create a new empty music file
//	
Music_File *Music_File_Create(char *music_filename);

//	Closes a music file
//	
void Music_File_Close(Music_File *mf);

//	Create a blank new song
//	
//	Appends the new song the provided music file.
//	Returns the index/ID of the newly created song.
int Music_File_CreateSong(Music_File *mf, int channels);

//	Queues up a song to be played next
//	
//	This allows for more seamless transitions to the next song, as
//	the system automatically prepares to continue playing music.
//	Only one song can be queued, so if this function is called more
//	than once before the queue moves on, only the last will be played next.
//	
//	If no song is queued after the current one, the song will loop if `loop_current`
//	is set, but otherwise will set `is_playing` to false and stop all music.
void Music_File_Queue(Music_File *mf, Song_ID sid);

//	Plays a song immediately
//	
//	Plays the provided song immediately, as long as nothing else was already playing.
void Music_File_Play(Music_File *mf, Song_ID sid);

//	Stops whatever is playing
//	
//	This is how songs are ended normally. If there is a next song queued
//	this will skip straight to it. If the file is set to loop, the song
//	will start again. This function is called automatically at the end
//	of a songs data (SNG_END);
void Music_File_End(Music_File *mf);


//	Stops the file playing whatever it's playing
//	
//	Unlike `Music_File_EndSong()`, this function immediately stops
//	whatever is playing on the current file and clears the queue.
void Music_File_Stop(Music_File *mf);

//	Saves a music file to disk
//	
//	Saves `mf` in the same data-block format that it reads from
//	to the data-block file contained in mf.
void Music_File_Save(Music_File *mf);

#endif
