#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "../inc/buffers.h"
#include "../inc/destroyer.h"
#include "../inc/threads.h"
#include "../inc/logger.h"

int messageBuffEmptyValue;

char* getMessageFromThread(int thread){
    switch (thread){
        case 0:
            return "Reader thread stopped responding, closing program!";
        break;
        case 1:
            return "Analyzer thread stopped responding, closing program!";
        break;
        case 2:
            return "Printer thread stopped responding, closing program!";
        break;
        case 3:
            return "Logger thread stopped responding, closing program!";
        break;
        default:
            return "Unknown thread stopped responding, closing program!";
        break;
    }
}

int checkResponseLongerThan2Seconds(int thread){
    long timeSinceLastUpdate = time(NULL) - updateBuffer[thread];
    return timeSinceLastUpdate > 2 ? 1 : 0;
}

int checkThreadsWithoutLogger(){
    for(int thread = 0; thread < THREAD_AMOUNT-1; thread++){
        if(checkResponseLongerThan2Seconds(thread)){
            return thread;
        }
    }
    return -1;
}

int checkLoggerThread(){
    //Check if message buffer is not empty, if it is empty, that means the logger can't update it's response, so don't check for it.
    sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
    if(messageBuffEmptyValue > 0){
        if(checkResponseLongerThan2Seconds(THREAD_AMOUNT-1)){
            return THREAD_AMOUNT-1;
        }
    }
    return -1;
}

//Go through every thread and check if any of them took longer than 2 seconds to respond then return that thread.
int checkAllThreadResponses(){
    int threadResponse;
    threadResponse = checkThreadsWithoutLogger();
    if(threadResponse != -1){
        return threadResponse;
    }
    threadResponse = checkLoggerThread();
    return threadResponse;
}

/*
    If stopEverything recieves a non -1 value, it means that some thread stopped responding.
    (It would be way more readable if stopEverything was just inside checkAllThreadResponses() instead, but it needs to be it's own function to make testing checkAllThreadResponses() easier.)
*/
void stopEverything(int thread){
    if(thread != -1){
        mtx_unlock(&watchdogUpdateMutex);
        logMessage(getMessageFromThread(thread));
        closeProgram(getMessageFromThread(thread));
    }
}

int checkLastUpdate(){
    while(threadsActive){
        usleep(500000);//Check every half a second.
        mtx_lock(&watchdogUpdateMutex);//Lock the watchdog mutex so updateWatchdogBuffer() doesn't update when checking buffers.

        stopEverything(checkAllThreadResponses());

        mtx_unlock(&watchdogUpdateMutex);
        
    }
    thrd_exit(0);
}


