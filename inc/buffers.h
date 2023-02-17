#ifndef BUFFERS_H
#define BUFFERS_H

#include <threads.h>

#define THREAD_AMOUNT 3

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

extern mtx_t cpuInfoMutex, cpuUsageMutex, watchdogUpdateMutex;

extern struct CpuInfo* currentCpuInfoBuffer;
extern struct CpuInfo* previousCpuInfoBuffer;
extern float* cpuUsageBuffer; 
extern time_t updateBuffer[THREAD_AMOUNT];

void updateWatchdogBuffer();

void initializeMutexes();
void destroyMutexes();

void allocateBufferMemory();
void collectBufferGarbage();

#endif