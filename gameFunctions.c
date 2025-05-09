#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "gameFunctions.h"
#include "types.h"
//TODO check for old structs and swap them with new ones
void printBoard(char board[SIZE][SIZE], char buffer[]) {
    sprintf(buffer, " %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    sprintf(buffer + strlen(buffer), "---|---|---\n");
    sprintf(buffer + strlen(buffer), " %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    sprintf(buffer + strlen(buffer), "---|---|---\n");
    sprintf(buffer + strlen(buffer), " %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

bool checkWin(char board[SIZE][SIZE], char player) {
    
    for (int i = 0; i < SIZE; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
            return true;
        }
    }

    for (int i = 0; i < SIZE; i++) {
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) {
            return true;
        }
    }

    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
        return true;
    }
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
        return true;
    }

    return false;
}

bool gameLogic(game* match) {
    char buffer[1024];
int i = 0;

printBoard(match->board, buffer);
send(match->host->fd, buffer, strlen(buffer), 0);  
send(match->guest->fd, buffer, strlen(buffer), 0);  

while (i < 9) {
    int current = i % 2;
    int other = (i + 1) % 2;

    if (current == 0) {
        send(match->host->fd, "Your turn\n", 10, 0);  
        recv(match->host->fd, buffer, sizeof(buffer), 0);
    } else {
        send(match->guest->fd, "Your turn\n", 10, 0);  
        recv(match->guest->fd, buffer, sizeof(buffer), 0);
    }

    int move = buffer[0] - '1';  
    int x = move / 3;
    int y = move % 3;

   
    if (match->board[x][y] == ' ') {
        match->board[x][y] = (current == 0) ? 'X' : 'O';  
        printBoard(match->board, buffer);
        
        
        send(match->host->fd, buffer, strlen(buffer), 0);  
        send(match->guest->fd, buffer, strlen(buffer), 0);

       
        if (checkWin(match->board, match->board[x][y])) {
            if (current == 0) {
                send(match->host->fd, "You win\n", 8, 0);
                send(match->guest->fd, "You lost\n", 9, 0);
            } else {
                send(match->guest->fd, "You win\n", 8, 0);
                send(match->host->fd, "You lost\n", 9, 0);
            }
            break;
        }
        i++;
    } else {
        
        if (current == 0) {
            send(match->host->fd, "Invalid move\n", 14, 0);
        } else {
            send(match->guest->fd, "Invalid move\n", 14, 0);
        }
    }
}


if (i == 9) {
    send(match->host->fd, "Draw\n", 5, 0);
    send(match->guest->fd, "Draw\n", 5, 0);
    return true;
}

return false;
}


bool handleRematch(int host, int guest) {
    char buffer[1024];
    char response;

    send(host, "It's a draw! Do you want a rematch? (y/n): \n", 46, 0);
    recv(host, buffer, sizeof(buffer) - 1, 0);
    buffer[strlen(buffer) - 1] = '\0';
    response = buffer[0];

    if (response != 'y' && response != 'Y') {
        send(host, "No rematch requested. Game over.\n", 32, 0);
        send(guest, "The host declined the rematch. Game over.\n", 42, 0);
        return false;
    }

    send(guest, "It's a draw. The host wants a rematch. Do you want to play again? (y/n): \n", 75, 0);
    recv(guest, buffer, sizeof(buffer) - 1, 0);
    buffer[strlen(buffer) - 1] = '\0';
    response = buffer[0];

    if (response != 'y' && response != 'Y') {
        send(host, "The guest declined the rematch. Game over.\n", 42, 0);
        send(guest, "You declined the rematch. Game over.\n", 37, 0);
        return false;
    }

    return true;
}