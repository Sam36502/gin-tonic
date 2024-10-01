#include "../include/scene.h"

static Scene __scenes[MAX_SCENES];
static char *__scene_names[MAX_SCENES];
static int __scene_count;

static Scene __curr_scene;

int Scene_Register(Scene scn, char *name) {
	if (__scene_count >= MAX_SCENES) return 1;

	size_t name_len = SDL_strlen(name) + 1;
	int index = __scene_count++;
	__scene_names[index] = SDL_malloc(sizeof(char) * name_len);
	SDL_memcpy(__scene_names[index], name, name_len);
	__scenes[index] = scn;

	return 0;
}

static int __find_scene(char *name) {
	for (int i=0; i<__scene_count; i++) {
		if (SDL_strcmp(name, __scene_names[i]) == 0) {
			return i;
		}
	}
	return -1;
}

void Scene_Remove(char *name) {
	int index = __find_scene(name);
	if (index < 0) return;

	int last = --__scene_count;
	SDL_free(__scene_names[index]);
	__scene_names[index] = __scene_names[last];
	__scenes[index] = __scenes[last];
}

void Scene_Set(char *name) {
	int index = __find_scene(name);
	if (index < 0) return;

	if (__curr_scene.teardown != NULL) __curr_scene.teardown();
	__curr_scene = __scenes[index];
	if (__curr_scene.setup != NULL) __curr_scene.setup();
}

void Scene_Execute() {
	bool redraw = true;
	SDL_Event curr_event;
	while (g_isRunning) {
		if (SDL_WaitEvent(&curr_event) != 0) {
			Events_HandleInternal(curr_event);

			if (__curr_scene.handle_events != NULL) __curr_scene.handle_events(curr_event);

			// Clock Tick
			if (curr_event.type == SDL_USEREVENT && curr_event.user.code == UEVENT_CLOCK_TICK) {
				redraw = true;
			}
		}

		if (redraw) {
			if (__curr_scene.draw_frame != NULL) __curr_scene.draw_frame();
			SDL_RenderPresent(g_renderer);
		}

	}
}
