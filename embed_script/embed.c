#include <SDL2/SDL.h>
#include "../include/screen.h"

// Script to embed images into the final DLL
// Primarily used for the text spritemap,
// so it doesn't need to be included seperately

// The text spritemap image only needs to be greyscale (alpha included),
// so this script takes an image and turns it into a binary file with
// a 32-bit width, 32-bit height (little endian), followed by 8-bit
// indices into a greyscale palette:
//     0x00 => RGBA 0x00, 0x00, 0x00, 0xFF
//     0x01 => RGBA 0x01, 0x01, 0x01, 0xFF
//      ...
//     0xFF => RGBA 0xFF, 0xFF, 0xFF, 0xFF

#define TXT_SPSH_FILE "../assets/text_sprites.bmp"

// Compile with:
// gcc -o embed.exe embed.c ../src/screen.c ../src/log.c -lmingw32 -lSDL2main -lSDL2

int main(int argc, char *argv[]) {
	if (argc < 3) return 1;

	char *infile = argv[1];
	char *outfile = argv[2];

	SDL_Init(SDL_INIT_VIDEO);
	Screen_Init("", 1920, 1080);

	SDL_Surface *surf = SDL_LoadBMP(infile);
	if (surf == NULL) {
		printf("Failed to load bmp file '%s'\n", infile);
		return 1;
	}

	// Convert to 1 Byte-per-pixel greyscale
	SDL_Surface *conv = SDL_CreateRGBSurfaceWithFormat(0, surf->w, surf->h, 8, SDL_PIXELFORMAT_INDEX8);
	if (conv == NULL) Log_SDLMessage(LOG_FATAL, "Failed to create conversion surface");

	// Generate palette
	SDL_Colour clrs[0x100];
	for (int i=0; i<0x100; i++) clrs[i] = (SDL_Colour){ i, i, i, 0xFF };
	SDL_SetPaletteColors(conv->format->palette, clrs, 0, 0x100);

	// Convert surface
	int err = SDL_BlitSurface(surf, NULL, conv, NULL);
	if (err != 0) Log_SDLMessage(LOG_FATAL, "Failed to convert surface");
	SDL_FreeSurface(surf);

	// Store the surface's pixel data to a file
	FILE *f = fopen(outfile, "wb");
	fwrite(&conv->w, sizeof(int), 1, f);
	fwrite(&conv->h, sizeof(int), 1, f);
	fwrite(conv->pixels, sizeof(Uint8), conv->pitch * conv->h, f);
	fclose(f);

	SDL_FreeSurface(conv);

	Screen_Term();

	return 0;
}