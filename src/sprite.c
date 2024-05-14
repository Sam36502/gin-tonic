#include "../include/sprite.h"

Spritesheet *__load_texture(const char *img_file) {
	SDL_Surface *surf = SDL_LoadBMP(img_file);
	if (surf == NULL) {
		char buf[256];
		SDL_snprintf(buf, 256, "Failed to load grid spritesheet '%s'", img_file);
		Log_Message(LOG_ERROR, buf);
		return NULL;
	}

	Spritesheet *sheet = SDL_malloc(sizeof(Spritesheet));
	sheet->texture = SDL_CreateTextureFromSurface(g_renderer, surf);

	SDL_FreeSurface(surf);
	return sheet;
}

Spritesheet *Sprite_LoadSheet(const char *img_file, const char *prt_file) {
	Log_Message(LOG_WARNING, "Sorry non-grid Spritesheet loading is not implemented yet");
	return NULL;
}

// Loads a spritesheet where all the sprites are the same size
//
Spritesheet *Sprite_LoadSheetGrid(const char *img_file, int sprite_width, int sprite_height) {
	Spritesheet *sheet = __load_texture(img_file);
	if (sheet == NULL) return NULL;

	int full_width, full_height;
	SDL_QueryTexture(sheet->texture, NULL, NULL, &full_width, &full_height);
	sheet->width = full_width/sprite_width;
	sheet->height = full_height/sprite_height;
	int num_sprites = sheet->width * sheet->height;

	sheet->spritelist = SDL_malloc(sizeof(Sprite) * num_sprites);
	size_t filename_len = SDL_strlen(img_file) + 1;
	sheet->filename = SDL_malloc(sizeof(char) * filename_len);
	SDL_memcpy(sheet->filename, img_file, filename_len);
	SpriteID sprite_id = 0;
	for (int y=0; y<sheet->height; y++) {
		for (int x=0; x<sheet->width; x++) {
			Sprite sprite = SDL_malloc(sizeof(Sprite));
			sprite->x = x * sprite_width;
			sprite->y = y * sprite_height;
			sprite->w = sprite_width;
			sprite->h = sprite_height;
			sheet->spritelist[sprite_id] = sprite;
			sprite_id++;
		}
	}
	sheet->sprite_width = sprite_width;
	sheet->sprite_height = sprite_height;

	return sheet;
}

void Sprite_FreeSheet(Spritesheet *sheet) {
	int len = SPRITE_COUNT(sheet);
	for (int i=0; i<len; i++) SDL_free(sheet->spritelist[i]);
	SDL_free(sheet->spritelist);
	SDL_DestroyTexture(sheet->texture);
	SDL_free(sheet);
}

void Sprite_Draw(Spritesheet *sheet, int x, int y, SpriteID id) {
	if (sheet == NULL) {
		char buf[256];
		SDL_snprintf(buf, 256, "Tried to draw sprite 0x%04X from null spritesheet to (%i/%i)", id, x, y);
		Log_Message(LOG_ERROR, buf);
		return;
	}

	if (id < 0 || id >= SPRITE_COUNT(sheet)) return;
	SDL_RenderCopyEx(g_renderer,
		sheet->texture,
		(SDL_Rect *) sheet->spritelist[id],
		&(struct SDL_Rect){
			x, y,
			sheet->sprite_width,
			sheet->sprite_height
		},
		0.0, NULL, SDL_FLIP_NONE
	);
}
