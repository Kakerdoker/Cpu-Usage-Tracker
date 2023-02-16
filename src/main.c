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

void destroy(){
    destroyMutexes();
    collectBufferGarbage();
}

int main(){

    initialize();

    runThreads();

    destroy();
    
}

//todo: what next: implementacja watchdog



