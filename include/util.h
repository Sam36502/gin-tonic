#ifndef UTIL_H
#define UTIL_H
//	
//				Generic / Basic Utilities
//	
//		Handles very basic general Utilities and is also
//		Where to declare truly universal variables
//	
//		(Hierarchy-Level: Bottom)
//	

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_filesystem.h>


//	
//		Constants & Macro Definitions
//	

#define U8_TO_U16(v,h,l) v = (h << 8) | l;
#define U16_TO_U8(v,h,l) h = (v) >> 8; l = (v) & 0xFF;
#define PRINT_U8_PTN "%i%i%i%i%i%i%i%i"
#define PRINT_U8(n) (n>>7)&1,(n>>6)&1,(n>>5)&1,(n>>4)&1,(n>>3)&1,(n>>2)&1,(n>>1)&1,n&1


//	
//		Universal Variable Declarations
//	

//	Universal variable that determines whether the game is running
//	
//	can be freely set externally
extern bool g_isRunning;
extern char *g_root_dir;


//	
//		Function Declarations
//	

//	Calculates the Fletcher checksum on a given block of data
//	
Uint32 Util_FletcherChecksum(Uint8 *data, size_t len);

//	Takes a full path string and separates out the basename
//	
//	Returns a pointer to a copy of the string which should be
//	freed when no longer in use
char *Util_FS_Basename(char *fullpath);

//	Takes a full path string and separates out the directory name
//	
//	This includes the trailing slash!
//	Returns a pointer to a copy of the string which should be
//	freed when no longer in use
char *Util_FS_Dirname(char *fullpath);

//	Looks for a file and returns valid path if possible
//	
//	Will check the same directories as `Util_FS_Open()`,
//	and return the path the works, if one does.
//	Mainly only necessary when calling file functions other than `fopen()`
//	The returned string must be freed with `SDL_free()` by the caller.
char *Util_FS_GetValidPath(char *path);

//	Finds a file either in the execution dir or the bin dir
//	
//	First checks if the filename alone exists, then checks
//	if the file exists in the bin directory.
FILE *Util_FS_Open(char *filename, char *mode);

#endif