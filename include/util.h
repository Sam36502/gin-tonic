#ifndef UTIL_H
#define UTIL_H
//	
//				Generic / Basic Utilities
//	
//		Handles very basic general Utilities and is also
//		Where to declare truly universal variables
//	
//		(Hierarchy-Level: Bottom)
//	

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>


//	
//		Constants & Macro Definitions
//	

#define U8_TO_U16(v,h,l) v = (h << 8) | l;
#define U16_TO_U8(v,h,l) h = v >> 8; l = v & 0xFF;
#define PRINT_U8_PTN "%i%i%i%i%i%i%i%i"
#define PRINT_U8(n) (n>>7)&1,(n>>6)&1,(n>>5)&1,(n>>4)&1,(n>>3)&1,(n>>2)&1,(n>>1)&1,n&1


//	
//		Universal Variable Declarations
//	

//	Universal variable that determines whether the game is running
//	
//	can be freely set externally
extern bool g_isRunning;

#endif