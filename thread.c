#include "thread.h"
#include "gameFunctions.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void* gameThread(void* arguement) {
    game* match = (game*)arguement;
    bool rematchRequested;

    do {
        
        memset(match->board, ' ', sizeof(match->board));

        
        bool draw = gameLogic(match);

        if (draw == true) {
            rematchRequested = handleRematch(match->host->fd, match->guest->fd);
        } else {
            
            rematchRequested = false;
        }

    } while (rematchRequested);

    
    /*close(match->playerfd[0]);
    close(match->playerfd[1]);
    free(match);*/
    return NULL;
}