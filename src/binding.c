#include "../include/binding.h"


Binding *g_keybinds = NULL;
int g_num_keybinds = 0;
int g_cap_keybinds = 0x100;


void Binding_Init() {
	g_keybinds = SDL_malloc(sizeof(Binding) * g_cap_keybinds);
	for (int i=0; i<g_cap_keybinds; i++) {
		g_keybinds[i].input = INPUT_NONE;
		g_keybinds[i].keycodes = NULL;
		g_keybinds[i].num_keycodes = 0;
	}
}

void Binding_InitFromFile(Datablock_File *dbf, Uint16 bind_dbtp) {
	if (dbf == NULL) return;
	Binding_Init();
	for (int dbi=0; dbi<dbf->num_blocks; dbi++) {
		Datablock *db = Datablock_File_GetBlock(dbf, dbi);
		if (db->block_type != bind_dbtp) {
			Datablock_Destroy(db);
			continue;
		}
		if (db->block_length < 8) {
			char buf[256];
			SDL_snprintf(buf, 256, "Ignored invalid keybinding: Block with type 0x%04X is too small! (%iB)", db->block_type, db->block_length);
			Log_Message(LOG_WARNING, buf);
			Datablock_Destroy(db);
			continue;
		}

		Input_Type in = db->data[0];
		in = (in << 8) | db->data[1];
		in = (in << 8) | db->data[2];
		in = (in << 8) | db->data[3];

		for (int i=4; i<db->block_length; i+=4) {
			SDL_KeyCode kc = db->data[i + 0];
			kc = (kc << 8) | db->data[i + 1];
			kc = (kc << 8) | db->data[i + 2];
			kc = (kc << 8) | db->data[i + 3];

			Binding_Add(in, kc);
		}

		Datablock_Destroy(db);
	}
}

void Binding_WriteToFile(Datablock_File *dbf, Uint16 bind_dbtp) {
	if (dbf == NULL) return;
	int start_blocks = dbf->num_blocks;
	Datablock **db_list = SDL_malloc(sizeof(Datablock *) * (start_blocks + g_num_keybinds));
	Datablock_File_GetAllBlocks(dbf, db_list);

	int dbi = dbf->num_blocks; // Append new blocks to the end of the list
	Uint8 data[0x400];
	for (int input_i=0; input_i<g_num_keybinds; input_i++) {
		Binding bind = g_keybinds[input_i];
		int blocklen = 0;

		data[blocklen++] = (bind.input >> 24) & 0xFF;
		data[blocklen++] = (bind.input >> 16) & 0xFF;
		data[blocklen++] = (bind.input >>  8) & 0xFF;
		data[blocklen++] = (bind.input >>  0) & 0xFF;

		for (int i=0; i<bind.num_keycodes; i++) {
			data[blocklen++] = (bind.keycodes[i] >> 24) & 0xFF;
			data[blocklen++] = (bind.keycodes[i] >> 16) & 0xFF;
			data[blocklen++] = (bind.keycodes[i] >>  8) & 0xFF;
			data[blocklen++] = (bind.keycodes[i] >>  0) & 0xFF;
		}
		db_list[dbi++] = Datablock_Create(bind_dbtp, data, blocklen);
	}

	Datablock_File *newfile = Datablock_File_Create(dbf->filename, db_list, dbi);
	if (newfile == NULL) {
		char buf[256];
		SDL_snprintf(buf, 256, "Failed to create %i datablocks for keybinds in settings file '%s'", dbi, dbf->filename);
		Log_Message(LOG_ERROR, buf);
		return;
	}
	Datablock_File_Close(dbf);
	dbf = newfile;

	for (int i=0; i<dbi; i++) {
		Datablock_Destroy(db_list[i]);
	}

	char buf[256];
	SDL_snprintf(buf, 256, "Successfully added %i keybinds to '%s'", dbi - start_blocks, dbf->filename);
	Log_Message(LOG_INFO, buf);
}

void Binding_Term() {
	if (g_keybinds == NULL) return;
	for (int i=0; i<g_cap_keybinds; i++) {
		if (g_keybinds[i].keycodes != NULL) SDL_free(g_keybinds[i].keycodes);
	}
	SDL_free(g_keybinds);
}

void Binding_Add(Input_Type in, SDL_KeyCode kc) {
	if (in == INPUT_NONE) return;

	// Get or create binding
	Binding *bind = NULL;
	for (int i=0; i<g_num_keybinds; i++) {
		if (g_keybinds[i].input == in) {
			bind = &g_keybinds[i];
			break;
		}
	}
	if (bind == NULL) {
		bind = &g_keybinds[g_num_keybinds++];
		bind->input = in;
		bind->keycodes = SDL_malloc(sizeof(SDL_KeyCode));
		bind->num_keycodes = 0;

		// Expand list if needed
		if (g_num_keybinds >= g_cap_keybinds) {
			g_cap_keybinds += 0x100;
			g_keybinds = SDL_realloc(g_keybinds, g_cap_keybinds);
		}
	}

	// Add current keycode
	int newlen = bind->num_keycodes+1;
	bind->keycodes = SDL_realloc(bind->keycodes, sizeof(SDL_KeyCode) * newlen);
	bind->keycodes[bind->num_keycodes] = kc;
	bind->num_keycodes = newlen;
}

Input_Type Binding_ConvKeyCode(SDL_KeyCode kc) {
	for (int i=0; i<g_num_keybinds; i++) {
		Binding bind = g_keybinds[i];
		for (int c=0; c<bind.num_keycodes; c++) {
			if (bind.keycodes[c] == kc) {
				return bind.input;
			}
		}
	}

	return INPUT_NONE;
}