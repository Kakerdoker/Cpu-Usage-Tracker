#include <unistd.h>

#include "global.h"
#include "buffers.h"

long unsigned calculateIdle(struct CpuInfo* cpuInfo, int index){
    return cpuInfo[index].idle + cpuInfo[index].iowait;
}

long unsigned calculateNonIdle(struct CpuInfo* cpuInfo, int index){
    return cpuInfo[index].user + cpuInfo[index].nice + cpuInfo[index].system + cpuInfo[index].irq + cpuInfo[index].softirq + cpuInfo[index].steal;
}

float calculateCpuUsage(int index){

    long unsigned prevIdle = calculateIdle(previousCpuInfoBuffer,index);
    long unsigned currIdle = calculateIdle(currentCpuInfoBuffer,index);

    long unsigned prevNonIdle = calculateNonIdle(previousCpuInfoBuffer,index);
    long unsigned currNonIdle = calculateNonIdle(currentCpuInfoBuffer,index);
    
    long unsigned prevTotal = prevIdle + prevNonIdle;
    long unsigned currTotal = currIdle + currNonIdle;

    long totalDiff = currTotal - prevTotal;
    long idleDiff = currIdle - prevIdle;

    float CpuPercantage = 0;
    //Make sure not to divide by zero;
    if(totalDiff != 0){
        CpuPercantage = ((float)(totalDiff-idleDiff)/(float)totalDiff)*100;
    }

    return CpuPercantage;
}

void addCpuUsageToBuffer(){
    //Repeat for every core
    for(int core = 0 ; core < cpuCoreAmount; core++){
        cpuUsageBuffer[core] = calculateCpuUsage(core);
    }
}

void* analyzeCpuInfo(){
    while(1){
        sleep(1);
        mtx_lock(&cpuInfoMutex); //Lock the cpu information buffer for reader.c

        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for printer.c

        addCpuUsageToBuffer();

        mtx_unlock(&cpuUsageMutex); //Unlock cpu usage buffer
        
        mtx_unlock(&cpuInfoMutex);//Unlock cpu information buffer
    }
    return NULL;
}