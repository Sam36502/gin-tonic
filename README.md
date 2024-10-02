# Gin-Tonic

Collection of my own utility code for making games with SDL2

## Features
 - General SDL Utils, such as for error-messages
 - Standardised logging
 - Sprite-sheet loading and displaying
 - Simple monospaced Text-rendering (using above sprite-sheets)
 - Embedded default font (no external image required)
 - Basic 8-Bit-Style audio generation tools (basic wave generation and sequencing)
 - Remappable inputs which can be saved to a settings file
 - Generic binary data storage for saving to files or sending across a network
 - Text-based generic menu system
 - Basic scene framework
 - Simple onscreen graphical interface elements (buttons, sliders)

## Planned Future Improvements
 - Embed interface: Lets you embed a datablock file and fetch
   data from it easily (`embed.h`)
 - Network interface: Provides basic net utilities for hosting
   and connecting to servers. Maybe also net-event infrastructure
   (`network.h`)

## Usage

Well it's not really intended to be used outside of my own projects,
but I suppose in case I forget:

 1. Download / Build `GinTonic.dll`
 2. Include it in the project root
 3. add `GinTonic` to the library list (Linker flag `-lGinTonic`, and maybe `-L./`)
 4. Make sure you have the header files somewhere locally,
    then add a `-I` linker flag that points to the include dir
    e.g.: `-I 'C:\gin-tonic\include'`
 5. Include whatever headers you need and build with above options
 6. Enjoy! ~~debugging your toolchain for hours...~~