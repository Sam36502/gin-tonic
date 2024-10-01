#include "../include/datablock.h"



Datablock *Datablock_Create(Uint16 type, void *data, size_t data_len) {
	if (data == NULL) {
		Log_Message(LOG_ERROR, "Tried to create data-block from NULL data");
		return NULL;
	}

	if (data_len > UINT16_MAX) {
		data_len = UINT16_MAX;
		Log_Message(LOG_WARNING, "Tried to create a data-block larger than `UINT16_MAX`. Data will be truncated!");
	}

	Datablock *db = SDL_malloc(sizeof(Datablock));
	db->block_type = type;
	db->block_length = data_len;
	db->data = SDL_malloc(sizeof(Uint8) * data_len);
	SDL_memcpy(db->data, data, data_len);

	Datablock_CalcSum(db);
	return db;
}

void Datablock_Destroy(Datablock *db) {
	SDL_free(db->data);
	SDL_free(db);
}

void Datablock_CalcSum(Datablock *db) {
	db->checksum = Util_FletcherChecksum(db->data, db->block_length);
}

bool Datablock_IsValid(Datablock *db) {
	Uint32 comp_sum = Util_FletcherChecksum(db->data, db->block_length);
	return db->checksum == comp_sum;
}

void Datablock_Print(Datablock *db) {
	if (db == NULL) {
		printf("  Datablock is NULL!");
		return;
	}

	printf("      Type: 0x%04X\n", db->block_type);
	printf("    Length: %i Bytes\n", db->block_length);
	printf("  Checksum: 0x%08X (%s)\n", db->checksum, Datablock_IsValid(db) ? "VALID":"BAD!");
	printf("      Data:");
	for (int i=0; i<db->block_length; i++) {
		if (i % 16 == 0) printf("\n  0x");
		printf("  %02X", db->data[i]);
		if (db->block_length > 8*16 && i > 4*16) {
			i = db->block_length - 4*16;
			printf("            ...\n");
		}
	}
}


Datablock_File *Datablock_File_Open(char *filename) {
	Datablock_File *dbf = SDL_malloc(sizeof(Datablock_File));
	
	dbf->filename = Util_FS_GetValidPath(filename);

	// Check Magic Bytes
	FILE *f = Util_FS_Open(filename, "rb");
	char errmsg[256];
	if (f == NULL) {
		SDL_snprintf(errmsg, 256, "Failed to open Data-Block File '%s': Couldn't access file", filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}

	char magic_bytes[5];
	if (fgets(magic_bytes, 5, f) == NULL) {
		SDL_snprintf(errmsg, 256, "Failed to open Data-Block File '%s': Couldn't read file", filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}
	if (SDL_strcmp(DATABLOCK_FILE_MAGIC, magic_bytes) != 0) {
		SDL_snprintf(errmsg, 256, "Failed to open Data-Block File '%s': Invalid Header", filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}

	// Skim file and build index
	int tmp_index_size = 256;
	Datablock_IndexEntry *tmp_index = SDL_malloc(sizeof(long) * tmp_index_size);
	dbf->num_blocks = 0;

	while (!feof(f)) {
		long curr_offset = ftell(f);
		Uint8 block_head[4];
		int read = fread(block_head, sizeof(Uint8), 4, f);
		if (read != 4) {
			if (feof(f)) break;

			SDL_snprintf(errmsg, 256, "Failed to open Data-Block File '%s': Ended in the middle of a block header", filename);
			Log_Message(LOG_ERROR, errmsg);
			return NULL;
		}
		Uint16 block_type, block_size;
		U8_TO_U16(block_type, block_head[0], block_head[1]);
		U8_TO_U16(block_size, block_head[2], block_head[3]);

		Datablock_IndexEntry entry = {
			.block_type = block_type,
			.file_offset = curr_offset,
		};
		tmp_index[dbf->num_blocks++] = entry;

		if (dbf->num_blocks >= tmp_index_size) {
			tmp_index_size += 256;
			tmp_index = SDL_realloc(tmp_index, tmp_index_size);
		}

		// Seek to next block
		fseek(f, block_size	+ sizeof(Uint32), SEEK_CUR);
	}

	dbf->index = SDL_malloc(sizeof(Datablock_IndexEntry) * dbf->num_blocks);
	SDL_memcpy(dbf->index, tmp_index, sizeof(Datablock_IndexEntry) * dbf->num_blocks);

	return dbf;
}

void Datablock_File_Close(Datablock_File *dbf) {
	if (dbf == NULL) return;
	SDL_free(dbf->index);
	SDL_free(dbf->filename);
	SDL_free(dbf);
}

Datablock *Datablock_ParseFromStream(FILE *f) {
	Datablock *db = SDL_malloc(sizeof(Datablock));

	// Read Header
	Uint8 block_head[4];
	int read = fread(block_head, sizeof(Uint8), 4, f);
	if (read != 4) return NULL;
	U8_TO_U16(db->block_type, block_head[0], block_head[1]);
	U8_TO_U16(db->block_length, block_head[2], block_head[3]);

	// Read data
	db->data = SDL_malloc(sizeof(Uint8) * db->block_length);
	read = fread(db->data, sizeof(Uint8), db->block_length, f);
	if (read != db->block_length) return NULL;

	// Read Checksum
	Uint8 chk_bytes[4];
	read = fread(chk_bytes, sizeof(Uint8), 4, f);
	if (read != 4) return NULL;
	db->checksum = (chk_bytes[0] << 24) | (chk_bytes[1] << 16) | (chk_bytes[2] << 8) | chk_bytes[3];

	return db;
}

bool Datablock_WriteToStream(FILE *f, Datablock *db) {
	
	// Write Head
	Uint8 block_head[4];
	U16_TO_U8(db->block_type, block_head[0], block_head[1]);
	U16_TO_U8(db->block_length, block_head[2], block_head[3]);
	int written = fwrite(block_head, sizeof(Uint8), 4, f);
	if (written != 4) return false;

	// Write Data
	written = fwrite(db->data, sizeof(Uint8), db->block_length, f);
	if (written != db->block_length) return false;

	// Write Checksum
	Uint8 chk_bytes[4] = {
		(db->checksum >> 24) & 0xFF,
		(db->checksum >> 16) & 0xFF,
		(db->checksum >>  8) & 0xFF,
		(db->checksum >>  0) & 0xFF,
	};
	written = fwrite(chk_bytes, sizeof(Uint8), 4, f);
	if (written != 4) return false;

	return true;
}


Uint16 Datablock_File_GetAllBlocks(Datablock_File *dbf, Datablock **block_array) {
	for (int i=0; i<dbf->num_blocks; i++) {
		Datablock *db = Datablock_File_GetBlock(dbf, i);
		block_array[i] = db;
	}
	return dbf->num_blocks;
}

Datablock *Datablock_File_GetBlock(Datablock_File *dbf, Uint16 block_index) {
	if (dbf == NULL || block_index >= dbf->num_blocks) return NULL;

	FILE *f = Util_FS_Open(dbf->filename, "rb");
	char errmsg[256];
	if (f == NULL) {
		SDL_snprintf(errmsg, 256, "Failed to open Data-Block File '%s'", dbf->filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}
	fseek(f, dbf->index[block_index].file_offset, SEEK_SET);
	Datablock *db = Datablock_ParseFromStream(f);
	if (db == NULL) {
		SDL_snprintf(errmsg, 256, "Failed to retrieve data from datablock file '%s' at index %i", dbf->filename, block_index);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}
	fclose(f);

	return db;
}

Datablock *Datablock_File_FindFirstOfType(Datablock_File *dbf, Uint16 block_type) {

	// Find index
	Uint16 index = 0;
	bool found = false;
	for (index=0; index<dbf->num_blocks; index++) {
		if (dbf->index[index].block_type == block_type) {
			found = true;
			break;
		}
	}
	if (!found) {
		char errmsg[256];
		SDL_snprintf(errmsg, 256, "Block with type 0x%04X not found in data-block file '%s'", block_type, dbf->filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}

	return Datablock_File_GetBlock(dbf, index);
}

Datablock_File *Datablock_File_Create(char *filename, Datablock **blocks, Uint16 num_blocks) {
	Datablock_File *dbf = SDL_malloc(sizeof(Datablock_File));
	
	dbf->filename = Util_FS_GetValidPath(filename);
	dbf->index = SDL_malloc(sizeof(Datablock_IndexEntry) * num_blocks);
	dbf->num_blocks = num_blocks;

	// Write Magic Bytes
	FILE *f = Util_FS_Open(filename, "wb");
	char errmsg[256];
	if (f == NULL) {
		SDL_snprintf(errmsg, 256, "Failed to open Data-Block File for writing '%s': Couldn't access file", filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}
	if (fputs(DATABLOCK_FILE_MAGIC, f) < 0) {
		SDL_snprintf(errmsg, 256, "Failed to write Data-Block File '%s': failed to write magic bytes", filename);
		Log_Message(LOG_ERROR, errmsg);
		return NULL;
	}

	// Write Blocks to file
	for (int i=0; i<num_blocks; i++) {
		Datablock *db = blocks[i];
		dbf->index[i] = (Datablock_IndexEntry){
			.block_type = db->block_type,
			.file_offset = ftell(f),
		};
		Datablock_WriteToStream(f, db);
	}
	fclose(f);

	return dbf;
}