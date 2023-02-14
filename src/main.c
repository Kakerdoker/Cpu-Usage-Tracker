#include <stdlib.h>
#include <stdio.h>

#include "global.h"
#include "buffers.h"
#include "threads.h"

void initialize(){
    initializeGlobalVariables();
    initializePrinterVariables();
    initializeMutexesAndSemaphores();

    allocateBufferMemory();
}

void destroy(){
    destroyMutexesAndSemaphores();
    collectBufferGarbage();
}

int main(){

    initialize();

    createThreads();

    destroy();

}

//todo: what next: write tests



