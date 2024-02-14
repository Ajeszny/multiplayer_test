#include <stdio.h>
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_net.h>
#include <unistd.h>
#define ROWS 20
#define COLS 20
#define BLOCK_SIZE 20
#define PORT 228

typedef struct {
    SDL_Point pos;
    int16_t id;
    int status;//0 - absent, 1 - placed, 2 - selected

}block;

struct game_data_t {
    SDL_Point cursor_pos;
    block map[ROWS][COLS];
};

typedef struct {
    SDL_Point cursor;
    int blockid;
} data_pack;