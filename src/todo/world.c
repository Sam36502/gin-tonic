#include "world.h"
#include "perlin.h"

int g_camera_x = 0;
int g_camera_y = 0;


// TODO: Load world gen settings from a world/save file
//       Allows for future different worlds (dimensions? lol)
float g_world_thresholds[WORLD_LAYER_COUNT] = {
	0.0, 0.32, 0.4, 0.44, 0.65, 0.77,
};

SDL_Colour g_world_colours[WORLD_LAYER_COUNT] = {
	(struct SDL_Colour){ 0x35, 0x51, 0x7E, 0xFF },
	(struct SDL_Colour){ 0x3A, 0x86, 0xFF, 0xFF },
	(struct SDL_Colour){ 0xF5, 0xDD, 0x90, 0xFF },
	(struct SDL_Colour){ 0x7F, 0xD7, 0x80, 0xFF },
	(struct SDL_Colour){ 0x21, 0xA1, 0x79, 0xFF },
	(struct SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF },
};
char *g_world_colour_names[WORLD_LAYER_COUNT] = {
	"Deep Water",
	"Shallow Water",
	"Beaches",
	"Grasslands",
	"Forests",
	"Mountains",
};
SpriteID g_world_sprites[WORLD_LAYER_COUNT][8] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 9, 10, 11, 10, 10, 9, 9 },
	{ 16, 16, 16, 16, 16, 16, 16, 16, },
	{ 24, 24, 24, 24, 24, 24, 24, 24, },
};

void World_Init() {

}

void World_Draw() {
	// DEBUG: Fill view box in red to make leaks obvious
	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){
		0, 0,
		VIEW_WIDTH, VIEW_HEIGHT
	});


	int w_chunks = VIEW_WIDTH / CHUNK_SIZE + 2;
	int h_chunks = VIEW_HEIGHT / CHUNK_SIZE + 2;

	float camera_xf = (float) g_camera_x / CHUNK_SIZE;
	float camera_yf = (float) g_camera_y / CHUNK_SIZE;
	int first_x_chunk = camera_xf;
	int first_y_chunk = camera_yf;
	int offset_x = (camera_xf - first_x_chunk) * CHUNK_SIZE;
	int offset_y = (camera_yf - first_y_chunk) * CHUNK_SIZE;

	for (int y=0; y<h_chunks; y++) {
		for (int x=0; x<w_chunks; x++) {
			World_DrawChunk(
				first_x_chunk + x,
				first_y_chunk + y,
				CHUNK_SIZE * (x-1) - offset_x,
				CHUNK_SIZE * (y-1) - offset_y
			);
		}
	}
}

//void World_Save(World_File *world) {
//
//}
//
//World_File *World_Load(const char* filename) {
//	return NULL;
//}
//
//World_Chunk *World_ReadChunk(World_File *world, int x, int y) {
//	return NULL;
//}
//
//void World_WriteChunk(World_File *world, int x, int y, World_Chunk *chunk) {
//
//}

void World_DrawChunk(int chunk_x, int chunk_y, int screen_x, int screen_y) {
	Uint32 rand = (chunk_x | 0xE20F8780) ^ (chunk_y | 0x4A88DFA2) ^ 0x5A6D02DB;
	int temp = (rand & 0x0F);
	for (int i=0; i<temp; i++) {
		Uint16 bit = ((rand >> 0) ^ (rand >> 1) ^ (rand >> 2) ^ (rand >> 3)) & 1;
		rand = (rand >> 1) | (bit << 31);
	}

	int sprite_x = (rand >> 4) % (CHUNK_SIZE - g_spsh_world->sprite_width);
	int sprite_y = (rand >> 8) % (CHUNK_SIZE - g_spsh_world->sprite_height);
	SpriteID sprite = NULL_SPRITE;
	Uint8 anim_index = (SDL_GetTicks() / 100 + rand) & 0b11;

	for (int v=0; v<CHUNK_SIZE; v++) {
		for (int u=0; u<CHUNK_SIZE; u++) {
			float fx = chunk_x + (float)(u) / CHUNK_SIZE;
			float fy = chunk_y + (float)(v) / CHUNK_SIZE;
			float res = perlin2d(fx, fy, NOISE_FREQ, NOISE_DEPTH);

			SDL_Colour clr = WORLD_BLACK_CLR;
			for (int i=0; i<WORLD_LAYER_COUNT; i++) {
				if (res < g_world_thresholds[i]) continue;
				clr = g_world_colours[i];
				
				if (rand < WORLD_RANDOM_THRESH
					&& u == sprite_x + WORLD_SPRITE_HOTSPOT_X
					&& v == sprite_y + WORLD_SPRITE_HOTSPOT_Y
				) sprite = g_world_sprites[i][anim_index];
			}

			// DEBUG
			//SDL_SetRenderDrawColor(g_renderer, 0xFF*res, 0xFF*res, 0xFF*res, 0xFF); // Draw Greyscale Noise directly
			//if (rand < db_thresh)
			//	SDL_SetRenderDrawColor(g_renderer, 0x80, 0x80, 0xFF, 0xFF);
			//else if (u == sprite_x + WORLD_SPRITE_HOTSPOT_X && v == sprite_y + WORLD_SPRITE_HOTSPOT_Y)
			//	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x80, 0x80, 0xFF);
			//else
			//	SDL_SetRenderDrawColor(g_renderer, rand&0xFF, rand&0xFF, rand&0xFF, 0xFF); // Draw Greyscale LFSR rand value directly

			SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(clr));
			SDL_RenderDrawPoint(g_renderer, screen_x + u, screen_y + v);

			Uint16 bit = ((rand >> 0) ^ (rand >> 1) ^ (rand >> 2) ^ (rand >> 3)) & 1;
			rand = (rand >> 1) | (bit << 15);
		}
	}

	if (sprite != NULL_SPRITE)
		Sprite_Draw(g_spsh_world, screen_x + sprite_x, screen_y + sprite_y, sprite);
}