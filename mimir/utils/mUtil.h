#pragma once
#include <string>
#include <mutex>
#include <vector>
#include "md5.h"

#define MLOCK(mtx) std::lock_guard<std::mutex> mlockGuard(mtx)

enum MLOGLV
{
	QUIET,
	DEBUG,
	RELEASE,
	DEBUG_WITH_CONSOLE,
	RELEASE_WITH_CONSOLE
};

enum LOGTYPE
{
	LNORMAL,
	LHEADER,
	LFOOTER
};

class MLogger
{
public:
	MLogger();
	MLogger(std::string logfile);
public:
	void log(std::string msg, LOGTYPE ltype=LOGTYPE::LNORMAL);
	void output(std::string status, float progress, std::string lasterror="none");
	void error(std::string msg, LOGTYPE ltype = LOGTYPE::LNORMAL);
	void debug(std::string msg, LOGTYPE ltype = LOGTYPE::LNORMAL);
	void crash(std::string msg,std::string module="");
	void crash(std::vector<std::string>msgs, std::string module = "");
	void setLogLevel(MLOGLV level);
private:
	void writeLog(std::string msg);
	void writePureLog(std::string msg);
	std::string genHeaderStr(std::string msg);
	void crash2Console(std::string msg, std::string module = "");
	void crash2Console(std::vector<std::string>msgs, std::string module = "");
	std::mutex logMtx;
	std::string logFilePath="";
#ifdef _DEBUG
	int logLevel = MLOGLV::DEBUG_WITH_CONSOLE;
#else
	int logLevel = MLOGLV::RELEASE;
#endif
};

class MUtil
{
public:
	static std::string getCurrentDate();
	static std::string getCurrentDateTime();
	static std::string getCurrentTime();
	static std::string trimLeft(std::string str);
	static std::string trimRight(std::string str);
	static std::string md5String(std::string str);
	static std::string md5File(std::string filePath);
	static std::vector<std::string> getFileLines(std::string filePath);
};

static MLogger logger;

