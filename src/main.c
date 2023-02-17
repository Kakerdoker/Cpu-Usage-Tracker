#include <stdlib.h>
#include <stdio.h>

#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"

void initialize(){
    initializeGlobalVariables();
    initializeMutexes();
    allocateBufferMemory();
    createThreads();
}

int main(){

    initialize();

    runThreads();
    
}

//todo: what next: test watchdog



