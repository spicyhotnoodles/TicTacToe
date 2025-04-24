#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include "ticTacToeFunctions.h"

#define SIZE 3
#define PORT 6969

void* gameThread(void* arguement) {
    Match* match = (Match*)arguement;
    bool rematchRequested;

    do {
        
        memset(match->board, ' ', sizeof(match->board));

        
        int winner = gameLogic(match);

        if (winner != -1) {
            int loser = (winner == 0) ? 1 : 0;
            rematchRequested = handleRematch(match->playerfd[winner], match->playerfd[loser]);
        } else {
            
            rematchRequested = false;
        }

    } while (rematchRequested);

    
    close(match->playerfd[0]);
    close(match->playerfd[1]);
    free(match);
    return NULL;
}
int main() {
    int serverSock;
    struct sockaddr_in server, client;
    socklen_t clientLen = sizeof(client);

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(serverSock, (struct sockaddr*)&server, sizeof(server));
    listen(serverSock, 10);
    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1) {
        Match* match = malloc(sizeof(Match));
        memset(match->board, ' ', sizeof(match->board));

        match->playerfd[0] = accept(serverSock, (struct sockaddr*)&client, &clientLen);
        printf("Player 1 connesso\n");

        match->playerfd[1] = accept(serverSock, (struct sockaddr*)&client, &clientLen);
        printf("Player 2 connesso\n");

        pthread_t thread;
        pthread_create(&thread, NULL, gameThread, match);
        pthread_detach(thread);
    }

    close(serverSock);
    return 0;
}