#include "../include/slider.h"

static Slider *__g_sliders[MAX_SLIDERS];
static unsigned int __g_slidercount = 0;

static int __knob_w = 10;
static int __knob_l = 30;
static SDL_Colour __knob_clr = { 0xAA, 0xAA, 0xAA, 0xFF };
static int __slider_w = 4;
static SDL_Colour __slider_clr = { 0x11, 0x11, 0x11, 0xFF };

void Slider_SetKnobStyle(int width, int length, SDL_Colour colour) {
	__knob_w = width;
	__knob_l = length;
	__knob_clr = colour;
}

void Slider_SetSliderStyle(int width, SDL_Colour colour) {
	__slider_w = width;
	__slider_clr = colour;
}

int Slider_Create(int x, int y, int length, SliderOrientation orient, float *var, float factor) {
	Slider *s = malloc(sizeof(Slider));
	s->x = x;
	s->y = y;
	s->length = length;
	s->orient = orient;
	s->factor = factor;
	s->var = var;
	s->is_held = false;
	s->enabled = true;

	__g_sliders[__g_slidercount] = s;
	return __g_slidercount++;
}

Slider *Slider_Get(int id) {
	if (id < 0 || id >= __g_slidercount) return NULL;
	return __g_sliders[id];
}

// Removes all Sliders
void Slider_Term() {
	for (int i=0; i<__g_slidercount; i++) {
		free(__g_sliders[i]);
	}
	__g_slidercount = 0;
}

SDL_Rect *__calc_rect_knob(Slider *s, SDL_Rect *r) {
	int val = *s->var * s->length * s->factor;
	if (s->orient == SLIDER_HORIZONTAL) {
		r->x = s->x + val - __knob_w/2;
		r->y = s->y - __knob_l/2;
		r->w = __knob_w;
		r->h = __knob_l;
		return r;
	} else {
		r->x = s->x - __knob_l/2;
		r->y = s->y + s->length - val - __knob_w/2;
		r->w = __knob_l;
		r->h = __knob_w;
		return r;
	}
}

void Slider_DrawAll() {
	for (int i=0; i<__g_slidercount; i++) {
		Slider *s = __g_sliders[i];

		SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(__slider_clr));
		if (s->orient == SLIDER_HORIZONTAL) {
			SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){ s->x, s->y-__slider_w/2, s->length, __slider_w });
		} else {
			SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){ s->x-__slider_w/2, s->y, __slider_w, s->length });
		}

		SDL_SetRenderDrawColor(g_renderer, CLR_TO_RGBA(__knob_clr));
		SDL_Rect r;
		SDL_RenderFillRect(g_renderer, __calc_rect_knob(s, &r));
	}
}

void Slider_HandleMouseButtonEvent(SDL_MouseButtonEvent event) {
	if (event.button != SDL_BUTTON_LEFT) return;

	for (int i=0; i<__g_slidercount; i++) {
		Slider *s = __g_sliders[i];
		if (!s->enabled) continue;
		
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			SDL_Rect knob;
			__calc_rect_knob(s, &knob);
			if (event.x >= knob.x
				&& event.y >= knob.y
				&& event.x <= knob.x+knob.w
				&& event.y <= knob.y+knob.h
			) {
				s->is_held = true;
			}
		}

		if (event.type == SDL_MOUSEBUTTONUP && s->is_held) s->is_held = false;
	}
}

void Slider_HandleMouseMotionEvent(SDL_MouseMotionEvent event) {
	for (int i=0; i<__g_slidercount; i++) {
		Slider *s = __g_sliders[i];
		if (!s->enabled) continue;

		if (s->is_held) {
			if (s->orient == SLIDER_HORIZONTAL) {
				int newx = event.x;
				if (newx < s->x) newx = s->x;
				if (newx > s->x+s->length) newx = s->x+s->length;
				*s->var = ((float) newx - s->x)/s->length;
			} else {
				int newy = event.y;
				if (newy < s->y) newy = s->y;
				if (newy > s->y+s->length) newy = s->y+s->length;
				*s->var = (s->length - ((float) newy - s->y))/s->length;
			}
		}
	}
}
