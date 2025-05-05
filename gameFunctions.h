#ifndef gameFunctions_H
#define gameFunctions_H
#define SIZE 3
#include "types.h"


void printBoard(char board[SIZE][SIZE], char buffer[]);


bool checkWin(char board[SIZE][SIZE], char player);

int gameLogic(Match* match);

bool handleRematch(int winnerFD, int loserFD);

#endif 