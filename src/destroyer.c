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
    printf("Done.\n");
    logClose();
    detachLogger();
}

//What: Closes program in the correct sequence for being called from the watchdog thread
//What for: So it can be closed properly without any memory leaks or any other issues
void closeProgramByWatchdog(const char* message) {
    detachThreadsExceptLogAndWatch();
    sleep(1);//Give the threads a second to finish before cleaning everything.
    waitForLoggerToFinishLogging();
    collectGarbage();
    
    printf("%s\n", message);
    detachWatchdog();
    //thrd_exit(0);//Detatch itself (watchdog thread)
}

static char msg[64];
static void logReceivedSignal(const int signum){
    switch(signum){
        case 2:
            strcpy(msg,"Signal received: SIGINT");
        break;
        case 3:
            strcpy(msg,"Signal received: SIGQUIT");
        break;
        case 15:
            strcpy(msg,"Signal received: SIGTERM");
        break;
    }

    logMessage(msg);
}

static sig_atomic_t sigLock = 1;
//What: Closes program in the correct sequence for being called from a termination signal
//What for: So it can be closed properly without any memory leaks or any other issues
static void closeProgramBySignal(const int signum){
    if(sigLock){
        sigLock = 0;

        logReceivedSignal(signum);
        detachThreadsExceptLogger();
        sleep(1);//Give the threads a second to finish before cleaning everything.
        waitForLoggerToFinishLogging();
        collectGarbage();
        exit(0);
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
