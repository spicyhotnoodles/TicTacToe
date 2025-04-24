#ifndef ticTacToeFunctions_H
#define ticTacToeFunctions_H

#define SIZE 3


typedef struct {
    int playerfd[2];
    char board[SIZE][SIZE];
} Match;


void printBoard(char board[SIZE][SIZE], char buffer[]);


bool checkWin(char board[SIZE][SIZE], char player);

int gameLogic(Match* match);

bool handleRematch(int winnerFD, int loserFD);

#endif 