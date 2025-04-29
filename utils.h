#ifndef utils_h
#define utils_h
#include "types.h"

void createGame(struct game *games, int *ngames, struct player host);

char* prepareListOfGamesForClient(struct game *games, int ngames);
#endif