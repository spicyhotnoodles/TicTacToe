#include "json.h"

/*

    Notes regarding cJSON lib

    1. cJSON_CreateObject() allocates memory which needs to be freed
    using the cJSON_Delete() function

    2. cJSON_Print() allocates memory which neeeds to be freed using
    the free() function

*/

/// @brief Takes a pointer to a `message_t` structure and serializes its contents into a JSON-formatted string using the cJSON library.
/// @param msg Pointer to the message that needs to be serialized
/// @return The serialized string on success or `NULL` if an error occurs during the serialization process.
char *message_serialize(message_t *msg) {
    if (!msg) return NULL;
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        fprintf(stderr, "Error: cJSON_CreateObject failed.\n");
        return NULL;
    }
    cJSON_AddNumberToObject(root, "status", msg->status_code);
    if (msg->payload) {
        // duplicate payload so we don't take ownership of caller's payload
        cJSON *payload_dup = cJSON_Duplicate(msg->payload, 1);
        if (!payload_dup) {
            fprintf(stderr, "Error: cJSON_Duplicate failed.\n");
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToObject(root, "payload", payload_dup);
    }
    char *output = cJSON_Print(root); // allocates string
    if (!output) {
        fprintf(stderr, "Error: cJSON_Print failed.\n");
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_Delete(root);
    return output; // caller must free()
}


/// @brief Takes a JSON string as input and parses it into a dynamically allocated `message_t` structure, extracting the "method" and "payload" fields from the JSON object. The caller is responsible for freeing the allocated memory using `message_free()` after use.
/// @param json_string The JSON-formated string
/// @return A `message_t` dynamic allocated object if the parsing was successfull, `NULL` otherwise 
message_t *message_parse(char *json_string) {
    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
        return NULL;
    }
    message_t *output = calloc(1, sizeof(*output));
    if (!output) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON *method_item = cJSON_GetObjectItem(root, "method");
    if (method_item && cJSON_IsString(method_item) && method_item->valuestring) {
        output->method = strdup(method_item->valuestring);
    } else {
        output->method = NULL;
    }
    cJSON *payload_item = cJSON_GetObjectItem(root, "payload");
    if (payload_item) {
        output->payload = cJSON_Duplicate(payload_item, 1); // deep copy
    } else {
        output->payload = NULL;
    }
    cJSON_Delete(root);
    return output;
}


/// @brief Dynamically allocates and initializes a `message_t` structure with a given status code and an optional payload.
/// @param status The status code for the message
/// @param payload_name The name of the payload item
/// @param payload_value The value of the payload item
/// @return A pointer to the newly created `message_t` structure, or NULL on failure
message_t * message_create(int status, char *payload_name, void *payload_value) {
    message_t *msg = calloc(1, sizeof(message_t)); // Caller needs to free() after use!
    if (!msg) {
        fprintf(stderr, "Error: Memory allocation for message_t failed.\n");
        return NULL;
    }
    msg->status_code = status;
    if (payload_name && payload_value) {
        msg->payload = cJSON_CreateObject();
        if (!msg->payload) {
            fprintf(stderr, "Error: cJSON_CreateObject failed.\n");
            free(msg);
            return NULL;
        }
        cJSON_AddItemToObject(msg->payload, payload_name, payload_value);
    } else {
        msg->payload = NULL;
    }
    return msg;
}

/// @brief Frees the memory allocated for a message_t structure.
/// @param msg Pointer to the message to be freed
void message_free(message_t *msg) {
    if (!msg) return;
    free(msg->method);
    if (msg->payload) cJSON_Delete(msg->payload);
    free(msg);
}
