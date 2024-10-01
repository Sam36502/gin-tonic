#ifndef GT_SYNTH_H
#define GT_SYNTH_H
//
//		Basic Sound Synthesizer
//		
//		Creates a synth with multiple channels for making
//		sounds. Pair with `music.h` for best results.

//
//		INCLUDES
//

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

//
//		DEFINITIONS
//

#define MAX_CHANNELS 64
#define MAX_FILTERS 64
#define TAU 6.283185f

// See synth.c Synth_OpenSynth() for default audio specifications

//
//		TYPES
//

typedef enum Synth_EnvState {
	SYNTH_ENV_HOLD,	 // Doubles as waiting before triggering and sustain states
	SYNTH_ENV_ATTACK,
	SYNTH_ENV_DECAY,
	SYNTH_ENV_RELEASE,
} Synth_EnvState;

typedef enum Synth_Wave {
	SYNTH_WAVE_SINE,
	SYNTH_WAVE_PULSE,
	SYNTH_WAVE_TRI,
	SYNTH_WAVE_SAW,
	SYNTH_WAVE_NOISE,
} Synth_Wave;

typedef enum Synth_FilterType {
	SYNTH_FILT_NONE,
	SYNTH_FILT_BITCRUSH,
	SYNTH_FILT_TIMECRUSH,
	SYNTH_FILT_LOW_PASS,
	SYNTH_FILT_HIGH_PASS,
} Synth_FilterType;

typedef struct Synth_Envelope {
	float attack;	// Attack time in seconds
	float decay;	// Decay time in seconds
	float sustain;	// Sustain volume in percent
	float release;	// Release time in seconds
} Synth_Envelope;

typedef struct Synth_Filter {
	float amount;
	float (* filter_func)(float sample, float amount, void *udata);
	void *udata;
} Synth_Filter;

typedef struct Synth_Oscilloscope {
	SDL_Color bg_colour;
	SDL_Color beam_colour;
	SDL_Color border_colour;
	SDL_Rect *rect;
	int offset;
	float *sample_buffer;
} Synth_Oscilloscope;

typedef struct Synth_Channel {
	float phase;
	float freq;
	float volume;
	float env_vol;
	float wave_mod;
	bool should_release;
	Synth_Oscilloscope *osc;
	Synth_Wave waveform;
	Synth_Envelope env_values;
	Synth_EnvState env_state;
	//Synth_Filter *filter_path[MAX_FILTERS];
} Synth_Channel;

typedef struct Synth {
	int num_channels;
	float volume;
	SDL_AudioDeviceID dev_id;
	SDL_AudioSpec spec;
	Synth_Oscilloscope *osc;
	Synth_Channel *channels[MAX_CHANNELS];
} Synth;

//
//		PUBLIC FUNCTIONS
//

//	Opens a new synth with a given audio device ID
//	
//	Pass `NULL` as dev_id to let SDL select a reasonable default device.
//	Returns `NULL` if it failed
Synth *Synth_OpenSynth(int num_channels, const char *dev_id);

//	Closes a previously opened synth
//
void Synth_CloseSynth(Synth *synth);

//	Returns a null-terminated name string for a given waveform
//
const char* Synth_GetWaveformName(Synth_Wave wave);


////	Synth Getters/Setters (Necessary?)


//	Sets the master volume for a synth
//	
//	The float volume value ranges from 0.0 to 1.0.
//	A synth's master volume is set to 0.5 by default.
void Synth_SetMasterVolume(Synth *synth, float volume);

//	Returns the current master volume of a synth
//
//	The float volume value ranges from 0.0 to 1.0.
float Synth_GetMasterVolume(Synth *synth);

//	Stops all channels from playing
//	
void Synth_StopAllChannels(Synth *synth);

//	Sets the volume of a channel
//	
//	The float volume value ranges from 0.0 to 1.0.
//	A channel's volume is set to 1.0 by default.
//	Set the channel's volume to 0.0 to disable it.
void Synth_SetChannelVolume(Synth *synth, int ch, float volume);

//	Returns the current volume of a given channel
//
//	The float volume value ranges from 0.0 to 1.0.
float Synth_GetChannelVolume(Synth *synth, int ch);

//	Sets the frequency of a channel
//	
//	A channel's frequency is set to 1000 by default.
void Synth_SetChannelFrequency(Synth *synth, int ch, float frequency);

//	Returns the current frequency of a given channel
//
float Synth_GetChannelFrequency(Synth *synth, int ch);

//	Sets the waveform of a channel
//	
//	The wave option selects which wave form the channel should use when
//	generating audio from a list of provided functions.
//	This is set to SYNTH_WAVE_SINE by default.
//	The mod option is for fine-tuning wave properties, but most
//	wave functions ignore it. Here are the properties it alters:
//
//		SYNTH_WAVE_PULSE	Sets the duty cycle in percent. 0.0 defaults to 0.5
//		SYNTH_WAVE_SAW	Sets the saw to be a falling wave instead of rising if mod != 0.0
void Synth_SetChannelWaveform(Synth *synth, int ch, Synth_Wave wave, float mod);

//	Returns the currently selected waveform of a channel
//
//	Passing `NULL` to mod is fine if you don't need it.
Synth_Wave Synth_GetChannelWaveform(Synth *synth, int ch, float *mod);

//	Sets a filter in the filter path for a channel
//
//	If NULL is passed for the filter, the filter is reset and ignored.
//	A Channel's filter-path is all NULLs by default.
void Synth_SetChannelFilter(Synth *synth, int ch, int filter_id, Synth_FilterType filter_type);


////	Envelope Functions


//	Sets the envelope values of a channel's envelope-generator
//	
//	The Synth_Envelope struct has following values:
//		float attack;	 // Attack time in seconds (Default 0.0)
//		float decay;	// Decay time in seconds (Default 0.0)
//		float sustain;	// Sustain volume in percent (Default 1.0)
//		float release;	// Release time in seconds (Default 0.0)
void Synth_SetChannelEnvelope(Synth *synth, int ch, Synth_Envelope env);

//	Returns the envelope settings for a given channel
//
Synth_Envelope Synth_GetChannelEnvelope(Synth *synth, int ch);

//	Triggers a channel envelope to begin
//
//	If the envelope was already in the middle of a cycle,
//	this resets it to the attack state and builds to max volume again.
void Synth_TriggerChannelEnvelope(Synth *synth, int ch);

//	Triggers a channel envelope to release
//
void Synth_ReleaseChannelEnvelope(Synth *synth, int ch);


////	Oscilloscope Functions


//	Creates an oscilloscope for one of the synth's channels to display on the screen
// 
//	This will automatically draw over itself as long as there is sound playing.
//	Rect is the box on the screen it should draw the oscilloscope inside of.
//	Set border's alpha to 0 for no border;
void Synth_CreateChannelOscilloscope(
	Synth *synth, int ch,
	SDL_Renderer *renderer, SDL_Rect *rect,
	SDL_Color beam, SDL_Color border, SDL_Color bg
);

//	Removes a previously created oscilloscope on a channel
//
void Synth_RemoveChannelOscilloscope(Synth *synth, int ch);

//	Creates an oscilloscope for the synth's main mix output
// 
//	This will automatically draw over itself as long as there is sound playing.
//	Rect is the box on the screen it should draw the oscilloscope inside of.
//	Set border's alpha to 0 for no border;
void Synth_CreateMixOscilloscope(
	Synth *synth,
	SDL_Renderer *renderer, SDL_Rect *rect,
	SDL_Color beam, SDL_Color border, SDL_Color bg
);

//	Removes a previously created oscilloscope for the synth
//
void Synth_RemoveMixOscilloscope(Synth *synth);

//	Draws all the oscilloscopes for a given synth
//
void Synth_DrawOscilloscopes(Synth *synth, SDL_Renderer *renderer);


////	Music File Functions


//
//		INTERNAL FUNCTIONS
//

//	Wave Functions
float __wave_sine(float phase);
float __wave_pulse(float phase, float mod);
float __wave_saw(float phase, float mod);
float __wave_triangle(float phase, float mod);
float __wave_noise(float phase, float mod);

//	Filter Functions
float __filter_bitcrush(float sample, float mod, void *udata);
float __filter_timecrush(float sample, float mod, void *udata);
float __filter_lowpass(float sample, float mod, void *udata);
float __filter_highpass(float sample, float mod, void *udata);

//	Filter Constructors
Synth_Filter *__filter_create_bitcrush();
Synth_Filter *__filter_create_timecrush();
Synth_Filter *__filter_create_lowpass();
Synth_Filter *__filter_create_highpass();

//	Called by SDL whenever the audio device needs more samples
//	
void __audio_callback(void *_synth, Uint8 *_buffer, int size);

//	Creates a new channel
//	
Synth_Channel *__create_channel(Synth *synth);

//	Called in the audio callback to progress the envelope one step
//
void __progress_envelope(Synth *synth, Synth_Channel *chan);

//	Renders a single Oscilloscope
//	
void __draw_oscilloscope(Synth *synth, SDL_Renderer *renderer, Synth_Oscilloscope *osc);

#endif