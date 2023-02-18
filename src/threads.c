#include <threads.h>

#include "../inc/logger.h"
#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"

thrd_t readFileThread, analyzeThread, printThread, watchdogThread, loggerThread;
int threadsActive = 1;

void createThreads(){
    thrd_create(&loggerThread, waitForNewMessagesToLog, NULL);
    logMessage("Created logger thread");
    thrd_create(&readFileThread, getProcStatInfo, NULL);
    logMessage("Created reader thread");
    thrd_create(&analyzeThread, analyzeCpuInfo, NULL);
    logMessage("Created analyzer thread");
    thrd_create(&printThread, printCpuUsagePercantages, NULL);
    logMessage("Created printer thread");
    thrd_create(&watchdogThread, checkLastUpdate, NULL);
    logMessage("Created watchdog thread");
}

void runThreads(){
    thrd_join(readFileThread, NULL);
    thrd_join(analyzeThread, NULL);
    thrd_join(printThread, NULL);
    thrd_join(watchdogThread, NULL);   
    thrd_join(loggerThread, NULL); 
}

void detachThreadsExceptLogAndWatch(){
    thrd_detach(readFileThread);
    thrd_detach(analyzeThread);
    thrd_detach(printThread);
}

void detachLogger(){
    thrd_detach(loggerThread);
}