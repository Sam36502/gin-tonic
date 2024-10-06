#include <stdio.h>
#include <SDL2/SDL.h>
#include "../include/screen.h"

// Script to embed images into the final DLL
// Primarily used for the text spritemap,
// so it doesn't need to be included seperately

// The text spritemap image only needs to be greyscale (alpha included),
// so this script takes an image and turns it into a binary file with
// a 32-bit width, 32-bit height (little endian), followed by 8-bit
// indices into a greyscale palette:
//     0x00 => RGBA 0x00, 0x00, 0x00, 0x00
//     0x01 => RGBA 0x01, 0x01, 0x01, 0x01
//      ...
//     0xFF => RGBA 0xFF, 0xFF, 0xFF, 0xFF

//  NOTE:
//  The conversion is done manually, and only keeps the alpha value from the original image!

#define TXT_SPSH_FILE "../assets/text_sprites.bmp"

// Compile with:
// gcc -o embed.exe embed.c ../src/screen.c ../src/log.c -lmingw32 -lSDL2main -lSDL2

int main(int argc, char *argv[]) {
	if (argc < 3) return 1;

	char *infile = argv[1];
	char *outfile = argv[2];

	SDL_Init(SDL_INIT_VIDEO);
	Screen_Init("", 256, 256);

	SDL_Surface *surf = SDL_LoadBMP(infile);
	if (surf == NULL) {
		printf("Failed to load bmp file '%s'\n", infile);
		return 1;
	}

	// Convert to RGBA32 Surface
	SDL_Surface *conv = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
	if (conv == NULL) Log_SDLMessage(LOG_FATAL, "Failed to create conversion surface");
	SDL_FreeSurface(surf);
	surf = conv;

	// Store the surface's pixel data to a file
	FILE *f = fopen(outfile, "wb");
	fwrite(&surf->w, sizeof(int), 1, f);
	fwrite(&surf->h, sizeof(int), 1, f);

	Uint32 *surf_data = (Uint32 *) surf->pixels;
	for (int i=0; i<surf->w * surf->h; i++) {
		Uint8 pixel = (surf_data[i] & surf->format->Amask) >> surf->format->Ashift;
		fwrite(&pixel, sizeof(Uint8), 1, f);
	}
	fclose(f);
	SDL_FreeSurface(surf);

	Screen_Term();
	SDL_Quit();

	return 0;
}