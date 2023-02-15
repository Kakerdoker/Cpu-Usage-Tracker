#ifndef BUFFERS_H
#define BUFFERS_H

#include <threads.h>

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

extern mtx_t cpuInfoMutex, cpuUsageMutex;

extern struct CpuInfo* currentCpuInfoBuffer;
extern struct CpuInfo* previousCpuInfoBuffer;
extern float* cpuUsageBuffer; 

void initializeMutexes();
void destroyMutexes();

void allocateBufferMemory();
void collectBufferGarbage();

#endif