#pragma once

char *slurp(const char *path);
#define len(X) (sizeof(X) / sizeof(X[0]))
uint64_t get_us(); 
