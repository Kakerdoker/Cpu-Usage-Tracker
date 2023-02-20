#include <threads.h>
#include <stdio.h>

#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/logger.h"
#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"

static thrd_t readFileThread, analyzeThread, printThread, watchdogThread, loggerThread;
int threadsActive = 1;
int loggerActive = 1;

static unsigned int delayArr[THREAD_AMOUNT+1];

//What: Creates a thread for a function that's supposed to loop every n microseconds
//What for: So looping threads can be easily created
static void createThreadLoop(thrd_t* thread, thrd_start_t function, char* failedMessage, unsigned int* delay, unsigned int microseconds){
    *delay = microseconds;
    if(thrd_create(thread, function, delay) != thrd_success)
        logMessage(failedMessage);
}

//What: Creates a looping thread specifically for the logger loop
//What for: So the failedMessage can be printed instead of logged
static void createLoggerThreadLoop(unsigned int* delay, unsigned int dl){
    *delay = dl;
    if(thrd_create(&loggerThread, loggerLoop, delay) != thrd_success)
        printf("Couldn't create logger thread (threads.c).");
}

//What: Creates every thread used by the program
//What for: So it can be called from the main function
void createThreads(void){
    createLoggerThreadLoop(&delayArr[0], 500000);
    createThreadLoop(&readFileThread, readerLoop, "Couldn't create analyzer thread (threads.c).", &delayArr[1], 1000000);
    createThreadLoop(&analyzeThread, analyzerLoop, "Couldn't create analyzer thread (threads.c).", &delayArr[2], 1000000);
    createThreadLoop(&printThread, printCpuUsagePercantages, "Couldn't create printer thread (threads.c).", &delayArr[3], 1000000);
    createThreadLoop(&watchdogThread, checkLastUpdate, "Couldn't create watchdog thread (threads.c).", &delayArr[4], 500000);
}

void runThreads(void){
    if(thrd_join(loggerThread, NULL) != thrd_success)
        printf("Couldn't join logger thread (threads.c)");  

    if(thrd_join(readFileThread, NULL) != thrd_success)
        logMessage("Couldn't join reader thread (threads.c)");

    if(thrd_join(analyzeThread, NULL) != thrd_success)
        logMessage("Couldn't join analyzer thread (threads.c)");

    if(thrd_join(printThread, NULL) != thrd_success)
        logMessage("Couldn't join printer thread (threads.c)");

    if(thrd_join(watchdogThread, NULL) != thrd_success)
        logMessage("Couldn't join watchdog thread (threads.c)");   
}


//What for: So when the watchdog thread detached all the threads it doesn't detach itself
void detachThreadsExceptLogAndWatch(void){
    if(thrd_detach(readFileThread) != thrd_success)
        logMessage("Couldn't detach reader thread (threads.c)");

    if(thrd_detach(analyzeThread) != thrd_success)
        logMessage("Couldn't detach analyzer thread (threads.c)");

    if(thrd_detach(printThread) != thrd_success)
        logMessage("Couldn't detach printer thread (threads.c)");
}

//What for: So when the threads are being detached the logger still has time to log
void detachThreadsExceptLogger(void){
    detachThreadsExceptLogAndWatch();
    
    if(thrd_detach(watchdogThread) != thrd_success)
        logMessage("Couldn't detach watchdog thread (threads.c)");
}

void detachLogger(void){
    if(thrd_detach(loggerThread) != thrd_success)
        printf("Couldn't detach logger thread (threads.c)");
}
