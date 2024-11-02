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
//		The Block format is a simple header that includes
//		the length of the included data and a type/id nr.
//		All multi-byte fields are big-endian
//		The data is sandwiched between two 4-byte sections;
//		at the top, is a header consisting of the 16-bit
//		block type/id, and a 16-bit length indicator, at
//		the end of the block is a 32-bit checksum of the
//		data. This is an example of a valid datablock:
//			
//			0x	12 34	--> Block type is 0x1234
//			0x	00 08	--> Block contains 8 bytes
//			0x	54 65 73 74 69 6E 67 00	--> Data (ASCII for "Testing" with null terminator)
//			0x	E5 CA ED 9C	--> Checksum
//			
//		A block must be at minimum 8 bytes long, even if
//		it contains no data. This is a valid block:
//			
//			0x	12 34 00 00 FF FF 55 55
//			

//		To-Do List / Future Features:
//		 - Create Arena/Stack allocator for datablocks in a file (could work like a cache as well?)

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
#define DATABLOCK_ITYPE_FILE_HEADER 0xFFFF	// Denotes a block that acts as a datablock-file header


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
//	`data` may also be NULL, in which case an empty block
//	of `data_len` 0x00 bytes will be created.
//	
//	`type` may be any 15-bit number with the MSB cleared.
//	block-types with bit 15 set are reserved for internal use.
//	i.e.:	0x0000 - 0x7FFF: Available
//			0x8000 - 0xFFFF: Reserved; Shouldn't be used
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

//	Retrieves a whole data block from a datablock file by its ID (type)
//	
//	Searches the index for the first block that matches the given type
//	and returns a pointer to that block
Datablock *Datablock_File_FindFirstOfType(Datablock_File *dbf, Uint16 block_type);

//	Retrieves a whole data block from a datablock file by its type and number
//	
//	Searches the index for the `num`-th block that matches the given type
//	and returns a pointer to that block. E.g.:
//	
//		Your file has blocks A (0x0001), B (0x0002), C (0x0001);
//		You call this function with `block_type` = 0x0001 and `num` = 1;
//		It returns a copy of block C
Datablock *Datablock_File_FindNthOfType(Datablock_File *dbf, Uint16 block_type, int num);

//	Appends a new datablock to the end of a file
//	
//	Returns the index of the new block or -1 if it failed
int Datablock_File_AppendBlock(Datablock_File *dbf, Datablock *db);

//	Updates the data of a block in a file
//	
//	Looks for the datablock in the file at the given `block_index` and
//	replaces a portion of the data bytes with the given data.
//	
//	It cannot create any new blocks or extend existing ones,
//	it only overwrites existing blocks with the same amount of data!
//	If `size` is smaller than the datablock,
//	the remaining bytes will be unaffected in the file
//	Additionally, you can set which part of the block to overwrite with `offset`.
//	Also, if `data` is NULL, the selected space will be overwritten with zeroes.
//	
//	Example:
//		Before:	0x00 0x01 0x02 0x03
//			
//		Then update with data={ 0xFF }, len=1, offset=0:
//				0xFF 0x01 0x02 0x03
//			
//		Then update with data={ 0xAB, 0xCD }, len=2, offset=1:
//				0xFF 0xAB 0xCD 0x03
//	
//	Returns 0 on successful write,
//	Returns -1 if the block couldn't be found
//	Returns -2 if `size` is larger than the datablock
//	Returns -3 if the file can't be overwritten (or is NULL)
int Datablock_File_UpdateBlock(Datablock_File *dbf, Uint16 block_index, void *data, size_t size, size_t offset);

//	Stores an array of Datablocks to a file
//	
//	Creates a new file pointer to the created file
Datablock_File *Datablock_File_Create(char *filename, Datablock **blocks, Uint16 num_blocks);

#endif
