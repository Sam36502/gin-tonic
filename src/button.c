#include "../include/button.h"

static Button *__buttons[MAX_BUTTONS];
static int __buttoncount = 0;
static bool mouse_held;

void Button_Term() {
	for (int i=0; i<__buttoncount; i++) {
		Button *b = __buttons[i];
		free(b);
	}
	__buttoncount = 0;
}

int Button_Create(
	SDL_Rect rect,
	bool is_toggleable,
	void (* callback_on)(int ID, void *udata),
	void (* callback_off)(int ID, void *udata),
	void *udata,
	SDL_Colour clr_off, SDL_Colour clr_hover, SDL_Colour clr_on 
) {
	Button *b = malloc(sizeof(Button));
	b->rect = rect;
	b->callback_on = callback_on;
	b->callback_off = callback_off;
	b->id = __buttoncount;
	b->udata = udata;
	b->is_toggle = is_toggleable;
    b->clr_off = clr_off;
    b->clr_hover = clr_hover;
    b->clr_on = clr_on;

	b->is_on = false;
	b->hovering = false;

	__buttons[__buttoncount] = b;
	__buttoncount++;
	return b->id;
}

void Button_DrawAll() {
	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
	for (int i=0; i<__buttoncount; i++) {
		Button *b = __buttons[i];

		if (b->is_on) {
			SDL_SetRenderDrawColor(g_renderer,
				b->clr_on.r,
				b->clr_on.g,
				b->clr_on.b,
				b->clr_on.a
			);
		} else if (b->hovering) {
			SDL_SetRenderDrawColor(g_renderer,
				b->clr_hover.r,
				b->clr_hover.g,
				b->clr_hover.b,
				b->clr_hover.a
			);
		} else {
			SDL_SetRenderDrawColor(g_renderer,
				b->clr_off.r,
				b->clr_off.g,
				b->clr_off.b,
				b->clr_off.a
			);
		}

		SDL_RenderFillRect(g_renderer, &b->rect);
	}
	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
}

Button *Button_Get(int id) {
	return __buttons[id];
}

void Button_HandleMouseButtonEvent(SDL_MouseButtonEvent event) {
	if (event.button != SDL_BUTTON_LEFT) return;

	mouse_held = event.type == SDL_MOUSEBUTTONDOWN;

	for (int i=__buttoncount-1; i>=0; i--) {
		Button *b = __buttons[i];

		if (b->hovering) {
			// Mouse clicked on button
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (b->is_toggle) {
					b->is_on = !b->is_on;

					if(b->is_on) {
						if (b->callback_on != NULL) (*b->callback_on)(b->id, b->udata);
					} else {
						if (b->callback_off != NULL) (*b->callback_off)(b->id, b->udata);
					}
				} else {
					b->is_on = true;
					if (b->callback_on != NULL) (*b->callback_on)(b->id, b->udata);
				}
			
			// Mouse button lifted on button
			} else {
				if (!b->is_toggle && b->is_on) {
					b->is_on = false;
					if (b->callback_off != NULL) (*b->callback_off)(b->id, b->udata);
				}
			}
		}
	}
}

void Button_HandleMouseMotionEvent(SDL_MouseMotionEvent event) {
	for (int i=0; i<__buttoncount; i++) {
		Button *b = __buttons[i];

			if (
				event.x >= b->rect.x
				&& event.x < b->rect.x+b->rect.w
				&& event.y >= b->rect.y
				&& event.y < b->rect.y+b->rect.h
			) {
				b->hovering = true;
				if (!b->is_on && !b->is_toggle && mouse_held) {
					if (b->callback_on != NULL) (*b->callback_on)(b->id, b->udata);
					b->is_on = true;
				}
			} else {
				b->hovering = false;
				if (b->is_on && !b->is_toggle) {
					if (b->callback_off != NULL) (*b->callback_off)(b->id, b->udata);
					b->is_on = false;
				}
			}
	}
}