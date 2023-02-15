#include <stdlib.h>
#include <stdio.h>

#include "global.h"
#include "buffers.h"
#include "threads.h"

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

//todo: what next: write tests



