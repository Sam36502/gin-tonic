//	
//	Compiles a textual representation of the song data to hex
//	and inserts it into the selected song and channel
//	

#include "global.h"

Uint8 parse_pitch(char *arg);
Uint8 parse_time(char *arg);
Uint8 parse_BPM(char *arg);

static int __curr_line = 0;

void log_error(char *msg);

void parse_instruction(Uint8 *data, int *data_offset, char *cmd, int argc, char *args[5]);

void write_data_to_song(Music_Song *song, int ch_num, Uint8 *data, int len, int ins_id);

void compile_song_data(Music_File *mf, char *src_file) {
	if (mf == NULL) return;
	int sid = Music_File_CreateSong(mf, 0);
	g_curr_song = sid;
	Music_Song *song = &mf->songs[g_curr_song];

	// Parse file and compile to data
	int curr_ch_num = -1;
	int curr_def_ins = -1;
	Uint8 *song_data = SDL_malloc(sizeof(Uint8) * UINT16_MAX);
	int index = 0;
	FILE *f = fopen(src_file, "r");

	int field_i = 0;
	int arg_c = 0;
	char cmd[32];
	cmd[0] = '\0';
	char **args = SDL_malloc(sizeof(char *) * 5);
	for (int i=0; i<5; i++) args[i] = SDL_malloc(sizeof(char) * 32);
	enum {
		LINE_START,
		CMD,
		SPACE,
		ARG,
		LINE_END,
	} state = LINE_START;
	bool comment = false;
	while (!feof(f)) {
		char c = fgetc(f);
		
		// Check for comments
		if (c == ';') {
			state = LINE_END;
			comment = true;
		}

		if (c == '\n') {
			comment = false;
			if (state == CMD) {
				cmd[field_i] = '\0';
				field_i = 0;
			}
			if (state == ARG) {
				args[arg_c][field_i] = '\0';
				field_i = 0;
				arg_c++;
			}
			state = LINE_END;
		}

		if (comment) continue;

		switch (state) {

			case LINE_START: {
				if (!SDL_isspace(c)) {
					field_i = 0;
					arg_c = 0;
					cmd[field_i++] = c;
					state = CMD;
				}
			} break;

			case CMD: {
				if (SDL_isspace(c)) {
					cmd[field_i] = '\0';
					field_i = 0;
					state = SPACE;
				} else {
					cmd[field_i++] = c;
				}
			} break;

			case SPACE: {
				if (!SDL_isspace(c)) {
					field_i = 0;
					args[arg_c][field_i++] = c;
					state = ARG;
				}
			} break;

			case ARG: {
				if (SDL_isspace(c)) {
					args[arg_c][field_i] = '\0';
					field_i = 0;
					arg_c++;
					state = SPACE;
					if (arg_c >= 5) state = LINE_END;
				} else {
					args[arg_c][field_i++] = c;
				}
			} break;

			case LINE_END: {
				if (cmd[0] != 0) {
					// Check for dot-directives
					bool was_dot = false;
					if (strcmp(".CH", cmd) == 0) {
						if (index > 0 && curr_ch_num >= 0) write_data_to_song(song, curr_ch_num , song_data, index, curr_def_ins);
						curr_ch_num = strtol(args[0], NULL, 0);
						index = 0;
						was_dot = true;
					}
					if (strcmp(".DINS", cmd) == 0) {
						curr_def_ins = strtol(args[0], NULL, 0);
						was_dot = true;
					}
					if (strcmp(".DTICK", cmd) == 0) {
						song->tick_ms = parse_BPM(args[0]);
						was_dot = true;
					}
					if (strcmp(".PLAY", cmd) == 0) {
						Uint8 note = parse_pitch(args[0]);
						Uint8 wait = parse_time(args[1]);
						Uint8 wait_on = wait/4*3 + (wait & 0b1);
						Uint8 wait_off = wait/4;
						Uint8 data[8] = {
							SONG_NOTE, note,
							SONG_TRIG,
							SONG_WAIT, wait_on,
							SONG_REL,
							SONG_WAIT, wait_off,
						};
						SDL_memcpy(song_data + index, data, 8);
						index += 8;
						was_dot = true;
					}
					if (strcmp(".PLSTAC", cmd) == 0) {
						Uint8 note = parse_pitch(args[0]);
						Uint8 wait = parse_time(args[1]);
						Uint8 wait_on = wait/2 + (wait & 0b1);
						Uint8 wait_off = wait/2;
						Uint8 data[8] = {
							SONG_NOTE, note,
							SONG_TRIG,
							SONG_WAIT, wait_on,
							SONG_REL,
							SONG_WAIT, wait_off,
						};
						SDL_memcpy(song_data + index, data, 8);
						index += 8;
						was_dot = true;
					}
					if (strcmp(".PLLEGA", cmd) == 0) {
						Uint8 note = parse_pitch(args[0]);
						Uint8 wait = parse_time(args[1]);
						Uint8 data[5] = {
							SONG_NOTE, note,
							SONG_TRIG,
							SONG_WAIT, wait,
						};
						SDL_memcpy(song_data + index, data, 5);
						index += 5;
						was_dot = true;
					}

					if (!was_dot) parse_instruction(song_data, &index, cmd, arg_c, (char **) args);
				}
				cmd[0] = '\0';
				state = LINE_START;
				__curr_line++;
			} break;
		}
	}
	fclose(f);

	if (index > 0) write_data_to_song(song, curr_ch_num , song_data, index, curr_def_ins);

	for (int i=0; i<5; i++) SDL_free(args[i]);
	SDL_free(args);
}

void log_error(char *msg) {
	printf("[ERROR] Parsing '%s' at line %i: %s\n", g_asm_file_name, __curr_line, msg);
}

Uint8 parse_pitch(char *arg) {
	int l = SDL_strlen(arg);
	if (l != 3) return strtol(arg, NULL, 0);

	// Get to octave range
	int oct = arg[2] - '0';
	Uint8 note_num = 12 + oct * 12;

	switch (arg[0]) {
		case 'C': note_num += 0; break;
		case 'D': note_num += 2; break;
		case 'E': note_num += 4; break;
		case 'F': note_num += 5; break;
		case 'G': note_num += 7; break;
		case 'A': note_num += 9; break;
		case 'B': note_num += 11; break;
	}

	if (arg[1] == '#') note_num++;
	if (arg[1] == 'b') note_num--;

	return note_num;
}

Uint8 parse_time(char *arg) {
	char *end;
	Uint8 time = strtol(arg, &end, 10);

	char unit = SDL_tolower(*end);
	int factor = 1;
	switch (unit) {
		case 'w': factor = 16; break;
		case 'h': factor =  8; break;
		case 'q': factor =  4; break;
		case 'e': factor =  2; break;
		case 's': factor =  1; break;
	}

	return time * factor;
}

Uint8 parse_BPM(char *arg) {
	char *end;
	int bpm = strtol(arg, &end, 0);
	if (strcmp(end, "BPM") != 0) {
		return bpm; // Not followed by 'BPM'; interpret as ms/t instead
	}

	double msps = ( 1/( (double)(bpm)/60 ) )/4 * 1000;
	Uint8 tick_ms = round(msps);
	double new_bpm = 1/((double)(tick_ms)/1000*4) * 60;

	printf("Parsed BPM '%s' to %ims/t (%2.4f BPM)!\n", arg, tick_ms, new_bpm); fflush(stdout);
	return tick_ms;
}

void parse_instruction(Uint8 *data, int *data_offset, char *cmd, int argc, char *args[5]) {

	if (strcmp(cmd, "END") == 0) {
		if (argc > 0) log_error("Extra arguments after 'END'; ignored.");
		data[(*data_offset)++] = SONG_END;
		return;
	}

	if (strcmp(cmd, "WAIT") == 0) {
		if (argc > 1) log_error("Extra arguments after 'WAIT'; ignored.");
		int time = 1;
		if (argc < 1) {
			log_error("'WAIT' requires a time argument! Defaulting to 1.");
		} else {
			time = parse_time(args[0]);
		}
		data[(*data_offset)++] = SONG_WAIT;
		data[(*data_offset)++] = time;
		return;
	}

	if (strcmp(cmd, "NOTE") == 0) {
		if (argc > 1) log_error("Extra arguments after 'NOTE'; ignored.");
		int pitch = 60;
		if (argc < 1) {
			log_error("'NOTE' requires a pitch argument! Defaulting to C-4.");
		} else {
			pitch = parse_pitch(args[0]);
		}
		data[(*data_offset)++] = SONG_NOTE;
		data[(*data_offset)++] = pitch;
		return;
	}

	if (strcmp(cmd, "VOL") == 0) {
		if (argc > 1) log_error("Extra arguments after 'VOL'; ignored.");
		int vol = 127;
		if (argc < 1) {
			log_error("'VOL' requires a volume argument! Defaulting to 127 (50%%).");
		} else {
			vol = strtol(args[0], NULL, 0);
		}
		data[(*data_offset)++] = SONG_VOL;
		data[(*data_offset)++] = vol;
		return;
	}

	if (strcmp(cmd, "TRIG") == 0) {
		if (argc > 0) log_error("Extra arguments after 'TRIG'; ignored.");
		data[(*data_offset)++] = SONG_TRIG;
		return;
	}

	if (strcmp(cmd, "REL") == 0) {
		if (argc > 0) log_error("Extra arguments after 'REL'; ignored.");
		data[(*data_offset)++] = SONG_REL;
		return;
	}

	if (strcmp(cmd, "TICK") == 0) {
		if (argc > 1) log_error("Extra arguments after 'TICK'; ignored.");
		int ms = 125;
		if (argc < 1) {
			log_error("'TICK' requires a tick-ms/BPM argument! Defaulting to 125 (120BPM).");
		} else {
			ms = parse_BPM(args[0]);
		}
		data[(*data_offset)++] = SONG_TICK;
		data[(*data_offset)++] = ms;
		return;
	}

	if (strcmp(cmd, "INS") == 0) {
		if (argc > 1) log_error("Extra arguments after 'INS'; ignored.");
		Uint16 ins = 0;
		if (argc < 1) {
			log_error("'INS' requires an instrument-ID argument! Defaulting to 0.");
		} else {
			ins = strtol(args[0], NULL, 0);
		}
		data[(*data_offset)++] = SONG_INS;
		data[(*data_offset)++] = ins >> 8;
		data[(*data_offset)++] = ins & 0xFF;
		return;
	}

	if (strcmp(cmd, "MOD") == 0) {
		if (argc > 1) log_error("Extra arguments after 'MOD'; ignored.");
		int mod = 0;
		if (argc < 1) {
			log_error("'MOD' requires a mod-value argument! Defaulting to 0...");
		} else {
			mod = strtol(args[0], NULL, 0);
		}
		data[(*data_offset)++] = SONG_MOD;
		data[(*data_offset)++] = mod;
		return;
	}

	log_error("Unknown instruction provided:");
	printf("  CMD: '%s'", cmd);
	for (int i=0; i<argc; i++) {
		printf(", Arg. %i: '%s'", i, args[i]);
	}
	putchar('\n');
	fflush(stdout);
}

void write_data_to_song(Music_Song *song, int ch_num, Uint8 *data, int len, int ins_id) {

	// Add channel to song if it doesn't already exist
	if (ch_num >= song->num_channels) {
		ch_num = song->num_channels;
		song->num_channels++;
		song->channels = SDL_realloc(song->channels, sizeof(Song_Channel) * song->num_channels);
	}
	Song_Channel *ch = &song->channels[ch_num];
	ch->channel_id = -1;
	ch->data_offset = 0;
	if (ins_id != -1) ch->ins_id = ins_id;
	else ch->ins_id = 0;
	
	// Write new data to channel
	ch->data = SDL_malloc(sizeof(Uint8) * len);
	ch->data_len = len;
	SDL_memcpy(ch->data, data, ch->data_len);

}