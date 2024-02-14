#include "../common.h"

typedef struct {
    SDL_Thread* send;
    SDL_Thread* recieve;
    TCPsocket socket;
    SDL_mutex* client_mutex;
} client;

struct game_data_t game_data;
client clients[10];
int client_num = 0;
int terminate = 0;
SDL_mutex* terminate_mutex;
SDL_mutex* game_data_mutex;

int send_data_to_client(void* args) {
    client* c = (client*) args;
    while(1) {
        SDL_LockMutex(terminate_mutex);
        if (terminate) {
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
        SDL_UnlockMutex(terminate_mutex);
        SDL_LockMutex(game_data_mutex);
        block map[ROWS][COLS];
        memcpy(map, game_data.map, ROWS*COLS);
        SDL_UnlockMutex(game_data_mutex);
        SDL_LockMutex(c->client_mutex);
        SDLNet_TCP_Send(c->socket, &map, ROWS*COLS);
        SDL_UnlockMutex(c->client_mutex);
    }
}

int recv_data_from_client(void* args) {
    client* c = (client*) args;
    while(1) {
        SDL_LockMutex(terminate_mutex);
        if (terminate) {
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
        SDL_UnlockMutex(terminate_mutex);
        data_pack packet;
        SDL_LockMutex(c->client_mutex);
        SDLNet_TCP_Recv(c->socket, &packet, sizeof(packet));
        SDL_UnlockMutex(c->client_mutex);

        SDL_LockMutex(game_data_mutex);
        game_data.map[packet.cursor.y][packet.cursor.x].id = (short)packet.blockid;
        SDL_UnlockMutex(game_data_mutex);
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();
    SDL_CreateWindow("server!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 10, 10, SDL_WINDOW_RESIZABLE);
    IPaddress ip;
    SDLNet_ResolveHost(&ip, NULL, PORT);
    TCPsocket serv = SDLNet_TCP_Open(&ip);
    if (!serv) {
        printf("%s", SDLNet_GetError());
        return 228;
    }
    SDL_Event e;
    while (1) {
        TCPsocket c = SDLNet_TCP_Accept(serv);
        if (c) {
            client_num++;
            clients[client_num].socket = c;
            clients[client_num].client_mutex = SDL_CreateMutex();
            clients[client_num].recieve = SDL_CreateThread(recv_data_from_client, NULL, clients+client_num);
            clients[client_num].send = SDL_CreateThread(send_data_to_client, NULL, clients+client_num);
        }
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_LockMutex(terminate_mutex);
            terminate = 1;
            SDL_UnlockMutex(terminate_mutex);
            break;
        }
    }
    for (int i = 0; i < client_num; ++i) {
        SDL_WaitThread(clients[i].send, NULL);
        SDL_WaitThread(clients[i].recieve, NULL);
    }
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
