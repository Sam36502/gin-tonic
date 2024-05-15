#include <SDL2/SDL.h>
#include "../include/screen.h"

// Idea I dabbled with, perhaps retry at another time:
// Embed the text bitmap into the GinTonic dll

// Compile with:
// gcc -o embed.exe embed.c ../src/screen.c ../src/log.c -lmingw32 -lSDL2main -lSDL2

int main(int argc, char *argv[]) {
	if (argc < 3) return 1;

	char *infile = argv[1];
	char *outfile = argv[2];

	SDL_Init(SDL_INIT_VIDEO);
	Screen_Init("nah", 1920, 1080);

	SDL_Surface *surf = SDL_LoadBMP(infile);
	if (surf == NULL) {
		printf("Failed to load bmp file '%s'\n", infile);
		return 1;
	}

	// Store the surface's pixel data to a file
	FILE *f = fopen(outfile, "wb");
	fwrite(&surf->w, sizeof(int), 1, f);
	fwrite(&surf->h, sizeof(int), 1, f);
	fwrite(&surf->format->BitsPerPixel, sizeof(int), 1, f);
	fwrite(&surf->pitch, sizeof(int), 1, f);

	fwrite(&surf->format->Rmask, sizeof(Uint32), 1, f);
	fwrite(&surf->format->Gmask, sizeof(Uint32), 1, f);
	fwrite(&surf->format->Bmask, sizeof(Uint32), 1, f);
	fwrite(&surf->format->Amask, sizeof(Uint32), 1, f);

	fwrite(surf->pixels, sizeof(Uint8), surf->pitch * surf->h * sizeof(Uint32), f);
	fclose(f);

	SDL_FreeSurface(surf);

	Screen_Term();

	return 0;
}