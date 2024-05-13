#include "txtbox.h"

Textbox g_txtbox_queue[TXTBOX_QUEUE_LEN];

static int queue_index = 0;


void Txtbox_DrawTextbox(Spritesheet *sheet, Textbox txtbox) {

	// Render Box
	int text_height = (sheet->sprite_height * TXTBOX_ROWS) + (TEXT_KERN * (TXTBOX_ROWS-1));
	SDL_Rect box = {
		0 + TXTBOX_MARGIN,
		SCREEN_HEIGHT - (TXTBOX_MARGIN*2) - text_height - 2 - (TXTBOX_PAD_Y*2),
		SCREEN_WIDTH - (TXTBOX_MARGIN*2),
		(TXTBOX_MARGIN*2) + text_height + (TXTBOX_PAD_Y*2),
	};
	SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(TXTBOX_CLR_BG));
	SDL_RenderFillRect(g_renderer, &box);
	SDL_SetRenderDrawColor(g_renderer, CLR_RGBA(TXTBOX_CLR_BORDER));
	SDL_RenderDrawRect(g_renderer, &box);

	// Render Text
	int col = 0;
	int row = 0;
	for (int i=0; i<TXTBOX_LEN && txtbox[i] != '\0'; i++) {
		char c = txtbox[i];

		if (row >= TXTBOX_ROWS-1 && col >= TXTBOX_COLS) return;
		if (row < TXTBOX_ROWS-1 && (c == '\n' || col >= TXTBOX_COLS)) {
			col = 0;
			row++;
		}
		if (c < ' ') continue;

		SpriteID spid = txtbox[i] - ' ';
		int char_x = box.x + 1 + col * (g_spsh_text->sprite_width + TEXT_KERN) + TXTBOX_PAD_X;
		int char_y = box.y + 1 + row * (g_spsh_text->sprite_height + TEXT_KERN) + TXTBOX_PAD_Y;
		Sprite_Draw(g_spsh_text, char_x, char_y, spid);
		col++;
	}
}

void Txtbox_AddToQueue(Textbox txtbox) {
	if (queue_index >= TXTBOX_QUEUE_LEN) return;
	SDL_memmove(g_txtbox_queue[queue_index++], txtbox, TXTBOX_LEN);
}