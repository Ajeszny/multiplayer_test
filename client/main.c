
#include "../common.h"
#define LOOPBACK_IP "127.0.0.1"

struct game_data_t game_data = {0};
SDL_Texture *id_map[INT16_MAX];
SDL_Event e;
SDL_Texture *choice;
SDL_mutex* game_data_mutex;
SDL_mutex* terminate_mutex;
int terminate = 0;

int get_info_from_server(void* args) {
    TCPsocket sock = (TCPsocket) args;
    while(1) {
        SDL_LockMutex(terminate_mutex);
        if (terminate) {
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
        SDL_UnlockMutex(terminate_mutex);
        block map[ROWS][COLS];
        SDLNet_TCP_Recv(sock, map, ROWS*COLS);
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                printf("%i ", map[i][j].id);
            }
            printf("\n");
        }
        SDL_LockMutex(game_data_mutex);
        memcpy(game_data.map, map, ROWS*COLS);
        SDL_UnlockMutex(game_data_mutex);
        sleep(10);
    }
}

int get_info_to_server(void* args) {
    TCPsocket sock = (TCPsocket) args;
    while(1) {
        SDL_LockMutex(terminate_mutex);
        if (terminate) {
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
        SDL_UnlockMutex(terminate_mutex);
        data_pack data;
        SDL_LockMutex(game_data_mutex);
        data.cursor = game_data.cursor_pos;
        data.blockid = game_data.map[game_data.cursor_pos.y][game_data.cursor_pos.x].id;
        SDL_UnlockMutex(game_data_mutex);
        SDLNet_TCP_Send(sock, &data, sizeof(data));
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();
    SDL_Window *win = SDL_CreateWindow("Hello!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, BLOCK_SIZE * COLS,
                                       BLOCK_SIZE * COLS, SDL_WINDOW_RESIZABLE);
    if (!win) {
        printf("win: %s\n", SDL_GetError());
        return 228;
    }
    SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    game_data_mutex = SDL_CreateMutex();
    terminate_mutex = SDL_CreateMutex();

    choice = SDL_CreateTextureFromSurface(r, IMG_Load("Assets/choice.png"));
    SDL_Surface *s = SDL_LoadBMP("Assets/bricks.bmp");


    SDL_Texture *bricks = SDL_CreateTextureFromSurface(r, s);
    SDL_Texture *air = SDL_CreateTextureFromSurface(r, IMG_Load("Assets/air.png"));
    id_map[0] = air;
    id_map[1] = bricks;
    game_data.cursor_pos.x = 3, game_data.cursor_pos.y = 4;
    IPaddress ip;
    SDLNet_ResolveHost(&ip, LOOPBACK_IP, PORT);
    TCPsocket serv = SDLNet_TCP_Open(&ip);
    if (!serv) {
        printf("Server not running: %s", SDLNet_GetError());
        return 1337;
    }
    SDL_Thread* get_map = SDL_CreateThread(get_info_from_server, "get info from server", serv);
    SDL_Thread* set_cursor = SDL_CreateThread(get_info_to_server, "get info to server", serv);

    while(1)
    {
        //OUTPUT
        SDL_RenderClear(r);
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                SDL_Rect dst = {j * BLOCK_SIZE, i * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
                SDL_LockMutex(game_data_mutex);
                SDL_RenderCopy(r, id_map[game_data.map[i][j].id], NULL, &dst);
                SDL_UnlockMutex(game_data_mutex);
            }
        }
        SDL_LockMutex(game_data_mutex);
        SDL_Rect dst = {game_data.cursor_pos.x * BLOCK_SIZE, game_data.cursor_pos.y * BLOCK_SIZE, BLOCK_SIZE,
                        BLOCK_SIZE};
        SDL_UnlockMutex(game_data_mutex);

        SDL_RenderCopy(r, choice, NULL, &dst);
        SDL_RenderPresent(r);



        //INPUT
        SDL_PollEvent(&e);
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    SDL_LockMutex(game_data_mutex);
                    if (game_data.cursor_pos.y > 0) game_data.cursor_pos.y -= 1;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
                case SDLK_s:
                    SDL_LockMutex(game_data_mutex);
                    if (game_data.cursor_pos.y < ROWS-1) game_data.cursor_pos.y += 1;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
                case SDLK_a:
                    SDL_LockMutex(game_data_mutex);
                    if (game_data.cursor_pos.x > 0) game_data.cursor_pos.x -= 1;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
                case SDLK_d:
                    SDL_LockMutex(game_data_mutex);
                    if (game_data.cursor_pos.x < COLS-1) game_data.cursor_pos.x += 1;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
                case SDLK_SPACE:
                    SDL_LockMutex(game_data_mutex);
                    game_data.map[game_data.cursor_pos.y][game_data.cursor_pos.x].id++;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
                case SDLK_LSHIFT:
                    SDL_LockMutex(game_data_mutex);
                    if (game_data.map[game_data.cursor_pos.y][game_data.cursor_pos.x].id > 0)
                        game_data.map[game_data.cursor_pos.y][game_data.cursor_pos.x].id--;
                    SDL_UnlockMutex(game_data_mutex);
                    break;
            }
        }
        if (e.type == SDL_QUIT) {
            SDL_LockMutex(terminate_mutex);
            terminate = 1;
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
    }
    SDL_WaitThread(set_cursor, NULL);
    SDL_WaitThread(get_map, NULL);

    SDLNet_TCP_Close(serv);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
