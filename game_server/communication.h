#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "types.h"
#include "hash.h"

// Function prototypes for communication handling
bool send_response(int fd, char *message, enum StatusCode status_code);
void handle_request(int fd, enum Requests request);

#endif