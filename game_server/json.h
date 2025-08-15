#ifndef JSON_H
#define JSON_H

#include "types.h"

char * message_serialize(message_t *msg);
message_t * message_parse(char *json_string);
void message_free(message_t *msg);
message_t * message_create(int status, char *payload_name, void *payload_value);

#endif