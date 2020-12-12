#LDFLAGS = -lSDL2 -lSDL2_image -lrt -lasound -ljack -lpthread -lportaudio -lm
LDFLAGS = -lSDL2 -lSDL2_image -lGL -ldl
INCLUDES = -I/usr/include/SDL2 -Iinc/
CFLAGS = -Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -g -O3
SRCS = $(wildcard *.c)

nxplore: $(SRCS)
	clang $(SRCS) -o  nxplore $(CFLAGS) $(INCLUDES) $(LDFLAGS)

.PHONY: clean
clean: