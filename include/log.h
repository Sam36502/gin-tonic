#ifndef GT_LOG_H
#define GT_LOG_H
//	
//				Logging Utilities
//	
//		Handles Basic logging and global error-handling
//	
//		(Hierarchy-Level: Bottom)
//	

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
//#include <stdarg.h> // Needed if making message with variables


//	
//		Constant Definitions
//	

// Define Output Thresholds
#if defined(GT_ENVIRONMENT_DEV)
	#define LOG_PRINT_LEVEL LOG_DEBUG
	#define LOG_POPUP_LEVEL LOG_INFO
#else
	#define LOG_PRINT_LEVEL LOG_WARNING
	#define LOG_POPUP_LEVEL LOG_ERROR
#endif

// Log Level Text
#define LOGLVL_TEXT_DEBUG	"DEBUG"
#define LOGLVL_TEXT_INFO	"INFO "
#define LOGLVL_TEXT_WARNING	"WARN "
#define LOGLVL_TEXT_ERROR	"ERROR"
#define LOGLVL_TEXT_FATAL	"FATAL"

// Define calling macros to make log use easier
#define Log_Message(lvl, msg) Log_MessageFull(g_window, lvl, msg, false)
#define Log_SDLMessage(lvl, msg) Log_MessageFull(g_window, lvl, msg, true)

//	
//		Type Definitions
//	

typedef enum {		// Usage guide:
	LOG_DEBUG,		// Info only for helping find bugs
	LOG_INFO,		// Info that could be useful to any user
	LOG_WARNING,	// Something went wrong, but it shouldn't impact the system
	LOG_ERROR,		// Something went wrong, and will have a negative impact, but it shouldn't crash the system
	LOG_FATAL		// System cannot continue, automatically exits
} Log_Level;


//	
//		Global Variable Declarations
//	


//	
//		Function Declarations
//	

void Log_SetPrintLevel(Log_Level lvl);
void Log_SetPopupLevel(Log_Level lvl);
void Log_MessageFull(SDL_Window *window, Log_Level lvl, const char *msg, bool is_sdl_error);

#endif
