#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "../inc/buffers.h"
#include "../inc/destroyer.h"
#include "../inc/threads.h"

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
        default:
            return "Unknown thread stopped responding, closing program!";
        break;
    }
}

int checkResponseLongerThan2Seconds(int thread){
    long timeSinceLastUpdate = time(NULL) - updateBuffer[thread];
    return timeSinceLastUpdate > 2 ? 1 : 0;
}

//Go through every thread and check if any of them took longer than 2 seconds to respond than return that thread.
int checkAllThreadResponses(){
    for(int thread = 0; thread < THREAD_AMOUNT; thread++){
        if(checkResponseLongerThan2Seconds(thread)){
            return thread;
        }
    }
    return -1;
}

/*
    If stopEverything recieves a non -1 value, it means that some thread stopped responding.
    (It would be way more readable if stopEverything was just inside checkAllThreadResponses() isntead, but it needs to be it's own function to make testing checkAllThreadResponses() easier.)
*/
void stopEverything(int thread){
    if(thread != -1){
        //todo: add message to logger
        closeProgram(getMessageFromThread(thread));
    }
}

int checkLastUpdate(){
    while(threadsActive){
        usleep(500000);//Check every half a second.
        mtx_lock(&watchdogUpdateMutex);

        stopEverything(checkAllThreadResponses());

        mtx_unlock(&watchdogUpdateMutex);
        
    }
    thrd_exit(0);
}


