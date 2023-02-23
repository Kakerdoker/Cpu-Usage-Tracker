#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include "../inc/destroyer.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/reader.h"
#include "../inc/logger.h"


static void collectGarbage(){
    collectBufferGarbage();
    closeStatFile();
    destroyMutexes();
    destroySemaphores();
}

//What for: So the logger can exit without being stuck on a semaphore and without causing any memory leaks
static void exitLogger(void){
    loggerActive = 0;
    sem_post(&messageBuffEmpty);
}

//What: Gets stuck in a loop until there are no messages waiting to be logged or 5 seconds have passed
//What for: So if the program crashes we can know what happened by looking inside logs.txt
static void waitForLoggerToFinishLogging(void){
    int messageBuffEmptyValue = 1; 
    sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
    time_t timeInTenSeconds = time(NULL) + 5;

    //Be stuck in a loop until there are no messages left to log or 5 seconds have passed.
    while(messageBuffEmptyValue > 0 && timeInTenSeconds > time(NULL)){
        printf("Waiting for logger to finish logging, this won't take longer than 5 seconds.\n");
        sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
        sleep(1);
    }
    logClose();
    exitLogger();
}

//What: Closes program in the correct sequence for being called from the watchdog thread
//What for: So it can be closed properly without any memory leaks or any other issues
void closeProgramByWatchdog(const char* message) {
    threadsActive = 0;
    sleep(1);//Give the threads a second to finish before cleaning everything.
    waitForLoggerToFinishLogging();
    collectGarbage();
    
    printf("%s\n", message);
}

static char msg[64];
static void logReceivedSignal(const int signum){
    switch(signum){
        case SIGINT:
            strcpy(msg,"Signal received: SIGINT");
        break;
        case SIGQUIT:
            strcpy(msg,"Signal received: SIGQUIT");
        break;
        case SIGTERM:
            strcpy(msg,"Signal received: SIGTERM");
        break;
    }

    logMessage(msg);
}

//What: Closes program in the correct sequence
//What for: So it can be closed properly without any memory leaks or any other issues
static void closeProgram(void){
    threadsActive = 0;
    sleep(1);//Give the threads a second to finish before cleaning everything.
    waitForLoggerToFinishLogging();
    collectGarbage();
}

void closeProgramByError(char* errorMessage){
    printf("\n%s\n", errorMessage);
    logMessage(errorMessage);
    closeProgram();
}

static sig_atomic_t sigLock = 1;
static void closeProgramBySignal(const int signum){
    if(sigLock){//Make sure with sigLock that it can happen only once
        sigLock = 0;
        logReceivedSignal(signum);
        closeProgram();
    }
}

//What: Initializes sigaction to catch termination signals
//What for: So the program can close properly
void initializeSigaction(void){
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = closeProgramBySignal;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
}
