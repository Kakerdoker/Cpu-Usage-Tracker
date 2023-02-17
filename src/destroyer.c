#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



void closeProgram(char* message){
    threadsActive = 0;
    detachThreads();
    sleep(2);//Give the threads 2 seconds to finish before cleaning everything.
    printf("\n%s\n", message);
    collectBufferGarbage();
    closeStatFile();
    destroyMutexes();
    
    thrd_exit(0);//Detatch itself (watchdog thread);
}