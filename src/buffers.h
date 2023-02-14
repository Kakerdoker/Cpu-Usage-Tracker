#ifndef BUFFERS_H
#define BUFFERS_H

#include <threads.h>
#include <semaphore.h>

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

extern mtx_t cpuInfoMutex, cpuUsageMutex;
extern sem_t semUsageEmpty, semUsageFull;

extern struct CpuInfo* currentCpuInfoBuffer;
extern struct CpuInfo* previousCpuInfoBuffer;

extern float** cpuUsageBuffer; 
extern const unsigned int cpuUsageBufferSize;
extern int cpuUsageBufferIndex;

void initializeMutexesAndSemaphores();
void destroyMutexesAndSemaphores();

void allocateBufferMemory();
void collectBufferGarbage();

#endif