#include <threads.h>

#include "reader.h"
#include "analyzer.h"
#include "printer.h"

thrd_t readFileThread, analyzeThread, printThread;

void createThreads(){
    thrd_create(&readFileThread, getProcStatInfo, NULL);
    thrd_create(&analyzeThread, analyzeCpuInfo, NULL);
    thrd_create(&printThread, printCpuUsagePercantages, NULL);
}

void runThreads(){
    thrd_join(readFileThread, NULL);
    thrd_join(analyzeThread, NULL);
    thrd_join(printThread, NULL);
}