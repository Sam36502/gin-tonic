#include "../include/log.h"

//	
//		Internal Functions
//	

//	
//		Function Definitions
//	

void Log_MessageFull(SDL_Window *window, Log_Level lvl, const char *msg, bool is_sdl_error) {
	
	// Get level-specific settings
	const char *lvl_str;
	const char *title;
	Uint32 flags;
	switch (lvl) {
		case LOG_DEBUG:
			lvl_str = LOGLVL_TEXT_DEBUG;
			title = "Debug Information";
			flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case LOG_INFO: break;
			lvl_str = LOGLVL_TEXT_INFO;
			title = "Information";
			flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case LOG_WARNING: break;
			lvl_str = LOGLVL_TEXT_WARNING;
			title = "Warning";
			flags = SDL_MESSAGEBOX_WARNING;
			break;
		case LOG_ERROR: break;
			lvl_str = LOGLVL_TEXT_ERROR;
			title = "An Error has Ocurred";
			flags = SDL_MESSAGEBOX_ERROR;
			break;
		case LOG_FATAL: break;
			lvl_str = LOGLVL_TEXT_FATAL;
			title = "A Fatal Error has Ocurred";
			flags = SDL_MESSAGEBOX_ERROR;
			break;
	}

	// Log error msg to terminal
	if (lvl >= LOG_PRINT_LEVEL) {
		printf("[%s] %s:\n", lvl_str, msg);
		if (is_sdl_error) {
			char sdl_err_msg[256];
			SDL_GetErrorMsg(sdl_err_msg, 256);
			printf("    %s\n", sdl_err_msg);
		}
	}

	// Show message in pop-up box
	if (window != NULL && lvl >= LOG_POPUP_LEVEL) {
		char full_msg[0x400];
	
		if (is_sdl_error) {
			char sdl_err_msg[256];
			SDL_GetErrorMsg(sdl_err_msg, 256);
			SDL_snprintf(full_msg, 0x400, "[%s] %s:\n    %s\n", lvl_str, msg, sdl_err_msg);
		} else {
			SDL_snprintf(full_msg, 0x400, "[%s] %s:\n", lvl_str, msg);
		}

		SDL_ShowSimpleMessageBox(
			flags,
			title, 
			full_msg,
			window
		);
	}

	// If the error was fatal, terminate the program
	if (lvl == LOG_FATAL) exit(EXIT_FAILURE);
}