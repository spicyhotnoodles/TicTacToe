#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"
#include <string.h>

void createGame(struct game *games, int *ngames, struct player host) {
    games[*ngames].match_id = rand();       
    games[*ngames].host = host;             
    games[*ngames].guest = NULL;            
    (*ngames)++;                            
    printf("DEBUG: Game created successfully. Total games: %d\n", *ngames);
}

char* prepareListOfGamesForClient(struct game *games, int ngames) {
    char *buffer = malloc(4096);
    if (!buffer) {
        perror("Malloc fallita");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0';

    for (int i = 0; i < ngames; i++) {
        sprintf(buffer + strlen(buffer), "Game ID: %d | Host: %s | Guest: %s\n",
                games[i].match_id,
                games[i].host.username,
                games[i].guest ? games[i].guest->username : "None");
    }

    return buffer;
}