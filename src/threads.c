#include <threads.h>

#include "../inc/logger.h"
#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"

thrd_t readFileThread, analyzeThread, printThread, watchdogThread, loggerThread;
int threadsActive = 1;

void createThreads(){
    if(thrd_create(&loggerThread, waitForNewMessagesToLog, NULL) != thrd_success)
        printf("Couldn't create logger thread (threads.c).");

    if(thrd_create(&readFileThread, getProcStatInfo, NULL) != thrd_success)
        logMessage("Couldn't create reader thread (threads.c).");

    if(thrd_create(&analyzeThread, analyzeCpuInfo, NULL) != thrd_success)
        logMessage("Couldn't create analyzer thread (threads.c).");

    if(thrd_create(&printThread, printCpuUsagePercantages, NULL) != thrd_success)
        logMessage("Couldn't create printer thread (threads.c).");

    if(thrd_create(&watchdogThread, checkLastUpdate, NULL) != thrd_success)
        logMessage("Couldn't create watchdog thread (threads.c).");
}

void runThreads(){
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

void detachThreadsExceptLogAndWatch(){
    if(thrd_detach(readFileThread) != thrd_success)
        logMessage("Couldn't detach reader thread (threads.c)");

    if(thrd_detach(analyzeThread) != thrd_success)
        logMessage("Couldn't detach analyzer thread (threads.c)");

    if(thrd_detach(printThread) != thrd_success)
        logMessage("Couldn't detach printer thread (threads.c)");
}

void detachThreadsExceptLogger(){
    detachThreadsExceptLogAndWatch();
    
    if(thrd_detach(watchdogThread) != thrd_success)
        logMessage("Couldn't detach watchdog thread (threads.c)");
}

void detachLogger(){
    if(thrd_detach(loggerThread) != thrd_success)
        printf("Couldn't detach logger thread (threads.c)");
}
