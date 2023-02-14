#include <threads.h>

#include "reader.h"
#include "analyzer.h"
#include "printer.h"



void createThreads(){
    thrd_t readFileThread, analyzeThread, printThread;

    thrd_create(&readFileThread, getProcStatInfo, NULL);
    thrd_create(&analyzeThread, analyzeCpuInfo, NULL);
    thrd_create(&printThread, printCpuUsagePercantages, NULL);

    thrd_join(readFileThread, NULL);
    thrd_join(analyzeThread, NULL);
    thrd_join(printThread, NULL);
}