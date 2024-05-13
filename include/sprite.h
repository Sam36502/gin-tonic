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
	const char *filename;
	SDL_Texture *texture;
} Spritesheet;

void Sprite_Init();

Spritesheet *Sprite_LoadSheet(const char *img_file, const char *prt_file); // Not Implemented!
Spritesheet *Sprite_LoadSheetGrid(const char *img_file, int sprite_width, int sprite_height);
void Sprite_FreeSheet(Spritesheet *sheet);
void Sprite_Draw(Spritesheet *sheet, int x, int y, SpriteID id);


#endif