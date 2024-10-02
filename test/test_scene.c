
#include <SDL2/SDL.h>
#include "../include/screen.h"
#include "../include/scene.h"
#include "../include/binding.h"

static int x = 0;
static int y = 0;

void scn_test_handle_events(SDL_Event evt) {
	switch (evt.type) {

		case SDL_KEYDOWN: {
			Input_Type in = Binding_ConvKeyCode(evt.key.keysym.sym);
			switch (in) {
				case INPUT_RIGHT: if (x<10) x++; break;
				case INPUT_LEFT: if (x>0) x--; break;
				case INPUT_DOWN: if (y<10) y++; break;
				case INPUT_UP: if (y>0) y--; break;
				case INPUT_BACK: Scene_Set("menu"); break;
				default: break;
			};
		} break;

	}
}

void scn_test_draw_frame() {
	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(g_renderer);

	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(g_renderer, &(struct SDL_Rect){
		10 + x * 5, 10 + y * 5,
		25, 25
	});
}

Scene scn_test = {
	.setup = NULL,
	.teardown = NULL,
	.handle_events = &scn_test_handle_events,
	.draw_frame = &scn_test_draw_frame,
};