#include "../include/text.h"


static Spritesheet *__text_spsh;

//static SDL_Surface *__getEmbeddedSurface() {
//
//	Uint8 *data = (Uint8 *) _binary_embeds_text_sprites_start;
//	int width = *(int *)(data); data += sizeof(int);
//	int height = *(int *)(data); data += sizeof(int);
//	int depth = *(int *)(data); data += sizeof(int);
//	int pitch = *(int *)(data); data += sizeof(int);
//
//	Uint32 Rmask = *(Uint32 *)(data); data += sizeof(Uint32);
//	Uint32 Gmask = *(Uint32 *)(data); data += sizeof(Uint32);
//	Uint32 Bmask = *(Uint32 *)(data); data += sizeof(Uint32);
//	Uint32 Amask = *(Uint32 *)(data); data += sizeof(Uint32);
//
//	SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(
//		data,
//		width, height, depth, pitch,
//		Rmask, Gmask, Bmask, Amask
//	);
//	if (surf == NULL) Log_SDLMessage(LOG_ERROR, "Failed to read embedded surface data");
//	return surf;
//}


void Text_Init(const char *text_bitmap_file) {
	__text_spsh = Sprite_LoadSheetGrid(text_bitmap_file, TEXT_SPRITE_WIDTH, TEXT_SPRITE_HEIGHT);
	if (__text_spsh == NULL) {
		Log_Message(LOG_ERROR, "Failed to load embedded text spritesheet");
	}
}

void Text_Term() {
	Sprite_FreeSheet(__text_spsh);
}

void Text_Draw(char *str, size_t str_len, int x, int y) {
	for (int i=0; i<str_len && str[i] != '\0'; i++)	 {
		char c = str[i];
		if (c < ' ') continue;
		SpriteID sid = c - ' ';
		int x_pos = x + i * (TEXT_SPRITE_WIDTH + 1);
		Sprite_Draw(__text_spsh, x_pos, y, sid);
	}
}

Textbox *TextBox_Create(char *str, size_t str_len, int cols, int rows) {
	Log_Message(LOG_ERROR, "Sorry, this function is not implemented yet.");
	return NULL;
}

void TextBox_DrawTextbox(Textbox *txtbox, int x, int y) {
	Log_Message(LOG_ERROR, "Sorry, this function is not implemented yet.");

	//// Render Box
	//int text_height = (sheet->sprite_height * TXTBOX_ROWS) + (TEXT_KERN * (TXTBOX_ROWS-1));
	//SDL_Rect box = {
		//0 + TXTBOX_MARGIN,
		//SCREEN_HEIGHT - (TXTBOX_MARGIN*2) - text_height - 2 - (TXTBOX_PAD_Y*2),
		//SCREEN_WIDTH - (TXTBOX_MARGIN*2),
		//(TXTBOX_MARGIN*2) + text_height + (TXTBOX_PAD_Y*2),
	//};
	//SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(TXTBOX_CLR_BG));
	//SDL_RenderFillRect(g_renderer, &box);
	//SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(TXTBOX_CLR_BORDER));
	//SDL_RenderDrawRect(g_renderer, &box);

	//// Render Text
	//int col = 0;
	//int row = 0;
	//for (int i=0; i<TXTBOX_LEN && txtbox[i] != '\0'; i++) {
		//char c = txtbox[i];

		//if (row >= TXTBOX_ROWS-1 && col >= TXTBOX_COLS) return;
		//if (row < TXTBOX_ROWS-1 && (c == '\n' || col >= TXTBOX_COLS)) {
			//col = 0;
			//row++;
		//}
		//if (c < ' ') continue;

		//SpriteID spid = txtbox[i] - ' ';
		//int char_x = box.x + 1 + col * (g_spsh_text->sprite_width + TEXT_KERN) + TXTBOX_PAD_X;
		//int char_y = box.y + 1 + row * (g_spsh_text->sprite_height + TEXT_KERN) + TXTBOX_PAD_Y;
		//Sprite_Draw(g_spsh_text, char_x, char_y, spid);
		//col++;
	//}
}