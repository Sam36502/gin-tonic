#
#	Gin-Tonic Makefile
#

VERSION = 0.1.0
BIN = GinTonic

CC = gcc
LD = ld
CFLAGS = -Wall -g
LFLAGS = -Wall -g -shared
REL_CFLAGS = -Wall -O2 -shared -mwindows

LIBS = mingw32 SDL2main SDL2_image SDL2_net SDL2

SRC = src
INC = include
OBJ = objects
EMB = embeds
TST = test
SRCS = $(wildcard $(SRC)/*.c) 
HDRS = $(wildcard $(INC)/*.h) 
EMBS = $(wildcard $(EMB)/*)
OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS)) $(patsubst $(EMB)/%,$(OBJ)/%.o,$(EMBS))

list:
	@echo -e '\n### Objects to Build: ###'
	@printf '%s\n' $(notdir $(OBJS))

test: build
	@echo -e '\n### Building & Running Test ###\n'
	-rm -f $(TST)/main.o $(TST)/test.exe
	${CC} ${CFLAGS} -o $(TST)/main.o -c $(TST)/main.c -L./ -lGinTonic $(addprefix -l,$(LIBS))
	${CC} ${CFLAGS} -o $(TST)/test.exe $(TST)/main.o -L./ -lGinTonic $(addprefix -l,$(LIBS))

build: $(OBJS)
	@echo -e '\n### Linking... ###'
	${CC} ${LFLAGS} -o ${BIN}.dll $(OBJS) $(addprefix -l,$(LIBS))

release: 
	@echo -e '### Creating Release... ###\n'
	${CC} ${REL_CFLAGS} -o ${BIN}_v${VERSION}.dll $(OBJS) $(addprefix -l,${LIBS})

clean:
	@echo -e '### Cleaning... ###\n'
	-rm -f $(BIN).dll
	-rm -rf $(OBJ)
	@mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.c $(INC)/%.h
	@echo -e 'Building $@...'
	@${CC} ${CFLAGS} -c $< -o $@

$(OBJ)/%.o: $(EMB)/%
	@echo -e 'Embedding $@...'
	@${LD} -r -b binary $< -o $@

$(OBJ):
	#echo -e 'Creating object directory...'
	@mkdir -p $(OBJ)

.PHONY: test run build release clean
