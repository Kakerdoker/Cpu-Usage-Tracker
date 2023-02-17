#include <threads.h>

#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"

thrd_t readFileThread, analyzeThread, printThread, watchdogThread;
int threadsActive = 1;

void createThreads(){
    thrd_create(&readFileThread, getProcStatInfo, NULL);
    thrd_create(&analyzeThread, analyzeCpuInfo, NULL);
    thrd_create(&printThread, printCpuUsagePercantages, NULL);
    thrd_create(&watchdogThread, checkLastUpdate, NULL);
}

void runThreads(){
    thrd_join(readFileThread, NULL);
    thrd_join(analyzeThread, NULL);
    thrd_join(printThread, NULL);
    thrd_join(watchdogThread, NULL);    
}

void detachThreads(){
    thrd_detach(readFileThread);
    thrd_detach(analyzeThread);
    thrd_detach(printThread);
    //Watchdog detaches itself at the end of closeProgram();
}