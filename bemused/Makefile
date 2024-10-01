#
#	Bemused Makefile
#

VERSION = 0.1.0
BIN = Bemused

CC = gcc
LD = ld
CFLAGS = -Wall -g -I '../include' -L./
REL_CFLAGS = -Wall -O2 -mwindows

TEST_MUSIC = music/metronome.dbf
TEST_FLAGS = -l
TEST_MIDI = midi/Solstice_Intro.mid

LIBS = mingw32 SDL2main SDL2_image SDL2_net SDL2

run: build $(TEST_MUSIC) $(TEST_MIDI)
	./${BIN}.exe $(TEST_FLAGS) $(TEST_MUSIC)

build:
	@echo -e '\n### Building... ###'
	${CC} ${CFLAGS} -o ${BIN}.exe src/bemused.c -lGinTonic $(addprefix -l,$(LIBS))

release: 
	@echo -e '### Creating Release... ###\n'
	${CC} ${REL_CFLAGS} -o ${BIN}_v${VERSION}.exe src/bemused.c $(addprefix -l,${LIBS})

clean:
	@echo -e '### Cleaning... ###\n'
	-rm $(BIN).dll

.PHONY: run build release clean
