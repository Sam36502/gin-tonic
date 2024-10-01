#ifndef GT_BINDING_H
#define GT_BINDING_H

//
//		Binding
//
//	Handles making inputs more uniform
//	by using bindings instead of just keys.
//

#include <SDL2/SDL.h>
#include "datablock.h"

#define MAX_KEYBINDS 0x400

//	
//		Type Definitions
//	

typedef enum {
	INPUT_NONE = 0x00,
	INPUT_UP,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_RIGHT,
	INPUT_SELECT,
	INPUT_BACK,
} Input_Type;

typedef struct {
	Uint32 input;
	SDL_KeyCode *keycodes;
	Uint32 num_keycodes;
} Binding;


//	
//		Function Declarations
//	

extern Binding *g_keybinds;
extern int g_num_keybinds;
extern int g_cap_keybinds;


//	
//		Function Declarations
//	

//	Initialises the keybind system with no binds set
//	
void Binding_Init();

//	Adds a binding to the current list
//	
void Binding_Add(Input_Type in, SDL_KeyCode kc);

//	Initialises the keybind system from a datablock file
//	
//	Reads all blocks of type `bind_dbtp` from the given file
//	and initialises the bindings from them.
//	Format: (Big-Endian)
//		- [4B](int): Input Type of this binding
//		- [n*4B](int): 1 or more SDL_KeyCodes that map to this binding
void Binding_InitFromFile(Datablock_File *dbf, Uint16 bind_dbtp);

//	Takes the currently bound keys and writes them to a file
//	
//	See `Binding_InitFromFile()` for the file format.
void Binding_WriteToFile(Datablock_File *dbf, Uint16 bind_dbtp);

//	Teriminates the keybind system
//	
void Binding_Term();

//	Takes an SDL_KeyCode from an event and returns the binding
//	
Input_Type Binding_ConvKeyCode(SDL_KeyCode kc);

#endif