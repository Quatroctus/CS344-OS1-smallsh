#include <stdbool.h>

#include "string/str.h"
#include "vector/vector.h"

typedef struct command {
    string commandName;
    vector args;
    string inOut[2];
    bool background;
} command;

command* ConstructCommand(command* c, size_t length, char* const command);
void PostProcessCommand(command* c, pid_t pid);
void PrintCommand(command* command);
void DestroyCommand(command* command);