//	
//	Tool to convert MIDI files to music files
//	

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// Gin-Tonic
#include <music.h>
#include <midi.h>
#include "global.h"

void print_midi_debug(MIDI_File *midi);


void convert_channel(Music_Song *song, MIDI_Track trk) {

}

bool convert_midi(Music_File *mf, char *midi_filename) {
	// Load MIDI
	MIDI_File *midi = MIDI_OpenFile(g_midi_file_name);
	if (midi == NULL) {
		printf("Failed to load midi file '%s'. Exiting....\n", g_midi_file_name);
		return false;
	}

	print_midi_debug(midi);

	// Check number of channels
	//int num_channels = 0;
	//int ch_trk_index[num_channels];
	for (int i=0; i<midi->header.num_tracks; i++) {
		MIDI_Track trk = midi->tracks[i];
		bool has_midi = false;
		for (int j=0; j<trk.length; j++) {
			MIDI_Event evt = trk.events[j];
			if (evt.type == MIDI_EVENT_TYPE_MIDI) {
				has_midi = true;
				break;
			}
		}
		if (has_midi) {
			//ch_trk_index[num_channels++] = i;
		}
	}

	// Create new song
	//int new_song_id = Music_File_CreateSong(g_music, num_channels);
	//Music_Song *song = &mf->songs[new_song_id];
	int tick_ms = roundf((float)(midi->uspq/4)/1000);
	float orig_tempo = 1 / (((float) midi->uspq) / 1000000) * 60; 
	float new_tempo = 1 / ((float)(tick_ms)/1000 * 4) * 60;

	printf("Original File Tempo: %0.2f BMP\n", orig_tempo);
	printf("   After Conversion: %0.2f BMP\n", new_tempo);
	
	// Convert MIDI Data for each channel
	//for (int i=0; i<num_channels; i++) {
	//	int track_i = ch_trk_index[i];
	//	MIDI_Track trk = midi->tracks[track_i];
	//	convert_channel(song, trk);
	//}

	// Termination
	MIDI_CloseFile(midi);
	//Music_File_Save(mf);
	return true;
}

void print_midi_debug(MIDI_File *midi) {
	puts("\n  MIDI-File Information:");
	puts("-------------------------");
	printf("       Timing: %ius/q\n", midi->uspq);
	printf("       Format: %s\n", MIDI_GetFormatName(midi->header.format));
	printf("  # of Tracks: %i\n", midi->header.num_tracks);
	printf("  Pulse per q: %i\n", midi->header.ppq);
	printf("       Tracks:\n");
	for (int i=0; i<midi->header.num_tracks; i++) {
		MIDI_Track trk = midi->tracks[i];
		printf("    [%i] Num. Events: %i, Length: %iB\n", i, trk.count, trk.length);
		bool end_of_track = false;
		for (int j=0; j<trk.count && !end_of_track; j++) {
			MIDI_Event evt = trk.events[j];
			printf("      Δt = %4i: ", evt.delta_time);
		
			if (evt.type == MIDI_EVENT_TYPE_META) {
				MIDI_MetaEvent m = evt.meta;
				printf("[META] ");
				bool print_string = false;
				switch (m.type) {
					case MIDI_META_SEQ_NUM: printf("Sequence Number: 0x%02X%02X\n", m.data[0], m.data[1]); break;
					case MIDI_META_TXT: printf("Text Event: "); print_string = true; break;
					case MIDI_META_COPYRIGHT: printf("Copyright: "); print_string = true; break;
					case MIDI_META_SEQ_NAME: printf("Sequence Name: "); print_string = true; break;
					case MIDI_META_INS_NAME: printf("Instrument Name: "); print_string = true; break;
					case MIDI_META_LYRIC: printf("Lyric: "); print_string = true; break;
					case MIDI_META_MARKER: printf("Marker: "); print_string = true; break;
					case MIDI_META_CUE_PT: printf("Cue Point: "); print_string = true; break;
					case MIDI_META_CH_PREFIX: printf("MIDI Channel Prefix: 0x%02X\n", m.data[0]); break;
					case MIDI_META_PORT: printf("MIDI Port: %i\n", m.data[0]); break;
					case MIDI_META_EOT: printf("End of Track\n"); end_of_track = true; break;
					case MIDI_META_SET_TEMPO: printf("Tempo: (%ius/q)\n", m.data[0] << 16 | m.data[1] << 8 | m.data[2]); break;
					case MIDI_META_SMPTE_OFFS: printf("SMPTE Offset: <TODO>\n"); break;
					case MIDI_META_TIME_SIG: {
						int den = 1;
						for (int d=0; d<m.data[1]; d++) den*=2;
						printf("Time Signature: %i/%i\n", m.data[0], den);
					} break;
					case MIDI_META_KEY_SIG: {
						int acc = m.data[0];
						if (acc == 0) {
							printf("Key Signature: No sharps/flats, %s\n", m.data[1] ? "Minor":"Major");
						} else {
							char *typ = "sharps";
							if (acc < 0) {
								typ = "flats";
								acc *= -1;
							}
							printf("Key Signature: %i %s, %s\n", acc, typ, m.data[1] ? "Minor":"Major");
						}
					} break;
					case MIDI_META_SEQSPEC: printf("Sequencer Specific Event\n"); break;
					default: printf("Huh? Type: 0x%02X\n", m.type); break;
				}
				if (print_string) {
					for (int c=0; c<m.length; c++) putchar(m.data[c]);
					putchar('\n');
				}
			} else if (evt.type == MIDI_EVENT_TYPE_MIDI) {
				MIDI_MidiEvent m = evt.midi;
				printf("[MIDI] CH-%02X: ", m.channel);
				switch (m.status) {
					case MIDI_STATUS_NOTE_OFF: printf("Note %s Off (Vel.: %i)\n", note_name(m.data[0]), m.data[1]); break;
					case MIDI_STATUS_NOTE_ON: {
						if (m.data[1] > 0) {
							printf("Note %s On  (Vel.: %i)\n", note_name(m.data[0]), m.data[1]);
						} else {
							printf("Note %s Off (Vel.: %i)\n", note_name(m.data[0]), m.data[1]);
						}
					} break;
					case MIDI_STATUS_POLY_PRESS: printf("Polyphonic Aftertouch, Note %s (Vel.: %i)", note_name(m.data[0]), m.data[1]); break;
					case MIDI_STATUS_CC: printf("Control Change %i: Value %i\n", m.data[0], m.data[1]); break;
					case MIDI_STATUS_PROG_CHANGE: printf("Program Change %i\n", m.data[0]); break;
					case MIDI_STATUS_CHAN_PRESS: printf("Channel Pressure: %i\n", m.data[0]); break;
					case MIDI_STATUS_PITCH_BEND: printf("Pitch Bend: %i\n", m.data[0]<<7|m.data[1]); break;
					default: printf("Huh? Status: 0x%02X\n", m.status); break;
				}
			}

		}
	}
	fflush(stdout);
}