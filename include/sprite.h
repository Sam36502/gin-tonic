#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include "log.h"
#include "screen.h"

#define SPRITE_COUNT(s) (s->width * s->height)

#define NULL_SPRITE -1

typedef int SpriteID;

typedef SDL_Rect *Sprite;

typedef struct Spritesheet {
	unsigned int width;
	unsigned int height;
	unsigned int sprite_width;
	unsigned int sprite_height;
	Sprite *spritelist;
	char *filename;
	SDL_Texture *texture;
} Spritesheet;

//
//	Function Declarations
//

//	Loads a spritesheet from a BMP with arbitrary sprite sizes
//	
//	This function requires an additional "partition" file (custom format)
//	that describes the sizes and positions of each sprite in the image.
//
//	WARNING: This function is not yet implemented
Spritesheet *Sprite_LoadSheet(const char *img_file, const char *prt_file);

//	Loads a spritesheet from a BMP where all the sprites are equally sized
//	
//	This function loads a BMP a divides it equally into sections `sprite_width` by
//	`sprite_height` and numbers the resulting sprites beginning at 0 in the top left
//	corner and incrementing to the right. e.g.:
//	
//		00 01 02 03
//		04 05 06 07
//	
//	Returns `NULL` if an error occurs while loading.
//	
//	You must also call `Sprite_FreeSheet()` when you are finished with it to avoid a memory leak
Spritesheet *Sprite_LoadSheetGrid(const char *img_file, int sprite_width, int sprite_height);

//	Frees a previously created spritesheet
//	
void Sprite_FreeSheet(Spritesheet *sheet);

//	Draws a sprite to the screen
//	
//	The sprite is drawn with its top left corner at the given
//	x and y coordinates.
void Sprite_Draw(Spritesheet *sheet, int x, int y, SpriteID id);


#endif