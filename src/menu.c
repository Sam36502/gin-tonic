#include "../include/menu.h"

Menu *Menu_Create(char *title) {
	Menu *menu = SDL_malloc(sizeof(Menu));

	if (title != NULL) {
		int len = SDL_strlen(title) + 1;
		menu->title = SDL_malloc(sizeof(char) * len);
		menu->title_len = len;
		SDL_memcpy(menu->title, title, len);
		menu->width = 2*TXTBOX_PAD_X + (len + 2*MENU_TITLE_PAD)*(TEXT_SPRITE_WIDTH+TEXT_KERN);
		menu->height = 2*TXTBOX_PAD_Y + 1*(TEXT_SPRITE_HEIGHT+TEXT_KERN);
	} else {
		menu->title = NULL;
		menu->title_len = 0;
		menu->width = 2*TXTBOX_PAD_X + 1*TEXT_KERN;
		menu->height = 2*TXTBOX_PAD_Y + 1*TEXT_KERN;
	}

	menu->option_count = 0;
	menu->selected = 0;
	return menu;
}

void Menu_Destroy(Menu *menu) {
	for (int i=0; i<menu->option_count; i++) SDL_free(menu->options[i].text);
	if (menu->title != NULL) SDL_free(menu->title);
	SDL_free(menu);
}

void Menu_AddOption(Menu *menu, char *text, void (* cb_selected)()) {
	int txtlen = SDL_strlen(text) + 1;
	Menu_Option opt = {
		.text = SDL_malloc(sizeof(char) * txtlen),
		.len = txtlen,
		.cb_selected = cb_selected,
	};
	SDL_memcpy(opt.text, text, txtlen);
	menu->options[menu->option_count++] = opt;

	//char *str = menu->txtbox->string;
	//char buf[256];
	//SDL_snprintf(buf, 256, "%s\n  %s", str, text);
	//Text_Box_SetText(menu->txtbox, buf);
	
	int width = (txtlen-1+2*MENU_OPT_PAD) * (TEXT_SPRITE_WIDTH+TEXT_KERN);
	if (width > menu->width) menu->width = width;
	menu->height += TEXT_SPRITE_HEIGHT + TEXT_KERN;
}

void Menu_Draw(Menu *menu, int x, int y) {
	// Render Box
	SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(TXTBOX_CLR_BG));
	SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){
		x, y,
		menu->width, menu->height,
	});
	SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(TXTBOX_CLR_BORDER));
	SDL_RenderDrawRect(g_renderer, &(struct SDL_Rect){
		x-1, y-1,
		menu->width+1, menu->height+1,
	});

	int text_x = x + TXTBOX_PAD_X;
	int text_y = y + TXTBOX_PAD_Y;

	// Draw Title
	if (menu->title != NULL) {
		int title_x = text_x + MENU_TITLE_PAD * (TEXT_SPRITE_WIDTH + TEXT_KERN);
		Text_Draw(menu->title, menu->title_len, title_x, text_y);
		text_y += TEXT_SPRITE_HEIGHT;
	}

	if (menu->option_count > 0) {
		// Draw Options
		int opt_y = text_y;
		int opt_x = text_x + MENU_OPT_PAD * (TEXT_SPRITE_WIDTH + TEXT_KERN);
		for (int i=0; i<menu->option_count; i++) {
			Menu_Option opt = menu->options[i];
			Text_Draw(opt.text, opt.len, opt_x, opt_y);
			opt_y += TEXT_KERN + TEXT_SPRITE_HEIGHT;
		}

		// Draw Cursor
		int cursor_x = x + TXTBOX_PAD_X + TEXT_SPRITE_WIDTH;
		for (int i=0; i<menu->selected; i++) text_y += TEXT_SPRITE_HEIGHT + TEXT_KERN;
		Sprite_Draw(g_text_spsh, cursor_x, text_y, MENU_CURSOR_CHAR);
	}
}

void Menu_HandleEvent(Menu *menu, SDL_Event evt) {
	if (evt.type == SDL_KEYUP) {
		switch (evt.key.keysym.sym) {

			case SDLK_UP:
			case SDLK_w:
			case SDLK_k: {
				if (menu->selected > 0) menu->selected--;
			} break;

			case SDLK_DOWN:
			case SDLK_s:
			case SDLK_j: {
				if (menu->selected < menu->option_count-1) menu->selected++;
			} break;

			case SDLK_RETURN: {
				menu->options[menu->selected].cb_selected();
			} break;

		}
		return;
	}
}

void Menu_HandleInput(Menu *menu, SDL_Event evt) {
	if (evt.type == SDL_KEYUP) {
		Input_Type in = Binding_ConvKeyCode(evt.key.keysym.sym);
		switch (in) {

			case INPUT_UP: {
				if (menu->selected > 0) menu->selected--;
			} break;

			case INPUT_DOWN: {
				if (menu->selected < menu->option_count-1) menu->selected++;
			} break;

			case INPUT_SELECT: {
				menu->options[menu->selected].cb_selected();
			} break;

			default: break;
		}
		return;
	}
}