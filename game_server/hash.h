#ifndef HASH_H
#define HASH_H

#include "types.h"

/// @brief Checks if a username already exists in the player table.
/// @param username The username to check for existence.
/// @return true if the username exists, false otherwise.
bool username_exists(const char* username);

/// @brief Generates a random 5-digit number (between 10000 and 99999) based on a given integer seed. It initializes the random number generator using the current time offset by the seed value to ensure variability.
/// @param seed The seed value for random number generation
/// @return A random 5-digit number
long random_id(int seed);

#endif