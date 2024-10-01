#ifndef GT_SCENE_C
#define GT_SCENE_C
//
//		Basic Scene Manager
//		
//		Lets you create Scenes to abstract the main loop
//		into separate files

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "util.h"
#include "screen.h"
#include "events.h"

#define MAX_SCENES 0x100


//	
//	Type Definitions
//	

typedef struct {
	void (* draw_frame)();
	void (* handle_events)(SDL_Event event);
	void (* setup)();
	void (* teardown)();
} Scene;


//	
//	Function Declarations
//	

//	Registers a scene with a given name
//	
//	Returns 0 on success
int Scene_Register(Scene scn, char *name);

//	Removes a scene from the register
//	
void Scene_Remove(char *name);

//	Sets which scene is currently in use
//	
//	Should be called before `Scene_Execute()` at startup
void Scene_Set(char *name);

//	Starts the main loop with a given scene
//	
void Scene_Execute();


#endif