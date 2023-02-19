#include <stdlib.h>
#include <stdio.h>

#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/logger.h"

void initialize(){
    initializeGlobalVariables();
    initializeMutexes();
    initializeSemaphores();
    initializeLogger("logs","logs.txt");
    allocateBufferMemory();
    createThreads();
}

int main(){

    initialize();

    runThreads();
    
}

//todo: what next: test watchdog



