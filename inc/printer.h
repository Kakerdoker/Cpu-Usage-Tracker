#ifndef PRINTER_H
#define PRINTER_H

//Calls printCpuUsageAverages() every (args) microseconds
int printCpuUsagePercantages(void*);

/*
    Function declarations used only for tests
*/
//Creates a percentage bar for the given core
char* makePercentageBar(const unsigned int);

#endif
