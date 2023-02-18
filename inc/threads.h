#ifndef THREADS_H
#define THREADS_H

extern int threadsActive;

void createThreads();
void runThreads();
void detachThreadsExceptLogAndWatch();
void detachLogger();

#endif