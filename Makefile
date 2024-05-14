#
#	Gin-Tonic Makefile
#

VERSION = 0.1.0
BIN = GinTonic

CC = gcc
CFLAGS = -Wall -g
LFLAGS = -Wall -g -shared
REL_CFLAGS = -Wall -O2 -mwindows

LIBS = mingw32 SDL2main SDL2_net SDL2

SRC = src
INC = include
OBJ = objects
SRCS = $(wildcard $(SRC)/*.c) 
HDRS = $(wildcard $(INC)/*.h) 
OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

test: build
	@echo -e '\n### Building & Running Test ###\n'
	-rm main.o test.exe
	${CC} ${CFLAGS} -o main.o -c main.c -L./ -lGinTonic $(addprefix -l,$(LIBS))
	${CC} ${CFLAGS} -o test.exe main.o -L./ -lGinTonic $(addprefix -l,$(LIBS))
	-./test.exe

build: $(OBJS)
	@echo -e '\n### Linking... ###'
	${CC} ${LFLAGS} -o ${BIN}.dll $(OBJS) $(addprefix -l,$(LIBS))

release: 
	@echo -e '### Creating Release... ###\n'
	${CC} ${REL_CFLAGS} -o ${BIN}_v${VERSION}.dll main.c src/* $(addprefix -l,${LIBS})

clean:
	@echo -e '### Cleaning... ###\n'
	-rm $(BIN).dll
	-rm -r $(OBJ)
	mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.c $(INC)/%.h
	@echo -e 'Building $@...'
	@${CC} ${CFLAGS} -c $< -o $@

.PHONY: test run build release clean
