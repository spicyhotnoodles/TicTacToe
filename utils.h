#ifndef utils_h
#define utils_h
#include "types.h"
#include <stdbool.h>

void createGame( game *games, int *ngames, struct player *host);

char* prepareListOfGamesForClient(game *games, int ngames);

bool declineOrAcceptGuest( game *games, int *ngames, int match_id, struct player *guest);

void joinGame( game *games, int *ngames, int match_id, struct player *guest);

int getGameIndexById(game *games, int ngames, int match_id);

// void hostLeavesGame(struct game *games, int *ngames , int match_id);

// void guestLeavesGame(struct game *games, int *ngames , int match_id);

// void setGameStatus(struct game *games, int *ngames , int match_id, gameStatus status);

// void setGameResult(struct game *games, int *ngames , int match_id, result res);


#endif