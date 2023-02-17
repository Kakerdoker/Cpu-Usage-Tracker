#ifndef READER_H
#define READER_H


void closeStatFile();
int getProcStatInfo();
void openStatFile(char*);
void readStatFileAndPutIntoBuffer();
void copyCpuInfoBuffer();

#endif