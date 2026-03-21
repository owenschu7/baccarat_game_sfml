#pragma once
#include <iostream>
#include <cstdio>

// --- GLOBAL DEBUG SETTINGS ---
// Change this to 0 to mute all DEBUG_PRINT statements across the entire game
#define DEBUGMODE 1

#if DEBUGMODE
  #define DEBUG_PRINT std::cout
  #define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT if (false) std::cout 
  #define DEBUG_PRINTF(...) do {} while(0)
#endif
