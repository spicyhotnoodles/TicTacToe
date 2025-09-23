#include "hash.h"

/*

    Notes regarding glib lib

    1. g_hash_table_new_full() allocates memory which needs to be freed
    2. gpointer is a void* pointer
    3. g_direct_hash is a hash function for integer keys
    4. g_direct_equal is a key equality function (keys are integers, use default)
    5. GHashTableIter is an iterator for GHashTable

*/


/// @brief Checks if a username already exists in the player table.
/// @param username The username to check for existence.
/// @return true if the username exists, false otherwise.
bool username_exists(const char* username) {
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, players);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        struct player *p = (struct player *)value;
        if (strcmp(p->username, username) == 0) {
            return true;
        }
    }
    return false;
}

/// @brief Generates a random 5-digit number (between 10000 and 99999) based on a given integer seed. It initializes the random number generator using the current time offset by the seed value to ensure variability.
/// @param seed The seed value for random number generation
/// @return A random 5-digit number
long random_id(int seed) {
    srand(time(0) + seed);
    return ((rand() % 900000) + 100000);
}