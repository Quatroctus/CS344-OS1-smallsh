#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize the command struct by parsing the commandStr.
**/
command* ConstructCommand(command* c, size_t length, char* const commandStr) {
    if (c == NULL) c = malloc(sizeof(command));
    char* token;
    char* savePtr = NULL;
    // The first strtok_r result will be the commandName/filename.
    token = strtok_r(commandStr, " \n", &savePtr);
    ConstructStr(&c->commandName, token);
    c->args = ConstructVector(sizeof(string), CopyConstructStr, (void (*)(void*)) DestroyStr);
    // If the last three characters in commandStr are " &\n" the command wants to run in the background.
    c->background = strcmp(commandStr + length - 3, " &\n") == 0;
    // If we are in the background default redirection to /dev/null.
    ConstructStr(&c->inOut[0], c->background ? "/dev/null" : "");
    ConstructStr(&c->inOut[1], c->background ? "/dev/null" : "");
    // If we are in the background remove the last 3 characters from the commandStr.
    if (c->background) commandStr[length-2] = 0;
    while ((token = strtok_r(NULL, " \n", &savePtr))) {
        switch (token[0]) {
            case '<': // Input redirection change command::inOut[0].
                SetCStr(&c->inOut[0], (const char*) strtok_r(NULL, " \n", &savePtr));
                break;
            case '>': // Output redirection change command::inOut[1].
                SetCStr(&c->inOut[1], (const char*) strtok_r(NULL, " \n", &savePtr));
                break;
            default:
                {
                    // Push the arg into the command::args vector.
                    string s;
                    ConstructStr(&s, token);
                    if (s.length > 0) PushBackVector(&c->args, &s);
                }
                break;
        }
    }
    return c;
}

/**
 * Replace "$$" with the pidStr.
**/
void PidReplace(string* s, char* pidStr) {
    size_t pidLength = strlen(pidStr);
    for (int i = 0; i < s->length; i++) {
        if (GetStrChar(s, i) == '$' && GetStrChar(s, i + 1) == '$') {
            // Create a temp copy of the right substring.
            string temp;
            ConstructStr(&temp, "");
            SubString(&temp, s, i + 2, s->length);
            // Reduce the length of s to i.
            s->length = i;
            // Append the pid and the right substring.
            AppendCStr(s, pidStr);
            AppendString(s, &temp);
            DestroyStr(&temp);
            i += pidLength;
        }
    }
}

/**
 * Replace "$$" in all command strings(commandName, args..., inOut[0], inOut[1]).
**/
void PostProcessCommand(command* c, pid_t pid) {
    char pidStr[12];
    sprintf(pidStr, "%d", pid);
    PidReplace(&c->commandName, pidStr);
    for (int i = 0; i < c->args.length; i++) {
        PidReplace(&((string*) c->args.items)[i], pidStr);
    }
    PidReplace(&c->inOut[0], pidStr);
    PidReplace(&c->inOut[1], pidStr);
}

/**
 * Destroy all command members.
**/
void DestroyCommand(command* command) {
    DestroyStr(&command->commandName);
    DestroyVector(&command->args);
    DestroyStr(&command->inOut[0]);
    DestroyStr(&command->inOut[1]);
}
