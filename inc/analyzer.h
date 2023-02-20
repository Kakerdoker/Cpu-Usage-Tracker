#ifndef ANALYZER_H
#define ANALYZER_H

//Calls addCpuUsageToBuffer(void) every (args) microseconds
int analyzerLoop(void*);

/*
    Function declarations used only for tests
*/
//Calculates the average cpu usage for every core
void addCpuUsageToBuffer(void);

#endif
