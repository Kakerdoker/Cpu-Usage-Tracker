#ifndef THREADS_H
#define THREADS_H

//1 if all threads (except logger) are supposed to be looping, 0 if they are not
extern int threadsActive;
//1 if logger is supposed to be looping, 0 if not
extern int loggerActive;

//Creates every thread used by the program
void createThreads(void);

//Joins every thread being used by the program
void runThreads(void);

//Detaches every thread except logger
void detachThreadsExceptLogger(void);

//Detaches every thread except logget and watchdog
void detachThreadsExceptLogAndWatch(void);

//Detaches the logger thread
void detachLogger(void);

//Detaches the watchdog thread and closes program
void detachWatchdog(void) __attribute__ ((noreturn));

#endif
