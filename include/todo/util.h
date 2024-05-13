#ifndef UTIL_H
#define UTIL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>
#include <stdbool.h>

#define TICK_MS 20
#define UEVENT_CODE_CLOCK 0x01

#define CLR_RGBA(clr) clr.r, clr.g, clr.b, clr.a

#define U8_TO_U16(v,h,l) v = (h << 8) | l;

#define U16_TO_U8(v,h,l) h = v >> 8; l = v & 0xFF;

typedef float vector[2];

extern bool g_isRunning;
extern bool g_debug;


void Util_Init();				// Init SDL and user event system
void Util_ErrMsg(const char *msg);	// Display error message and exit program
//void Util_DrawText(int x, int y, char *str, size_t len);	// Draw a text string to the screen
void Util_DrawBinary(int x, int y, Uint32 num);				// Draw a Uint32 as black and white pixels
//void Util_DrawVector(int x, int y, vector vec);				// Draw a red vector
void Util_FormatIP(char *buf, size_t buflen, IPaddress ip);	// Formats an IP-Address into a string

#endif