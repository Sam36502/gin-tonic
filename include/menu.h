#ifndef GT_MENU_C
#define GT_MENU_C
//		
//		Menu System
//		
//		Lets you create Menus you can select options in
//		

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "util.h"
#include "screen.h"
#include "events.h"
#include "text.h"
#include "binding.h"

#define MENU_MAX_OPTIONS 0x40
#define MENU_CURSOR_CHAR 0x63
#define MENU_TITLE_PAD 1
#define MENU_OPT_PAD MENU_TITLE_PAD+2

//	
//	Type Definitions
//	

typedef struct {
	char *text;
	size_t len;
	void (* cb_selected)();
} Menu_Option;

typedef struct {
	int width;
	int height;
	char *title;
	size_t title_len;
	Menu_Option options[MENU_MAX_OPTIONS];
	int option_count;
	int selected;
} Menu;


//	
//	Function Declarations
//	

//	Create a new menu
//	
//	`title` may be `NULL` and will then not be displayed
//	Returns NULL on error
Menu *Menu_Create(char *title);

//	Destroys a menu and its options (if any)
//	
void Menu_Destroy(Menu *menu);

//	Adds an option to an existing menu
//	
//	`title` may be `NULL` and will then not be displayed
void Menu_AddOption(Menu *menu, char *text, void (* cb_selected)());

//	Draws a Menu to the screen
//	
void Menu_Draw(Menu *menu, int x, int y);

//	Handles the events for the menu
//	
void Menu_HandleEvent(Menu *menu, SDL_Event evt);

//	Handles the inputs for the menu using Gin-Tonic bindings
//	
//	Takes an event and converts it to a keybinding
void Menu_HandleInput(Menu *menu, SDL_Event evt);

#endif