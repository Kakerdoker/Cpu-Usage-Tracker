#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../inc/destroyer.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/reader.h"
#include "../inc/logger.h"


//What: Gets stuck in a loop until there are no messages waiting to be logged or 5 seconds have passed
//What for: So if the program crashes we can know what happened by looking inside logs.txt
static void waitForLoggerToFinishLogging(void){
    int messageBuffEmptyValue = 1; 
    sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
    time_t timeInTenSeconds = time(NULL) + 5;

    //Be stuck in a loop until there are no messages left to log or 5 seconds have passed.
    while(messageBuffEmptyValue > 0 && timeInTenSeconds > time(NULL)){
        printf("Waiting for logger to finish logging, this won't take longer than 5 seconds.\n");
        sem_getvalue(&messageBuffEmpty, &messageBuffEmptyValue);
        sleep(1);
    }
    printf("Done.\n");
    detachLogger();
}

//What: Closes program in the correct sequence for being called from the watchdog thread
//What for: So there won't be any memory leaks and the function won't be interrupted until the end
void closeProgramByWatchdog(const char* message) {
    threadsActive = 0;
    detachThreadsExceptLogAndWatch();
    sleep(1);//Give the threads a second to finish before cleaning everything.
    waitForLoggerToFinishLogging();
    collectBufferGarbage();
    closeStatFile();
    destroyMutexes();
    closeLogger();
    
    printf("%s\n", message);
    loggerActive = 0;
    thrd_exit(0);//Detatch itself (watchdog thread)
}
