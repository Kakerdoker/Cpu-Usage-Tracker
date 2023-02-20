#include <unistd.h>

#include "../inc/analyzer.h"
#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"

static long unsigned calculateIdle(const struct CpuInfo* cpuInfo, const int core){
    return cpuInfo[core].idle + cpuInfo[core].iowait;
}

static long unsigned calculateNonIdle(const struct CpuInfo* cpuInfo, const int core){
    return cpuInfo[core].user + cpuInfo[core].nice + cpuInfo[core].system + cpuInfo[core].irq + cpuInfo[core].softirq + cpuInfo[core].steal;
}

//What: Calculates the percentage of cpu usage for a given core.
static double calculateCpuUsage(const int core){

    long unsigned prevIdle = calculateIdle(previousCpuInfoBuffer,core);
    long unsigned currIdle = calculateIdle(currentCpuInfoBuffer,core);

    long unsigned prevNonIdle = calculateNonIdle(previousCpuInfoBuffer,core);
    long unsigned currNonIdle = calculateNonIdle(currentCpuInfoBuffer,core);
    
    long unsigned prevTotal = prevIdle + prevNonIdle;
    long unsigned currTotal = currIdle + currNonIdle;

    long unsigned totalDiff = currTotal - prevTotal;
    long unsigned idleDiff = currIdle - prevIdle;

    double CpuPercantage = 0;

    if(totalDiff != 0){
        CpuPercantage = ((double)(totalDiff-idleDiff)/(double)totalDiff)*100;
    }

    return CpuPercantage;
}

//What: Goes through every cpu core and get's its average usage
//What for: So it can be used in printer.c to show the user
void addCpuUsageToBuffer(void){
    for(unsigned int core = 0 ; core < cpuCoreAmount; core++){
        cpuUsageBuffer[core] = calculateCpuUsage((int)core);
    }
}

//What: Calls addCpuUsageToBuffer() every (args) microseconds using mutexes to lock readerLoop and printerLoop
//What for: So it can be synchronized with readerLoop and printerLoop so the data won't change when it's being written/read
int analyzerLoop(void* args){
    unsigned int delay = *(unsigned int *)args;

    while(threadsActive){
        updateWatchdogBuffer(1);
        usleep(delay);

        mtx_lock(&cpuInfoMutex); //Lock the cpu information buffer for reader.c

        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for printer.c

        addCpuUsageToBuffer();

        mtx_unlock(&cpuUsageMutex); 
        
        mtx_unlock(&cpuInfoMutex);
    }
    thrd_exit(0);
}
