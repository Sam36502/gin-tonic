#ifndef GT_DATABLOCK_H
#define GT_DATABLOCK_H
//	
//				Data-Block Utilities
//	
//		This file defines a generic data format that's
//		quite easy to parse and can be used to read/write
//		data to/from files or over a network interface in a
//		custom format without as much hassle of parsing.
//	

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "log.h"
#include "util.h"
#include "screen.h"

//	
//		Constant Definitions
//	

#define DATABLOCK_FILE_MAGIC "DBF!"
#define DATABLOCK_NULL_TYPE 0xFFFF

// Internal Data-Block Types
#define DATABLOCK_ITYPE_FILE_HEADER 0x8000	// Denotes a block that acts as a datablock-file header


//	
//		Type Definitions
//	

typedef struct {
	Uint16 block_type;
	Uint16 block_length;	// Length of `data`; doesn't include header & checksum
	Uint8 *data;
	Uint32 checksum;		// Fletcher's Algorithm
} Datablock;

typedef struct {
	Uint16 block_type;
	long file_offset;
} Datablock_IndexEntry;

typedef struct {
	char *filename;
	Uint16 num_blocks;
	Datablock_IndexEntry *index;
} Datablock_File;


//	
//		Function Declarations
//	

//	Create a data-block from some arbitrary data
//	
//	This will produce a pointer to the data block and
//	will also calculate the checksum. If you directly
//	alter the block's data after this point, you must
//	call `Datablock_CalcSum()` to recalculate the checksum,
//	otherwise the data will by considered corrupted.
//	
//	`data` may be up to a maximum of `UINT16_MAX` bytes,
//	if `data_len` is greater than `UINT16_MAX`, the function
//	will log a warning and truncate the data.
//	
//	`type` may be any 15-bit number with the MSB cleared.
//	block-types with bit 15 set are reserved for internal use.
Datablock *Datablock_Create(Uint16 type, void *data, size_t data_len);

//	Destroys a datablock
//	
//	Should be called once you are no longer using `db`
void Datablock_Destroy(Datablock *db);

//	Recalculates the checksum of a data block
//	
//	Must be called if you manually alter the data in `db`
void Datablock_CalcSum(Datablock *db);

//	Validates the data in a block against its checksum
//	
//	Returns `true` if `db` has a valid checksum
bool Datablock_IsValid(Datablock *db);

//	Mainly for debugging purposes
//	
//	Takes a Datablock and prints out its data
void Datablock_Print(Datablock *db);


//	Opens a data-block file
//	
//	This will scan the whole file and create an index to make
//	fetching blocks again easier.
//	
//	If the file is not a valid data-block file, it will return
//	NULL and log an error message describing the issue.
Datablock_File *Datablock_File_Open(char *filename);

//	Closes a data-block file
//	
//	Must be called when you don't intend to read/write any more
//	data from/to the file.
void Datablock_File_Close(Datablock_File *dbf);

//	Parses a data-block from a `FILE *`
//	
//	Mainly for internal use.
//	This function assumes `f` is a pointer to a datablock file
//	that has been `fseek()`-ed to beginning of a data block.
Datablock *Datablock_ParseFromStream(FILE *f);

//	Writes a datablock to a `FILE *`
//	
//	Mainly for internal use.
//	This function assumes `f` is a pointer to a datablock file
//	that has been `fseek()`-ed to where a datablock should be written
//	Returns true if successful.
bool Datablock_WriteToStream(FILE *f, Datablock *db);


//	Retrieves all the data blocks in a file
//	
//	Mainly intended to retrieve the blocks for editing in memory,
//	so they can be reordered/edited and saved back to a file
//	`block_array` must be allocated before passed in.
//	
//	Returns the number of blocks returned
Uint16 Datablock_File_GetAllBlocks(Datablock_File *dbf, Datablock **block_array);

//	Retrieves a whole data block from a datablock file
//	
//	This function will allocate the Datablock pointer for you
//	so it should be `Datablock_Destroy()`-ed when you're finished with it.
Datablock *Datablock_File_GetBlock(Datablock_File *dbf, Uint16 block_index);

//	Retrieves a whole data block from a datablock file by its type
//	
//	Searches the index for the first block that matches the given type
//	and returns a pointer to that block
Datablock *Datablock_File_FindFirstOfType(Datablock_File *dbf, Uint16 block_type);

//	Stores an array of Datablocks to a file
//	
//	Creates a new file pointer to the created file
Datablock_File *Datablock_File_Create(char *filename, Datablock **blocks, Uint16 num_blocks);

#endif