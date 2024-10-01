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
#define TEXT_BASE_COLOUR (SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF }
#define TEXT_KERN 1 // px; distance between chars (horizontally & vertically)
#define TXTBOX_PAD_X 2 // px
#define TXTBOX_PAD_Y 1 // px
#define TXTBOX_CLR_BG (SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
#define TXTBOX_CLR_BORDER (SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF }

#define TEXT_ESC_CHAR '\x1B'
#define TEXT_ESC_RESET	"\x1B\x00"
#define TEXT_ESC_CLR	"\x1B\x01"

#define TXTBOX_WIDTH(b) (TXTBOX_PAD_X + b->cols*(1+TEXT_SPRITE_WIDTH) + TXTBOX_PAD_X)
#define TXTBOX_HEIGHT(b) (TXTBOX_PAD_Y + b->rows*(1+TEXT_SPRITE_HEIGHT) + TXTBOX_PAD_Y)

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

extern Spritesheet *g_text_spsh;


//	Initialise Text Utilities
//	
//	Initialises the text spritesheet from embedded image
void Text_Init(char *text_bitmap_file);

//	Terminate Text Utilities
//	
//	Cleans up text spritesheet
void Text_Term();

//	Sets the colour of text
//	
//	Can be called internally with Escape Sequences
void Text_SetColour(SDL_Colour clr);

//	Sets the colour back to white
//	
void Text_ResetColour();

//	Handles escape codes in text strings
//	
//	Updates index to be the first char after the escape sequence
//	
//	Escape sequences:
//		\x1B \x00: Reset formatting
//		\x1B \x01 \xRR \xGG \xBB: Set text colour to provided RGB
void Text_HandleEscape(char *str, int *index);

//	Draw text to the screen
//	
//	Requires `Text_Init()` to have been called beforehand.
//	Writes a string to the screen at a given position.
//	Doesn't handle non-printable characters (e.g. '\n').
void Text_Draw(char *str, size_t str_len, int x, int y);

//	Prints all chars in the Gin-Tonic custom encoding
//	
//	Prints a grid of all characters in the Gin-Tonic custom encoding with
//	black backgrounds and spaced out with hex indicators
void Text_PrintEncoding(int x, int y);

//	Creates a text box
//	
//	Takes the given string and fits it into a textbox
//	of the given size. It will also automatically handle
//	Newlines and cut off the text (internally, not just on draw)
//	at the border.
Textbox *Text_Box_Create(char *str, size_t str_len, int cols, int rows);

//	Destroys a text box created with `Text_Box_Create()`
//	
void Text_Box_Destroy(Textbox *txtbox);

//	Sets the text of a text box
//	
//	Trims the string internally to fit the set size
void Text_Box_SetText(Textbox *txtbox, char *str);

//	Sets the size of a text box
//	
//	If one of the sizes is negative, that value won't
//	be updated. (e.g. w = -1 --> keeps old width)
void Text_Box_SetSize(Textbox *txtbox, int w, int h);

//	Renders a text box to the screen
//	
//	Takes a text box object created with `TextBox_Create()` and
//	draws it at the given position on the screen.
void Text_Box_Draw(Textbox *txtbox, int x, int y);

// void Text_Box_AddToQueue(Textbox txtbox);
// bool Text_Box_IsQueueFull();
// bool Text_Box_IsQueueEmpty();

#endif