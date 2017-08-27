#ifndef logger_h
#define logger_h
#include <SdFat.h>

class Logger{
	char prefix;
	int max_num;
	unsigned long currFile;
public:
	Logger(char prefix,int max_num) : prefix(prefix), max_num(max_num), currFile(0xFFFFFFFFUL) {};
	SdFile file;
	void openFile(void);
	void flush(void);
	void log_P(PGM_P fmt, ... );
	void log_NS(char *fmt, ... );  //This version does not log to serial
	void write(byte *src, byte len); // Used for binary entries
	void write(byte character); 
};

extern Logger eventLogger;
extern Logger dataLogger;

#endif