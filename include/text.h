#ifndef GT_TEXT_H
#define GT_TEXT_H
//	
//				Text Utilities
//	
//		Handles drawing text using the sprite system
//		Also includes drawing text-boxes (with wrapping handled)
//		And creating a queue of text boxes to be displayed and clicked through
//	


#include <stdbool.h>
#include "sprite.h"


//	
//		Constant Definitions
//	

#define TEXT_SPRITE_WIDTH 5
#define TEXT_SPRITE_HEIGHT 10
#define TXTBOX_PAD_X 2 // px
#define TXTBOX_PAD_Y 1 // px
//#define TXTBOX_QUEUE_LEN 0x40


//	
//		Type Definitions
//	

typedef struct {
	char *string;
	int cols;
	int rows;
} Textbox;


//	
//		Embedded Data & Global Variable Declarations
//	

// Embedded Bitmap (Not used)
//extern const Uint8 _binary_embeds_text_sprites_start[];
//extern const Uint8 _binary_embeds_text_sprites_end[];


//	Initialise Text Utilities
//	
//	Initialises the text spritesheet from embedded image
void Text_Init(const char *text_bitmap_file);

//	Terminate Text Utilities
//	
//	Cleans up text spritesheet
void Text_Term();

//	Draw text to the screen
//	
//	Requires `Text_Init()` to have been called beforehand.
//	Writes a string to the screen at a given position.
//	Doesn't handle non-printable characters (e.g. '\n').
void Text_Draw(char *str, size_t str_len, int x, int y);

//	Creates a text box
//	
//	Takes the given string and fits it into a textbox
//	of the given size. It will also automatically handle
//	Newlines and cut off the text (internally, not just on draw)
//	at the border.
Textbox *TextBox_Create(char *str, size_t str_len, int cols, int rows);

//	Renders a text box to the screen
//	
//	Takes a text box object created with `TextBox_Create()` and
//	draws it at the given position on the screen.
void TextBox_DrawTextbox(Textbox *txtbox, int x, int y);

void TextBox_AddToQueue(Textbox txtbox);
bool TextBox_IsQueueFull();
bool TextBox_IsQueueEmpty();

#endif