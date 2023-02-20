#ifndef LOGGER_H
#define LOGGER_H

//Write message to message buffer
void logMessage(const char*);

//Create dir and file for logs
void initializeLogger(const char*,const char*);

//Logs the time the program has closed
void closeLogger(void);

//Calls logMessageFromBuffer() every (args) microseconds
int loggerLoop(void*);

#endif
