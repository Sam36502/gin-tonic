#ifndef TXTBOX_H
#define TXTBOX_H

#include <stdbool.h>
#include "sprite.h"

#define TXTBOX_MARGIN 1 // px
#define TXTBOX_PAD_X 2 // px
#define TXTBOX_PAD_Y 1 // px
#define TXTBOX_ROWS 3
#define TXTBOX_COLS 20
#define TXTBOX_LEN (TXTBOX_ROWS * TXTBOX_COLS)
#define TXTBOX_CLR_BG (struct SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }
#define TXTBOX_CLR_BORDER (struct SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF }
#define TXTBOX_QUEUE_LEN 0x40

typedef char Textbox[TXTBOX_LEN];

extern Textbox g_txtbox_queue[TXTBOX_QUEUE_LEN];

void Txtbox_DrawTextbox(Spritesheet *sheet, Textbox txtbox);
void Txtbox_AddToQueue(Textbox txtbox);
bool Txtbox_IsQueueFull();
bool Txtbox_IsQueueEmpty();

#endif