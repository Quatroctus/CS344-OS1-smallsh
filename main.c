#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "command.h"

/**
 * Wait for the current foreground child process then
 *    change the foregroundOnly mode and alert the user.
**/
void HandleSIGTSTP(int sigID) {
    char FLAG[129] = "4u7x!A%D*G-KaPdSgVkYp3s6v8y/B?E(H+MbQeThWmZq4t7w!z$C&F)J@NcRfUjXn2r5u8x/A?D*G-KaPdSgVkYp3s6v9y$B&E)H+MbQeThWmZq4t7w!z%C*F-JaNcRf";
    void* walker = FLAG + 1;
    // Walk the stack until we find the first occurence of the FLAG(should be in main).
    while (memcmp(walker++, FLAG, sizeof(FLAG)) != 0);
    pid_t* parentPid = walker + 139; // The parentPid variable in main.
    if (*parentPid == getpid()) {
        int status;
        pid_t* childPid = parentPid + 1; // The childPid variable in main.
        if (*childPid != -1)
            waitpid(*childPid, &status, 0);
        const char foregroundOnly[] = "\nWe've entered forground-only mode.\n";
        const char backgroundAllowed[] = "\nWe've left foreground-only mode.\n";
        bool* foregroundOnlyPtr = walker + 150; // The foregroundOnly variable in main.
        *foregroundOnlyPtr = !(*foregroundOnlyPtr);
        if (*foregroundOnlyPtr)
            write(1, foregroundOnly, sizeof(foregroundOnly));
        else
            write(1, backgroundAllowed, sizeof(backgroundAllowed));
        size_t* commandCount = walker + 151;
        char message[32] = {0};
        sprintf(message, ": ", *commandCount);
        write(1, message, strlen(message));
    }
    fflush(stdout);
    memset(FLAG, 0, sizeof(FLAG));
}

/**
 * Set the SIGINT handler to HandleSIGINT.
 * Set the SIGTSTP handler to HandleSIGTSTP.
**/
void SetupSigHandlers(void (*HandleSIGINT)(int), void (*HandleSIGTSTP)(int)) {
    struct sigaction sigInt = {0};
    sigInt.sa_handler = HandleSIGINT;
    sigfillset(&sigInt.sa_mask);
    sigInt.sa_flags = 0;
    sigaction(SIGINT, &sigInt, NULL);

    struct sigaction sigTstp = {0};
    sigTstp.sa_handler = HandleSIGTSTP;
    sigfillset(&sigTstp.sa_mask);
    sigTstp.sa_flags = 0;
    sigaction(SIGTSTP, &sigTstp, NULL);
}

/**
 * Perform the cd command using chdir.
**/
void CommandCD(command* c) {
    if (c->args.length > 0) {
        if (chdir(((string*) c->args.items)[0].str) < 0)
            printf("No such directory %s.\n", ((string*) c->args.items)[0].str);
    } else {
        const char* homeDir = getenv("HOME");
        if (homeDir == NULL) homeDir = getpwuid(getuid())->pw_dir;
        if (chdir(homeDir) < 0)
            printf("No such directory %s.\n", homeDir);
    }
}

/**
 * Open the command::inOut strings as files if possible and
 *    use dup2() to map them to stdin and stdout and return false.
 * If not possible print error messages and return true.
**/
bool PerformIO(command* c, int* inFD, int* outFD) {
    int badIO = 0;
    if (c->inOut[0].length > 0) {
        if ((*inFD = open(c->inOut[0].str, O_RDONLY, 0760)) < 0) badIO |= 1;
        else if (dup2(*inFD, 0) < 0) badIO |= 5;
    }
    if (c->inOut[1].length > 0) {
        if ((*outFD = open(c->inOut[1].str, O_WRONLY | O_CREAT | O_TRUNC, 0760)) < 0) badIO |= 2;
        else if (dup2(*outFD, 1) < 0) badIO |= 10;
    }

    if (badIO & 1) printf(badIO & 4 ? "Could no dup2 input.\n" : "Could not open file %s for input.\n", c->inOut[0].str);
    if (badIO & 2) printf(badIO & 8 ? "Could no dup2 output.\n" : "Could not open file %s for output.\n", c->inOut[1].str);
    return badIO;
}

/**
 * Construct a char** array where the first char* is
 *    the command name and the rest are the args and the last
 *    is NULL.
**/
char** ConstructExecArgs(command* c) {
    char** args = malloc(sizeof(char*) * (c->args.length + 2));
    args[0] = c->commandName.str;
    for (int i = 0; i < c->args.length; i++)
        args[i + 1] = ((string*) c->args.items)[i].str;
    args[c->args.length + 1] = NULL;
    return args;
}

/**
 * Iterate over the background pids and if they have exited
 *    print their status and remove from vector.
**/
void CheckBGPids(vector* bgPids) {
    size_t i = 0;
    int status, rPid;
    while (i < bgPids->length) {
        rPid = waitpid(((pid_t*) bgPids->items)[i], &status, WNOHANG);
        if (rPid > 0) {
            if (WIFEXITED(status))
                printf("The process %d exited normally with status: %d.\n", ((pid_t*) bgPids->items)[i], WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                printf("The process %d was terminated with signal: %d.\n", ((pid_t*) bgPids->items)[i], WTERMSIG(status));
            fflush(stdout);
            RemoveVector(bgPids, i);
        } else i++;
    }
}

int main(int argc, char* args[]) {
    SetupSigHandlers(SIG_IGN, HandleSIGTSTP);
    command c;
    vector bgPids = ConstructVector(sizeof(pid_t), NULL, NULL);
    int status;
    bool running = true;
    char commandInput[2049];
    int commandLength = 0;
    volatile size_t commandCount = 0;
    volatile bool foregroundOnly = false;
    volatile pid_t childPid = -1, parentPid = getpid();
    // Since I was determined not to use globals I have this 129-byte string as a marker for walking up the stack.
    char FLAG[] = "4u7x!A%D*G-KaPdSgVkYp3s6v8y/B?E(H+MbQeThWmZq4t7w!z$C&F)J@NcRfUjXn2r5u8x/A?D*G-KaPdSgVkYp3s6v9y$B&E)H+MbQeThWmZq4t7w!z%C*F-JaNcRf";
    while (running) {
        printf(": ", commandCount);
        fflush(stdout);
        do {
            if (fgets(commandInput, sizeof(commandInput), stdin) == NULL)
                commandInput[0] = 0;
            commandLength = strlen(commandInput);
        } while (commandLength <= 0);
        if (commandInput[0] == '#' || commandInput[0] == '\n') continue;

        ConstructCommand(&c, commandLength, commandInput);
        PostProcessCommand(&c, parentPid);
        commandCount++;

        // Built in commands first then everything else.
        if (strcmp(commandInput, "exit") == 0) {
            running = false;
        } else if (strcmp(commandInput, "cd") == 0) {
            CommandCD(&c);
        } else if (strcmp(commandInput, "status") == 0) {
            if (WIFEXITED(status)) printf("The last foreground process exited normally with exit code %d.\n", WEXITSTATUS(status));
            else printf("The last foreground process was terminated by signal %d.\n", WTERMSIG(status));
        } else {
            pid_t tempPid = fork();
            if (tempPid == 0) {
                if (c.background && !foregroundOnly) SetupSigHandlers(SIG_IGN, SIG_IGN);
                else SetupSigHandlers(SIG_DFL, SIG_IGN);
                char** args = ConstructExecArgs(&c);
                int inFD = -1, outFD = -1;
                // IO has failed flush stdout and jump to cleanup/exit.
                if (PerformIO(&c, &inFD, &outFD)) {
                    fflush(stdout);
                    goto exit_child;
                }
                execvp(args[0], args);
                printf("No such file or directory named %s.\n", args[0]);
exit_child:
                // Clean up even though we are going to exit anyways.
                if (inFD >= 0) close(inFD);
                if (outFD >= 0) close(outFD);
                free(args);
                DestroyCommand(&c);
                DestroyVector(&bgPids);
                exit(1);
            } else if (parentPid == getpid()) {
                // Wait for the process to die if it should be run in the foreground.
                if (!c.background || foregroundOnly) {
                    childPid = tempPid;
                    waitpid(childPid, &status, 0);
                    if (WIFSIGNALED(status)) {
                        printf("\nThe foreground process %d was terminated by signal %d.\n", childPid, WTERMSIG(status));
                        fflush(stdout);
                    }
                } else { // If we are running in the background store the pid and print the pid.
                    PushBackVector(&bgPids, (void*) &tempPid);
                    printf("The background process is %d.\n", tempPid);
                    fflush(stdout);
                }
            } else {
                printf("Could not fork. Command %s will not run.\n", c.commandName);
                fflush(stdout);
            }
        }
        DestroyCommand(&c);
        CheckBGPids(&bgPids);
    }
    // Wait until background processes close.
    CheckBGPids(&bgPids);
    for (size_t i = 0; i < bgPids.length; i++)
        kill(((pid_t*) bgPids.items)[i], SIGTERM);
    DestroyVector(&bgPids);
    memset(FLAG, 0, sizeof(FLAG));
    return 0;
}
