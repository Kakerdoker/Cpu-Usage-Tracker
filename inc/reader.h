#ifndef READER_H
#define READER_H

int getProcStatInfo();
void openStatFile(char*);
void readStatFileAndPutIntoBuffer();
void copyCpuInfoBuffer();

#endif