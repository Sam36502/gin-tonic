#ifndef WORLD_H
#define WORLD_H

#include "sprite.h"
#include "util.h"

#define CHUNK_SIZE 16
#define MOVEMENT_MARGIN 2 * CHUNK_SIZE
#define VIEW_WIDTH SCREEN_WIDTH
#define GRASS_THRESHOLD 0.5
#define NOISE_FREQ 0.25
#define NOISE_DEPTH 5
#define VIEW_HEIGHT SCREEN_HEIGHT
#define WORLD_WATER_CLR (struct SDL_Colour){ 0x7C, 0xAF, 0xC4, 0xFF }
#define WORLD_GRASS_CLR (struct SDL_Colour){ 0x8E, 0xD0, 0x81, 0xFF }
#define WORLD_BLACK_CLR (struct SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
#define WORLD_RANDOM_THRESH 14230
#define WORLD_SPRITE_HOTSPOT_X 3
#define WORLD_SPRITE_HOTSPOT_Y 7

#define WORLD_LAYER_COUNT 6

extern int g_camera_x; // World coordinates of the top-left corner of the view window
extern int g_camera_y;

extern SDL_Colour g_world_colours[WORLD_LAYER_COUNT];
extern char *g_world_colour_names[WORLD_LAYER_COUNT];
extern float g_world_thresholds[WORLD_LAYER_COUNT];
extern float g_world_thresholds[WORLD_LAYER_COUNT];
extern SpriteID g_world_sprites[WORLD_LAYER_COUNT][8];

typedef struct World_File {
	const char *filename;
	FILE *file;
} World_File;

// World Tiles
/*
typedef Uint8 World_TileID;
#define WT_AIR		0x00
#define WT_GRASS	0x01
#define WT_NULL		0xFF

typedef struct {
	Sint32 x_coord;
	Sint32 y_coord;
	World_TileID tiles[CHUNK_SIZE*CHUNK_SIZE];
} World_Chunk;
*/

void World_Init();
void World_Draw();
World_File *World_Load(const char* filename);
void World_Save(World_File *world);
//World_Chunk *World_ReadChunk(World_File *world, int x, int y);
//void World_WriteChunk(World_File *world, int x, int y, World_Chunk *chunk);

void World_DrawChunk(int chunk_x, int chunk_y, int screen_x, int screen_y);

#endif