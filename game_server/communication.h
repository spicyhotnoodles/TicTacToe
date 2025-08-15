#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "types.h"
#include "hash.h"
#include "json.h"
#include "game.h"

// Function prototypes for communication handling
void handle_request(int fd, message_t *message);
bool send_message(int fd, message_t *message);
message_t * receive_message(int fd);

#endif