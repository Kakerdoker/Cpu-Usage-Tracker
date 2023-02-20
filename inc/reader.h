#ifndef READER_H
#define READER_H

//Closes statFile
void closeStatFile(void);

//Calls readProcStat() every (args) microseconds
int readerLoop(void *);

/*
    Function declarations used only for tests
*/
//Opens statFile
void openStatFile(const char*);

//Copies all the values from currentCpuInfoBuffer into previousCpuInfoBuffer
void copyCpuInfoBuffer(void);

//Reads only the important values from the currently opened file
void readStatFileAndPutIntoBuffer(void);

#endif
