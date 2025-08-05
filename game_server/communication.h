#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "types.h"
#include "hash.h"
#include "game.h"

// Function prototypes for communication handling
bool send_response(int fd, char *message, enum StatusCode status_code);
void handle_request(int fd, enum Requests request);

bool send_data(void *data, size_t size, int fd);

#endif