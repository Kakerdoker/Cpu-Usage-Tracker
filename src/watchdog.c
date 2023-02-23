#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "../inc/watchdog.h"
#include "../inc/buffers.h"
#include "../inc/destroyer.h"
#include "../inc/threads.h"
#include "../inc/logger.h"

static int messageBuffEmptyValue;

//What: Gets the appropriate message depending on which thread failed
//What for: So it can be logged
char* getMessageFromThread(const int thread){
    switch (thread){
        case 0:
            return "Reader thread stopped responding, closing program!";
        case 1:
            return "Analyzer thread stopped responding, closing program!";
        case 2:
            return "Printer thread stopped responding, closing program!";
        case 3:
            return "Logger thread stopped responding, closing program!";
        default:
            return "Unknown thread stopped responding, closing program!";
    }
}

//What: Checks if the given thread hasn't responded for longer than 2 seconds
//What for: So the program can be closed if it didn't
static int checkResponseLongerThan2Seconds(const int thread){
    long timeSinceLastUpdate = time(NULL) - updateBuffer[thread];
    return timeSinceLastUpdate > 2 ? 1 : 0;
}

//What: Checks if every thread except for logger is responsive 
static int checkThreadsWithoutLogger(void){
    for(int thread = 0; thread < THREAD_AMOUNT-1; thread++){
        if(checkResponseLongerThan2Seconds(thread)){
            return thread;
        }
    }
    return -1;
}

//What: Checks if logger is being responsive, but only if it has anything in it's buffer
//What for: So the logger won't be seen as unresponsive if it is waiting for messages
int checkLoggerThread(void){
    //Check if message buffer is not empty, if it is empty, that means that logger can't update it's response, so don't check for it.
    sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
    if(messageBuffEmptyValue > 0){
        if(checkResponseLongerThan2Seconds(THREAD_AMOUNT-1)){
            return THREAD_AMOUNT-1;
        }
    }
    return -1;
}

//What: Goes through every thread and returns the first one that is being unresponsive.
//What for: So the program can be closed if any of them are.
int checkAllThreadResponses(void){
    int threadResponse;
    threadResponse = checkThreadsWithoutLogger();
    if(threadResponse != -1){
        return threadResponse;
    }
    threadResponse = checkLoggerThread();
    return threadResponse;
}


//What: Checks if the value is not -1 and stops the program if it isn't
//What for: Because if it isn't -1, that means one of the threads is not responding
static void stopEverything(const int thread){
    if(thread != -1){
        mtx_unlock(&watchdogUpdateMutex);
        logMessage(getMessageFromThread(thread));
        closeProgramByWatchdog(getMessageFromThread(thread));
    }
}

//What: Calls stopEverything() every (args) microseconds and uses mutexes to lock every other thread from updating themselves
//What for: So the data won't change when it's being read by the watchdog
int checkLastUpdate(void* args){
    unsigned int delay = *(unsigned int *)args;

    while(threadsActive){
        usleep(delay);
        mtx_lock(&watchdogUpdateMutex);//Lock the watchdog mutex so updateWatchdogBuffer() doesn't update when checking buffers.

        stopEverything(checkAllThreadResponses());

        mtx_unlock(&watchdogUpdateMutex);
        
    }
    thrd_exit(0);
}


