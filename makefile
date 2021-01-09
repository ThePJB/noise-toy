LDFLAGS = -lSDL2 -lSDL2_image -lGL -ldl -lm

INCLUDES = -I/usr/include/SDL2 -Iinc/ -I. -Iggl/

CFLAGS = -Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -Wno-missing-braces -g -O3
SRCS = $(wildcard *.c)
SRCS += $(wildcard ggl/*.c)

nxplore: $(SRCS)
	clang $(SRCS) -o nxplore $(CFLAGS) $(INCLUDES) $(LDFLAGS)

.PHONY: clean
clean:
