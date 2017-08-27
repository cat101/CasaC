#include "config.h"
#include "debug.h"
#include "logger.h"

extern Sd2Card card;
extern SdVolume volume;
extern SdFile root;
extern uint32_t getUnixTime(void);

// Open the appropiate file for the current time interval. Rotate filenames, close current if done, do nothing if we are ok
void Logger::openFile(void){
	uint32_t currTime=getUnixTime()/3600;	
	if(currFile==currTime)
		return; //The current file is the right one
	// We need to open a new file
	// See if we need to recycle files
	dir_t p;
	int noFiles=0;
	unsigned long oldestFileNum=0xFFFFFFFFUL;
	root.rewind();
	while (root.readDir(&p) > 0) {
		// done if past last used entry
		if (p.name[0] == DIR_NAME_FREE) break;
		// skip deleted entry and entries for . and  ..
		if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;
		// Sample file name eHHHHHH where H is the number of hours since Jan 01 1970.
		if (p.name[0] != prefix) continue; //Process only files with the right prefix
		noFiles++;
		unsigned long cf=strtoul((char *)(&p.name[1]), NULL,10);
		if(cf<oldestFileNum) oldestFileNum=cf;
	}
	char name[9];
	name[0]=prefix;
	if(noFiles>=max_num){
		// Make room for a new file
		SdFile oldestFile;
		snprintf(&name[1], sizeof(name)-1, "%lu",oldestFileNum);
        DEBUG_PRINT_P("Deleting: %s\n",name);      
		oldestFile.open(&root, name, O_WRITE);
		oldestFile.remove(); // Remove oldest file
		oldestFile.close();
	}
	//Close the existing one in case we had one opened
	if(currFile!=0xFFFFFFFFUL){ 
		file.write("EOF\n"); 
		file.close();
	}	
	//Open a new file
	currFile=currTime;
	snprintf(&name[1], sizeof(name)-1, "%lu",currFile);
    DEBUG_PRINT_P("Opening: %s (noFiles=%d, oldestFileNum=%lu)\n",name,noFiles, oldestFileNum);      
	file.open(&root, name, O_CREAT | O_APPEND | O_WRITE);
}

void Logger::log_P(PGM_P fmt, ... )
{
	char entry[100];
	openFile();
	uint32_t currTime=getUnixTime();
	snprintf(entry, sizeof(entry), "%08lX:",currTime);
	va_list args;
	va_start (args, fmt );
	vsnprintf_P(&entry[9], sizeof(entry)-9, fmt, args);
	va_end (args);
	file.write(entry);
	file.write('\n');
	DEBUG_PRINT_P("log: %s\n",entry);
}
//This version does not log to serial
void Logger::log_NS(char *fmt, ... )
{
	char entry[100];
	openFile();
	uint32_t currTime=getUnixTime();
	snprintf(entry, sizeof(entry), "%08lX:",currTime);
	va_list args;
	va_start (args, fmt );
	vsnprintf(&entry[9], sizeof(entry)-9, fmt, args);
	va_end (args);
	file.write(entry);
	file.write('\n');
}
void Logger::write(byte *src, byte len)
{
	file.write(src,len);
}
void Logger::write(byte src)
{
	file.write(src);
}
void Logger::flush(void){
	if(currFile==0xFFFFFFFFUL) 
		return;
	file.close();
	currFile=0xFFFFFFFFUL;
}
