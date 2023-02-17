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

int checkAllThreadResponses(){
    //Go through the last update of each thread
    for(int thread = 0; thread < THREAD_AMOUNT; thread++){
        if(checkResponseLongerThan2Seconds(thread)){
            //todo: add message to logger
            closeProgram(getMessageFromThread(thread));
            return 1;
        }
    }
    return 0;
}

int checkLastUpdate(){
    while(threadsActive){
        usleep(500000);//Check every half a second.
        mtx_lock(&watchdogUpdateMutex);

        checkAllThreadResponses();

        mtx_unlock(&watchdogUpdateMutex);
        
    }
    thrd_exit(0);
}


