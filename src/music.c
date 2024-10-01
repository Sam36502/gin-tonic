#include "../include/music.h"

Synth *g_synth;

static bool __channel_map[MAX_CHANNELS]; // true = free
static int __max_channels = 0;

int Music_ChannelAlloc() {
	for (int i=0; i<__max_channels; i++) {
		if (__channel_map[i]) {
			__channel_map[i] = false;
			return i;
		}
	}

	Log_Message(LOG_WARNING, "No Free Channels");
	return __max_channels-1;
}

void Music_ChannelFree(int ch) {
	__channel_map[ch] = true;
}

void Music_Init(int num_channels) {
	if (g_synth != NULL) {
		Log_Message(LOG_WARNING, "Tried to re-initialise music system");
		return;
	}

	__max_channels = num_channels;
	for (int i=0; i<__max_channels; i++) __channel_map[i] = true;

	g_synth = Synth_OpenSynth(num_channels, NULL);
	if (g_synth == NULL) {
		Log_Message(LOG_ERROR, "Failed to open Synth");
	}
}

void Music_Term() {
	Synth_CloseSynth(g_synth);
	g_synth = NULL;
	__max_channels = 0;
}

//	Music File Format:
//	
//	Header:			Block 0x0000 = [2B] num songs, [2B] num instruments;
//	Instrument:		Block 0x0001 = [2B] ins ID, [1B] wave type, [1B] wave_mod, [4B] attack, [4B] decay, [4B] sustain, [4B] release;
//	Song Header:	Block 0x0002 = [2B] song ID, [1B] ms per tick, [1B] num chans, [2nB] instrument-id per channel;
//	Song Data:		Block 0x0003 = [2B] song ID, [1B] chan nr, [nB] song data;
//	Song Metadata:	Block 0x0004 = [2B] song ID, [1B] name len, [nB] song name, [1B] author len, [nB] author

Music_File *Music_File_Open(char *music_filename) {
	Datablock_File *dbf = Datablock_File_Open(music_filename);
	if (dbf == NULL) return NULL;

	Music_File *mf = SDL_malloc(sizeof(Music_File));
	mf->curr_song_id = -1;
	mf->next_song_id = -1;
	mf->timer_id = -1;
	mf->is_playing = false;
	mf->loop_current = false;
	mf->dbf = dbf;

	// Parse Header
	Uint16 ins_count = 0;
	Uint16 song_count = 0;
	bool success = true;
	for (int bi=0; bi<dbf->num_blocks; bi++) {
		Datablock *db = Datablock_File_GetBlock(dbf, bi);
		if (db == NULL) success = false;
		switch (db->block_type) {

			case SONG_DB_HEADER: {
				if (db->block_length < 4) {
					Log_Message(LOG_ERROR, "Failed to read music file: Invalid header");
					success = false;
					continue;
				}
				U8_TO_U16(mf->song_count, db->data[0], db->data[1]);
				U8_TO_U16(mf->ins_count, db->data[2], db->data[3]);

				mf->instruments = SDL_malloc(sizeof(Music_Instrument) * mf->ins_count);
				mf->songs = SDL_malloc(sizeof(Music_Song) * mf->song_count);
			} break;

			case SONG_DB_INSTRUMENT: {
				if (ins_count >= mf->ins_count) {
					Log_Message(LOG_WARNING, "Music file contains more instruments than reported");
					continue;
				}
				if (db->block_length < 20) {
					Log_Message(LOG_ERROR, "Failed to read music file: Invalid instrument block");
					success = false;
					continue;
				}

				Music_Instrument ins = {
					.id = (db->data[0] << 8) | db->data[1],
					.wave = (Synth_Wave)(db->data[2]),
					.wave_mod = (float)(db->data[3]) / 255.0f,
					.env = (Synth_Envelope){ 0.0, 0.0, 0.0, 0.0 }
				};

				// Read the envelope values from memory
				Uint32 attack  = ((Uint32)(db->data[ 4]) << 24) | ((Uint32)(db->data[ 5]) << 16) | ((Uint32)(db->data[ 6]) << 8) | db->data[ 7];
				Uint32 decay   = ((Uint32)(db->data[ 8]) << 24) | ((Uint32)(db->data[ 9]) << 16) | ((Uint32)(db->data[10]) << 8) | db->data[11];
				Uint32 sustain = ((Uint32)(db->data[12]) << 24) | ((Uint32)(db->data[13]) << 16) | ((Uint32)(db->data[14]) << 8) | db->data[15];
				Uint32 release = ((Uint32)(db->data[16]) << 24) | ((Uint32)(db->data[17]) << 16) | ((Uint32)(db->data[18]) << 8) | db->data[19];
				ins.env.attack  = *(float *)( (void *)(&attack) );
				ins.env.decay   = *(float *)( (void *)(&decay) );
				ins.env.sustain = *(float *)( (void *)(&sustain) );
				ins.env.release = *(float *)( (void *)(&release) );

				mf->instruments[ins_count++] = ins;
			} break;

			case SONG_DB_SONG_HEAD: {
				if (song_count >= mf->song_count) {
					Log_Message(LOG_WARNING, "Music file contains more song-headers than reported");
					continue;
				}
				if (db->block_length < 6) {
					Log_Message(LOG_ERROR, "Failed to read music file: Invalid song-header block");
					success = false;
					continue;
				}

				Music_Song song = {
					.id = (db->data[0] << 8) | db->data[1],
					.tick_ms = db->data[2],
					.num_channels = db->data[3],
				};

				song.channels = SDL_malloc(sizeof(Song_Channel) * song.num_channels);
				
				for (int i=0; i<song.num_channels; i++) {
					song.channels[i].ins_id = (db->data[4+2*i] << 8) | db->data[4+2*i+1];
					song.channels[i].timer = 0;
					song.channels[i].data_offset = 0;
				}
				
				mf->songs[song_count++] = song;
			} break;

			case SONG_DB_SONG_DATA: {
				if (db->block_length < 2) {
					Log_Message(LOG_ERROR, "Failed to read music file: Invalid song-data block");
					success = false;
					continue;
				}

				Song_ID song_id = (db->data[0] << 8) | db->data[1];
				Uint8 chan_id = db->data[2];
				Music_Song *song = NULL;
				for (int i=0; i<song_count; i++) {
					if (mf->songs[i].id == song_id) {
						song = &mf->songs[i];
					}
				}
				if (song == NULL) {
					Log_Message(LOG_ERROR, "Failed to read music file: Orphaned song-data block");
					success = false;
					continue;
				}

				song->channels[chan_id].data_len = db->block_length-3;
				song->channels[chan_id].data = SDL_malloc(sizeof(Uint8) * song->channels[chan_id].data_len);
				SDL_memcpy(song->channels[chan_id].data, db->data+3, song->channels[chan_id].data_len);
			} break;

		}
		Datablock_Destroy(db);
	}

	if (!success) {
		Music_File_Close(mf);
		return NULL;
	}

	return mf;
}

Music_File *Music_File_Create(char *music_filename) {
	Datablock *db_array[1];

	// Create Header Block and Create File
	Uint8 data[4] = { 0x00, 0x00, 0x00, 0x00 }; // 0 Songs, 0 Instruments
	db_array[0] = Datablock_Create(SONG_DB_HEADER, data, 4);
	Datablock_File *dbf = Datablock_File_Create(music_filename, db_array, 1);

	Music_File *mf = SDL_malloc(sizeof(Music_File));
	mf->curr_song_id = -1;
	mf->next_song_id = -1;
	mf->timer_id = -1;
	mf->is_playing = false;
	mf->loop_current = false;
	mf->dbf = dbf;

	mf->ins_count = 0;
	mf->song_count = 0;
	mf->instruments = NULL;
	mf->songs = NULL;

	return mf;
}

void Music_File_Close(Music_File *mf) {
	if (mf == NULL) return;
	if (mf->instruments != NULL) SDL_free(mf->instruments);
	if (mf->songs != NULL) SDL_free(mf->songs);
	Datablock_File_Close(mf->dbf);
	SDL_free(mf);
}

int Music_File_CreateSong(Music_File *mf, int channels) {
	mf->song_count++;
	mf->songs = SDL_realloc(mf->songs, sizeof(Music_Song) * mf->song_count);
	int new_index = mf->song_count-1;
	
	Music_Song *song = &mf->songs[new_index];
	song->id = new_index;
	song->channels = SDL_malloc(sizeof(Song_Channel) * channels);
	song->num_channels = channels;
	song->tick_ms = 125; // 120 bpm default

	// Create channels
	for (int i=0; i<channels; i++) {
		Song_Channel *ch = &song->channels[i];
		ch->channel_id = -1;
		ch->data = SDL_malloc(sizeof(Uint8) * 1);
		ch->data[0] = 0x00; // Data is just the END command
		ch->data_len = 1;
		ch->ins_id = 0x0000;
		ch->timer = 0;
	}

	return new_index;
}

void Music_File_Queue(Music_File *mf, Song_ID sid) {
	mf->next_song_id = sid;
}

// Processes one channel worth of steps up until it needs to wait
static bool __music_process(Song_Channel *ch, Music_Song *song) {
	bool end_of_song = false;
	bool done = false;

	while (!done) {
		switch (ch->data[ch->data_offset++]) {
			case SONG_END: {
				done = true;
				ch->data_offset--;
				end_of_song = true;
			} break;

			case SONG_WAIT: {
				end_of_song = false;
				ch->timer = ch->data[ch->data_offset++];
				if (ch->timer > 0) ch->timer--;
				done = true;
			} break;

			case SONG_NOTE: {
				end_of_song = false;
				Uint8 note = ch->data[ch->data_offset++];
				float freq = MIDI_TUNING / 16; // 440 (A4) / 2^4 -> (A0)
				for (int i=MIDI_A0; i<note; i++) {
					freq *= MIDI_SEMITONE;
				}
				Synth_SetChannelFrequency(g_synth, ch->channel_id, freq);
			} break;

			case SONG_VOL: {
				end_of_song = false;
				float vol = (float)(ch->data[ch->data_offset++]) / 255.0f;
				Synth_SetChannelVolume(g_synth, ch->channel_id, vol);
			} break;

			case SONG_TRIG: {
				end_of_song = false;
				Synth_TriggerChannelEnvelope(g_synth, ch->channel_id);
			} break;

			case SONG_REL: {
				end_of_song = false;
				Synth_ReleaseChannelEnvelope(g_synth, ch->channel_id);
			} break;

			case SONG_TICK: {
				end_of_song = false;
				song->tick_ms = ch->data[ch->data_offset++];
			} break;

			case SONG_INS: {
				end_of_song = false;
				ch->ins_id = ch->data[ch->data_offset++] << 8;
				ch->ins_id |= ch->data[ch->data_offset++];
			} break;

			case SONG_MOD: {
				end_of_song = false;
				float mod = (float)(ch->data[ch->data_offset++]) / 255.0f;
				Synth_Wave wav = Synth_GetChannelWaveform(g_synth, ch->channel_id, NULL);
				Synth_SetChannelWaveform(g_synth, ch->channel_id, wav, mod);
			} break;

		}
	}

	return end_of_song;
}

static Uint32 __cb_music_tick(Uint32 _, void *_mf) {
	Music_File *mf = (Music_File *)(_mf);
	if (!mf->is_playing) return 0;
	Music_Song *song = &mf->songs[mf->curr_song_id];

	// DEBUG:
	//static Uint64 last_tick = 0;
	//Uint64 now = SDL_GetTicks64();
	//Uint64 tms = now - last_tick;
	//if (last_tick != 0) printf("Music tick took %llims\n", tms);
	//fflush(stdout);
	//last_tick = now;

	bool end_of_song = true;
	for (int ci=0; ci<song->num_channels; ci++) {
		Song_Channel *ch = &song->channels[ci];
		if (ch->timer > 0) {
			ch->timer--;
			end_of_song = false;
			continue;
		}

		end_of_song = __music_process(ch, song);
	}

	// All channels finished
	if (end_of_song) {
		Music_File_End(mf);
		return 0;
	}

	return song->tick_ms;
}

void Music_File_Play(Music_File *mf, Song_ID sid) {
	if (mf->is_playing) return;
	if (sid < 0 || sid >= mf->song_count) return;
	mf->curr_song_id = sid;
	mf->is_playing = true;
	Music_Song *song = &mf->songs[sid];

	// Set up channels for the song
	for (int ci=0; ci<song->num_channels; ci++) {
		Song_Channel *ch = &song->channels[ci];
		ch->timer = 0;
		ch->data_offset = 0;
		ch->channel_id = Music_ChannelAlloc();

		Music_Instrument ins = mf->instruments[ch->ins_id];
		Synth_SetChannelWaveform(g_synth, ch->channel_id, ins.wave, ins.wave_mod);
		Synth_SetChannelEnvelope(g_synth, ch->channel_id, ins.env);
	}

	// TODO: Is 10ms consistant? Waiting 10 ms seems to be long enough to load the data without
	// impacting the rhythm too much. I need a better way of knowing when the data is ready...
	// Is it even that the data isn't ready? Asynch is a PITA.
	mf->timer_id = SDL_AddTimer(10, &__cb_music_tick, mf);
	if (mf->timer_id == 0) Log_SDLMessage(LOG_ERROR, "Failed to create music timer");
}

void Music_File_End(Music_File *mf) {
	Music_File_Stop(mf);

	// Queue next song or loop
	if (mf->loop_current) {
		Music_File_Play(mf, mf->curr_song_id);
		return;
	}

	if (mf->next_song_id >= 0) {
		Music_File_Play(mf, mf->next_song_id);
	}
}

void Music_File_Stop(Music_File *mf) {
	if (mf->curr_song_id < 0) return;
	mf->is_playing = false;
	Music_Song *song = &mf->songs[mf->curr_song_id];
	if (mf->timer_id != -1) {
		SDL_RemoveTimer(mf->timer_id);
	}

	// Free channels
	for (int ci=0; ci<song->num_channels; ci++) {
		Song_Channel ch = song->channels[ci];
		Synth_ReleaseChannelEnvelope(g_synth, ch.channel_id);
		Music_ChannelFree(ch.channel_id);
	}
}

void Music_File_Save(Music_File *mf) {

	int db_count = 1 + mf->ins_count;
	for (int i=0; i<mf->song_count; i++) {
		db_count += 1 + mf->songs[i].num_channels;
		// TODO: If I add metadata support,
		// this will have to check if a metadata block is included
	}
	Datablock *db_array[db_count];

	// Turn all the Music file data into datablocks
	int data_index = 0;
	Uint8 header_data[4];
	U16_TO_U8(mf->song_count, header_data[0], header_data[1]);
	U16_TO_U8(mf->ins_count, header_data[2], header_data[3]);
	db_array[data_index++] = Datablock_Create(SONG_DB_HEADER, header_data, 4);

	for (int i=0; i<mf->ins_count; i++) {
		Music_Instrument ins = mf->instruments[i];
		Uint8 ins_data[20];
		U16_TO_U8(ins.id, ins_data[0], ins_data[1]);
		ins_data[2] = ins.wave;
		ins_data[3] = (Uint8)(ins.wave_mod * 255.0f);
		Uint32 attack = *(Uint32 *)( (void *)(&ins.env.attack) );
		Uint32 decay = *(Uint32 *)( (void *)(&ins.env.decay) );
		Uint32 sustain = *(Uint32 *)( (void *)(&ins.env.sustain) );
		Uint32 release = *(Uint32 *)( (void *)(&ins.env.release) );
		ins_data[4] = attack >> 24; ins_data[5] = (attack >> 16) & 0xFF; ins_data[6] = (attack >> 8) & 0xFF; ins_data[7] = attack & 0xFF; 
		ins_data[8] = decay >> 24; ins_data[9] = (decay >> 16) & 0xFF; ins_data[10] = (decay >> 8) & 0xFF; ins_data[11] = decay & 0xFF; 
		ins_data[12] = sustain >> 24; ins_data[13] = (sustain >> 16) & 0xFF; ins_data[14] = (sustain >> 8) & 0xFF; ins_data[15] = sustain & 0xFF; 
		ins_data[16] = release >> 24; ins_data[17] = (release >> 16) & 0xFF; ins_data[18] = (release >> 8) & 0xFF; ins_data[19] = release & 0xFF; 
		db_array[data_index++] = Datablock_Create(SONG_DB_INSTRUMENT, ins_data, 20);
	}

	for (int i=0; i<mf->song_count; i++) {
		Music_Song song = mf->songs[i];

		// Song Headers
		int head_len = 4 + 2*song.num_channels;
		Uint8 head_data[head_len];
		U16_TO_U8(song.id, head_data[0], head_data[1]);
		head_data[2] = song.tick_ms;
		head_data[3] = song.num_channels;
		for (int ci=0; ci<song.num_channels; ci++) {
			Song_Channel ch = song.channels[ci];
			U16_TO_U8(ch.ins_id, head_data[4+2*ci], head_data[4+2*ci+1]);
		}
		db_array[data_index++] = Datablock_Create(SONG_DB_SONG_HEAD, head_data, head_len);

		// TODO: If I add song metadata support, it should go here

		// Song Data
		for (int ci=0; ci<song.num_channels; ci++) {
			Song_Channel ch = song.channels[ci];
			Uint8 song_data[3 + ch.data_len];
			U16_TO_U8(song.id, song_data[0], song_data[1]);
			song_data[2] = ci;
			SDL_memcpy(song_data + 3, ch.data, ch.data_len);
			db_array[data_index++] = Datablock_Create(SONG_DB_SONG_DATA, song_data, 3 + ch.data_len);
		}
	}

	// Calculate Checksums
	for (int i=0; i<db_count; i++) {
		Datablock_CalcSum(db_array[i]);
	}

	// Write results to file

	Datablock_File *new = Datablock_File_Create(mf->dbf->filename, db_array, db_count);
	Datablock_File_Close(mf->dbf);
	mf->dbf = new;
	

	for (int i=0; i<db_count; i++) {
		Datablock_Destroy(db_array[i]);
	}

	Log_Message(LOG_INFO, "Music File Saved Successfully");
}
