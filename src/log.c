#include "../include/log.h"

//	
//		Internal Global Variables
//	

static Log_Level _g_print_level = LOG_ERROR;
static Log_Level _g_popup_level = LOG_FATAL;

//	
//		Function Definitions
//	

void Log_SetPrintLevel(Log_Level lvl) {
	_g_print_level = lvl;
}

void Log_SetPopupLevel(Log_Level lvl) {
	_g_popup_level = lvl;
}

void Log_MessageFull(SDL_Window *window, Log_Level lvl, const char *msg, bool is_sdl_error) {
	
	// Get level-specific settings
	const char *lvl_str = "     ";
	const char *title;
	Uint32 flags;
	switch (lvl) {
		case LOG_DEBUG:
			lvl_str = LOGLVL_TEXT_DEBUG;
			title = "Debug Information";
			flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case LOG_INFO:
			lvl_str = LOGLVL_TEXT_INFO;
			title = "Information";
			flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case LOG_WARNING:
			lvl_str = LOGLVL_TEXT_WARNING;
			title = "Warning";
			flags = SDL_MESSAGEBOX_WARNING;
			break;
		case LOG_ERROR:
			lvl_str = LOGLVL_TEXT_ERROR;
			title = "An Error has Ocurred";
			flags = SDL_MESSAGEBOX_ERROR;
			break;
		case LOG_FATAL:
			lvl_str = LOGLVL_TEXT_FATAL;
			title = "A Fatal Error has Ocurred";
			flags = SDL_MESSAGEBOX_ERROR;
			break;
	}

	// Log error msg to terminal
	if (lvl >= _g_print_level) {
		printf("[%s] %s", lvl_str, msg);
		if (is_sdl_error) {
			char sdl_err_msg[256];
			SDL_GetErrorMsg(sdl_err_msg, 256);
			printf(":\n    %s\n", sdl_err_msg);
		} else {
			putchar('\n');
		}
	}
	fflush(stdout);

	// Show message in pop-up box
	if (lvl >= _g_popup_level) {
		char full_msg[0x400];
	
		if (is_sdl_error) {
			char sdl_err_msg[256];
			SDL_GetErrorMsg(sdl_err_msg, 256);
			SDL_snprintf(full_msg, 0x400, "[%s] %s:\n    %s\n", lvl_str, msg, sdl_err_msg);
		} else {
			SDL_snprintf(full_msg, 0x400, "[%s] %s\n", lvl_str, msg);
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