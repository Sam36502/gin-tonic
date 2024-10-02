#include "../include/text.h"


Spritesheet *g_text_spsh;

static SDL_Surface *__getEmbeddedSurface() {

	Uint8 *data = (Uint8 *) _binary_embeds_text_sprites_start;
	int width = *(int *)(data); data += sizeof(int);
	int height = *(int *)(data); data += sizeof(int);

	// Generate greyscale surface & palette
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
	if (surf == NULL) Log_SDLMessage(LOG_FATAL, "Failed to create greyscale surface");
	SDL_Colour clrs[0x100];
	for (int i=0; i<0x100; i++) clrs[i] = (SDL_Colour){ i, i, i, 0xFF };
	SDL_SetPaletteColors(surf->format->palette, clrs, 0, 0x100);

	// Load embedded pixel data
	SDL_memcpy(surf->pixels, data, _binary_embeds_text_sprites_end - data);

	return surf;
}


void Text_Init(char *text_bitmap_file) {
	if (text_bitmap_file == NULL) {
		SDL_Surface *surf_emb = __getEmbeddedSurface();
		g_text_spsh = Sprite_LoadSheetGridFromSurface(surf_emb, TEXT_SPRITE_WIDTH, TEXT_SPRITE_HEIGHT);
		SDL_FreeSurface(surf_emb);
	} else {
		g_text_spsh = Sprite_LoadSheetGrid(text_bitmap_file, TEXT_SPRITE_WIDTH, TEXT_SPRITE_HEIGHT);
	}

	if (g_text_spsh == NULL) {
		Log_Message(LOG_FATAL, "Failed to load text spritesheet");
	}
}

void Text_Term() {
	Sprite_FreeSheet(g_text_spsh);
}

void Text_SetColour(SDL_Colour clr) {
	SDL_SetTextureColorMod(g_text_spsh->texture, CLR_TO_RGB(clr));
}

void Text_ResetColour() {
	Text_SetColour(TEXT_BASE_COLOUR);
}

void Text_HandleEscape(char *str, int *index) {
	(*index)++;
	Uint8 cmd = str[(*index)++];
	switch (cmd) {
		case 0x00: Text_ResetColour(); break;

		case 0x01: {
			Uint8 r = str[(*index)++];
			Uint8 g = str[(*index)++];
			Uint8 b = str[(*index)++];
			Text_SetColour((SDL_Colour){
				r, g, b, 0xFF
			});
		} break;
	}
	(*index)--;
}

void Text_Draw(char *str, size_t str_len, int x, int y) {
	int col = 0;
	for (int i=0; i<str_len && str[i] != '\0'; i++)	 {
		Uint8 c = str[i];
		if (c == TEXT_ESC_CHAR) {
			Text_HandleEscape(str, &i);
			continue;
		}
		if (c < ' ') continue;
		SpriteID sid = c - ' ';
		int x_pos = x + col * (TEXT_SPRITE_WIDTH + TEXT_KERN);
		Sprite_Draw(g_text_spsh, x_pos, y, sid);
		col++;
	}
}

void Text_PrintEncoding(int x, int y) {
	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xFF);

	Text_Draw("\x7F 0123456789ABCDEF", 18, x, y);

	char row_nr[3];
	char ch = ' ';
	for (int cy=0; cy<8; cy++) {
		int screen_y = y + (1+cy)*(1+TEXT_SPRITE_HEIGHT);

		SDL_snprintf(row_nr, 3, "%01X ", cy+2);
		Text_Draw(row_nr, 2, x, screen_y);

		for (int cx=0; cx<16; cx++) {
			int screen_x = x + (2+cx)*(1+TEXT_SPRITE_WIDTH);

			SDL_RenderFillRect(g_renderer,
				&(struct SDL_Rect){
					screen_x, screen_y, TEXT_SPRITE_WIDTH, TEXT_SPRITE_HEIGHT
				}
			);

			Text_Draw(&ch, 1, screen_x, screen_y);
			ch++;
		}
	}
}

Textbox *Text_Box_Create(char *str, size_t str_len, int cols, int rows) {
	Textbox *box = SDL_malloc(sizeof(Textbox));
	box->cols = cols;
	box->rows = rows;
	box->string = SDL_malloc(sizeof(char) * cols * rows);

	for (int i=0; i<cols*rows; i++) {
		Uint8 c = 0x00;
		if (i < str_len) c = str[i];
		box->string[i] = c;
	}

	return box;
}

void Text_Box_Destroy(Textbox *txtbox) {
	SDL_free(txtbox->string);
	SDL_free(txtbox);
}

void Text_Box_SetText(Textbox *txtbox, char *str) {
	int size = txtbox->cols * txtbox->rows + 1;
	int newlen = SDL_strlen(str) + 1;
	if (newlen > size) newlen = size;

	txtbox->string = SDL_realloc(txtbox->string, sizeof(char) * newlen);
	SDL_memcpy(txtbox->string, str, newlen);
}

void Text_Box_SetSize(Textbox *txtbox, int w, int h) {
	if (w >= 0) txtbox->cols = w;
	if (h >= 0) txtbox->rows = h;
	int size = txtbox->cols * txtbox->rows + 1;
	if (SDL_strlen(txtbox->string) + 1 > size) {
		txtbox->string[size-1] = '\0';
		txtbox->string = SDL_realloc(txtbox->string, sizeof(char) * size);
		for (int i=0; i<size; i++) txtbox->string[i] = 0x00;
	}
}

void Text_Box_Draw(Textbox *txtbox, int box_x, int box_y) {

	// Render Box
	SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(TXTBOX_CLR_BG));
	SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){
		box_x, box_y,
		TXTBOX_WIDTH(txtbox), TXTBOX_HEIGHT(txtbox),
	});
	SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(TXTBOX_CLR_BORDER));
	SDL_RenderDrawRect(g_renderer, &(struct SDL_Rect){
		box_x-1, box_y-1,
		TXTBOX_WIDTH(txtbox)+1, TXTBOX_HEIGHT(txtbox)+1,
	});

	// Render Text
	int col = 0;
	int row = 0;
	for (int i=0; i<txtbox->cols*txtbox->rows && txtbox->string[i] != '\0'; i++) {
		Uint8 ch = txtbox->string[i];

		if (ch == TEXT_ESC_CHAR) {
			Text_HandleEscape(txtbox->string, &i);
			continue;
		}

		if (row >= txtbox->rows-1 && col >= txtbox->cols) return;
		if (row < txtbox->rows-1 && (ch == '\n' || col >= txtbox->cols)) {
			col = 0;
			row++;
		}

		if (ch < ' ') continue;
		SpriteID spid = ch - ' ';

		int char_x = box_x + TXTBOX_PAD_X + col * (TEXT_SPRITE_WIDTH + TEXT_KERN);
		int char_y = box_y + TXTBOX_PAD_Y + row * (TEXT_SPRITE_HEIGHT + TEXT_KERN);
		Sprite_Draw(g_text_spsh, char_x, char_y, spid);
		col++;
	}
}
