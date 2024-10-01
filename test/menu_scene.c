
#include <SDL2/SDL.h>
#include "../include/screen.h"
#include "../include/scene.h"
#include "../include/menu.h"

static Menu *__menu = NULL;

void scn_menu_cb_play() {
	Scene_Set("test");
}

void scn_menu_cb_hello() {
	puts("Hello, world!");
	fflush(stdout);
}

void scn_menu_cb_exit() {
	g_isRunning = false;
}

void scn_menu_setup() {
	__menu = Menu_Create(NULL);
	Menu_AddOption(__menu, "Play Game", &scn_menu_cb_play);
	Menu_AddOption(__menu, "Say Hello", &scn_menu_cb_hello);
	Menu_AddOption(__menu, "Exit", &scn_menu_cb_exit);
}

void scn_menu_teardown() {
	Menu_Destroy(__menu);
}

void scn_menu_handle_events(SDL_Event evt) {
	Menu_HandleInput(__menu, evt);
}

void scn_menu_draw_frame() {
	SDL_SetRenderDrawColor(g_renderer, 0x2c, 0x8d, 0xa3, 0xFF);
	SDL_RenderClear(g_renderer);

	Menu_Draw(__menu, 10, 10);
}

Scene scn_menu = {
	.setup = &scn_menu_setup,
	.teardown = &scn_menu_teardown,
	.handle_events = &scn_menu_handle_events,
	.draw_frame = &scn_menu_draw_frame,
};