#include "mUtil.h"
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string.h>
namespace fs = std::experimental::filesystem::v1;


MLogger::MLogger()
{
    char* home = getenv("MIMIR_HOME");
    std::string logFolder = "";
    if (home!=NULL)
    {
        logFolder = std::string(home) + "/log";
    }
    else
    {
        logFolder = "./log";
    }
    logFilePath = logFolder + "/" + MUtil::getCurrentDate() + ".log";
    //perpare log env
    fs::path folder(logFolder);
    fs::path logfile(logFilePath);
    if (!fs::exists(folder))
    {
        fs::create_directories(folder);
    }
    if (!fs::exists(logfile))
    {
        std::ofstream fileStream;
        fileStream.open(logFilePath,std::ios::app);
        fileStream.close();
    }
}

MLogger::MLogger(std::string logfile)
{
    if (!fs::exists(logfile))
    {
        fs::path logPath(logfile);
        fs::path folder = logPath.parent_path();
        fs::create_directories(folder);
        std::ofstream fileStream;
        fileStream.open(logfile, std::ios::app);
        fileStream.close();
        logFilePath = logfile;
    }
}

void MLogger::log(std::string msg, LOGTYPE ltype)
{
    if (ltype==LHEADER||ltype==LFOOTER)
    {
        msg = genHeaderStr(msg);
    }
    writeLog("[LOG]" + msg);
}

void MLogger::error(std::string msg,LOGTYPE ltype)
{
    if (ltype == LHEADER || ltype == LFOOTER)
    {
        msg = genHeaderStr(msg);
    }
    writeLog("[ERR]" + msg);
}

void MLogger::debug(std::string msg, LOGTYPE ltype)
{
    if (ltype == LHEADER || ltype == LFOOTER)
    {
        msg = genHeaderStr(msg);
    }
    if (logLevel==MLOGLV::DEBUG|| logLevel == MLOGLV::DEBUG_WITH_CONSOLE)
    {
        writeLog("[DBG]" + msg);
    }
}

void MLogger::crash(std::string msg, std::string module)
{
    writeLog("[CRASH]-----------------" + module + "-----------------");
    writeLog("[CRASH]" + msg);
    writeLog("[CRASH]----------------------------------------");
}

void MLogger::crash(std::vector<std::string> msgs, std::string module)
{
    writeLog("[CRASH]-----------------" + module + "-----------------");
    for (size_t i = 0; i < msgs.size(); i++)
    {
        writeLog("[CRASH]" + msgs[i]);
    }
    writeLog("[CRASH]----------------------------------------");
}

void MLogger::setLogLevel(MLOGLV level)
{
    logLevel = level;
}

void MLogger::writeLog(std::string msg)
{
    try
    {
        MLOCK(logMtx);
        std::string curTime = MUtil::getCurrentTime();
        msg = curTime + msg;
        if (logLevel == MLOGLV::DEBUG_WITH_CONSOLE || logLevel == MLOGLV::RELEASE_WITH_CONSOLE)
        {
            std::cout << msg << std::endl;
        }
        std::ofstream fileStream;
        fileStream.open(logFilePath, std::ios::app);
        fileStream << msg << std::endl;
        fileStream.flush();
        fileStream.close();
    }
    catch (std::logic_error& e)
    {
        if (logLevel == MLOGLV::DEBUG_WITH_CONSOLE || logLevel == MLOGLV::RELEASE_WITH_CONSOLE)
        {
            std::vector<std::string> crashLog;
            crashLog.push_back("Cur Msg:" + msg);
            crash2Console(e.what(), "LOGGER");
        }
    }
}

std::string MLogger::genHeaderStr(std::string msg)
{
    if (msg == "")
    {
        msg = "------";
    }
    msg = "----------------" + msg + "-----------------";
    return msg;
}

void MLogger::crash2Console(std::string msg, std::string module)
{
    std::cout << "[CRASH]-----------------" + module + "-----------------" << std::endl;
    std::cout << msg << std::endl;
    std::cout << "[CRASH]----------------------------------------" << std::endl;
}

void MLogger::crash2Console(std::vector<std::string> msgs, std::string module)
{
    std::cout << "[CRASH]-----------------" + module + "-----------------" << std::endl;
    for (size_t i = 0; i < msgs.size(); i++)
    {
        std::cout << msgs[i] << std::endl;
    }
    std::cout << "[CRASH]----------------------------------------" << std::endl;
}

std::string MUtil::getCurrentDate()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d",
        (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday);
    return std::string(date);
}

std::string MUtil::getCurrentDateTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d-%02d:%02d:%02d",
        (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

std::string MUtil::getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = { 0 };
    sprintf(date, "%02d:%02d:%02d",
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

std::string MUtil::trimLeft(std::string str)
{
    if (str.empty())
    {
        return "";
    }
    for (size_t i =0 ; i < str.length(); i++)
    {
        if (str[i]!=' '&&str[i]!='\t')
        {
            return i == 0 ? str : str.substr(i);
        }
    }
    return str;
}

std::string MUtil::trimRight(std::string str)
{
    if (str.empty())
    {
        return "";
    }
    for (size_t i = str.length()-1; i >= 0; i--)
    {
        if (str[i] != ' ' && str[i] != '\t')
        {
            return i == 0 ? str : str.substr(0,i+1);
        }
    }
    return str;
}

std::string MUtil::md5File(std::string fileName) {
    if (!fs::exists(fileName))
    {
        return "";
    }
    ifstream f;
    f.open(fileName, std::ios::binary);
    MD5 m(f);
    f.close();
    return m.toString();
}

std::string MUtil::md5String(std::string input) {
    MD5 m(input);
    return m.toString();
}
