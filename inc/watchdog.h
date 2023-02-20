#ifndef WATCHDOG_H
#define WATCHDOG_H

//Checks if all threads are responsive every (args) microseconds
int checkLastUpdate(void*);

/*
    Function declarations used only for tests
*/
//Gets the appropriate message depending on which thread failed
char* getMessageFromThread(const int);

//Checks if logger is being responsive
int checkLoggerThread(void);

//Checks if all the threads are being responsive
int checkAllThreadResponses(void);

#endif
