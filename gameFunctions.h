#ifndef gameFunctions_h
#define gameFunctions_h
#define SIZE 3
#include <stdbool.h>
#include "types.h"


void printBoard(char board[SIZE][SIZE], char buffer[]);


bool checkWin(char board[SIZE][SIZE], char player);

bool gameLogic(game* match);

bool handleRematch(int winnerFD, int loserFD);

#endif 