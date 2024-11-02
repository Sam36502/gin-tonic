#ifndef GT_BINDING_H
#define GT_BINDING_H

//
//		Binding
//
//	Handles making inputs more uniform
//	by using bindings instead of just keys.
//
//	TODO:
//		- Fix Write to File

#include <SDL2/SDL.h>
#include "datablock.h"

#define MAX_KEYBINDS 0x400

//	
//		Type Definitions
//	

typedef Uint32 Input_Type;
#define INPUT_NONE		(Input_Type)(0x00)
#define INPUT_UP		(Input_Type)(0x01)
#define INPUT_DOWN		(Input_Type)(0x02)
#define INPUT_LEFT		(Input_Type)(0x03)
#define INPUT_RIGHT		(Input_Type)(0x04)
#define INPUT_SELECT	(Input_Type)(0x05)
#define INPUT_BACK		(Input_Type)(0x06)

typedef struct {
	Input_Type input;
	SDL_KeyCode *keycodes;
	Uint32 num_keycodes;
	bool is_held;
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
//	Does nothing if the system is already initialised
void Binding_Init();

//	Terminates the keybind system
//	
void Binding_Term();

//	Initialises the keybind system from a datablock file
//	
//	Reads all blocks of type `bind_dbtp` from the given file
//	and initialises the bindings from them.
//	Format: (Big-Endian)
//		- [4B](int): Input Type of this binding
//		- [n*4B](int): 1 or more SDL_KeyCodes that map to this binding
//	
//	Does nothing if the system is already initialised
void Binding_InitFromFile(Datablock_File *dbf, Uint16 bind_dbtp);

//	Takes the currently bound keys and writes them to a file
//	
//	NOTE! Creates a new datablock file and closes it!
//	Any other references to `dbf` will be invalidated!
//	TODO: Rename to 'create_file` or something and make new version that just appends without closing
//	
//	See `Binding_InitFromFile()` for the file format.
void Binding_WriteToFile(Datablock_File *dbf, Uint16 bind_dbtp);

//	Adds a binding to the current list
//	
//	Will not bind anything to INPUT_NONE
void Binding_Add(Input_Type in, SDL_KeyCode kc);

//	Finds the binding that corresponds to a given keycode
//	
//	Returns the index of the binding or -1 otherwise
int Binding_GetByKeyCode(SDL_KeyCode kc);

//	Takes an SDL_KeyCode from an event and returns the binding
//	
//	Always returns `INPUT_NONE` if the system hasn't been initialised yet.
Input_Type Binding_ConvKeyCode(SDL_KeyCode kc);

//	Checks an SDL_Event and updates binding states accordingly
//	
void Binding_HandleEvent(SDL_Event event);

//	Returns whether a certain input is being held or not
//	
//	Always returns `false` if...
//	 - The Binding system hasn't been initialised yet;
//	 - The input type `in` is INPUT_NONE; or
//	 - input type `in` isn't bound to anything
bool Binding_IsHeld(Input_Type in);


#endif
