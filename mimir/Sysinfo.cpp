#include "Sysinfo.h"
#include <fstream>
#include <vector>

#include "./utils/mUtil.h"

Sysinfo::Sysinfo()
{
    bool succ=Load();
    if (!succ)
    {
        release();
        throw new std::exception();
    }
}

Sysinfo::~Sysinfo()
{
    release();
}

std::string Sysinfo::GetSysFingerPrint()
{
    if (cpu==nullptr||netcard==nullptr)
    {
        return "";
    }
    return MUtil::md5String(cpu->toString() + "||" + netcard->toString());
}

bool Sysinfo::Load()
{
	bool cpuLoadSucc = LoadCpuInfo();
	bool netLoadSucc = LoadNetcardInfo();
	return cpuLoadSucc&&netLoadSucc;
}

bool Sysinfo::LoadCpuInfo()
{
    try
    {
        cpu = new Cpuinfo();
        std::fstream File;
        File.open("/proc/cpuinfo", std::ios::in);
        std::vector<std::string> vecErrorCode;
        //logger.debug("LoadCpuInfo",LHEADER);
        while (!File.eof())
        {
            std::string strErrCode = "";
            std::getline(File, strErrCode);
            //logger.debug(strErrCode);
            cpu->tryLoad(strErrCode);
        }
        File.close();
        //logger.debug("", LFOOTER);
        return true;
    }
    catch (const std::exception& e)
    {
        logger.crash(std::string("LoadCPUInfo:") + e.what(), "SysInfo");
        return false;
    }
}

bool Sysinfo::LoadNetcardInfo()
{
    try
    {
        //std::fstream File;
        //File.open("/sys/class/net/eth0/address",std::ios::in);
        std::vector<std::string> vecErrorCode;
        //logger.debug("LoadNetcardInfo", LHEADER);
        //while (!File.eof())
        //{
        //    std::string strErrCode = "";
        //    std::getline(File, strErrCode);
        //    //logger.debug("NetCardInfo:" + strErrCode);
        //    vecErrorCode.push_back(strErrCode);
        //}
        //File.close();
        //logger.debug("", LFOOTER);
        char* cmd = "lshw -c network | grep serial | head -n 1";
        char output[256] = { 0 };
        MUtil::get_system_output(cmd, output, 256);
        std::string netcardInfo = std::string(output);
        //logger.log("Netcard Info:" + netcardInfo);
        netcard=new NetCard(std::string(output));
        return true;
    }
    catch (const std::exception& e)
    {
        logger.crash(std::string("LoadNetcardInfo:") + e.what(), "SysInfo");
        return false;
    }
   
}

void Sysinfo::release()
{
    if (cpu != nullptr)
    {
        delete cpu;
    }
    if (netcard != nullptr)
    {
        delete netcard;
    }
}

Cpuinfo::Cpuinfo()
{
}

void Cpuinfo::tryLoad(std::string cpuInfoLine)
{
    std::pair<std::string, std::string> propPair = splitCpuInfo(cpuInfoLine);
    if (propPair.first=="processor"&&processor_index.empty()== true)
    {
        processor_index = propPair.second;
        return;
    }
    if (propPair.first == "vendor_id" && vendor_id.empty() == true)
    {
        vendor_id = propPair.second;
    }
    if (propPair.first == "cpu family" && cpu_family.empty() == true)
    {
        cpu_family = propPair.second;
    }
    if (propPair.first == "model" && model.empty() == true)
    {
        model = propPair.second;
    }
    if (propPair.first == "model name" && model_name.empty() == true)
    {
        model_name = propPair.second;
    }
    //logger.debug("PROP:" + propPair.first);
    //logger.debug("VALUE:" + propPair.second);
    return;
}

void Cpuinfo::print()
{
    logger.log("processor_index:" + processor_index);
    logger.log("vendor_id:" + vendor_id);
    logger.log("cpu_family:" + cpu_family);
    logger.log("model:" + model);
    logger.log("model_name:" + model_name);
}

std::string Cpuinfo::toString()
{
    std::string result = processor_index + vendor_id + cpu_family + model + model_name;
    return result;
}

std::pair<std::string, std::string> Cpuinfo::splitCpuInfo(std::string cpuInfoLine)
{
    if (cpuInfoLine=="")
    {
        return std::pair<std::string, std::string>("","");
    }
    std::string propName = "";
    std::string propValue = "";
    std::size_t pos = cpuInfoLine.find(":");
    if (pos ==std::string::npos)
    {
        return std::pair<std::string, std::string>("", "");
    }
    propName = MUtil::trimRight(cpuInfoLine.substr(0, pos));
    propValue = MUtil::trimLeft(cpuInfoLine.substr(pos + 1));
    return std::pair<std::string, std::string>(propName, propValue);
}

std::string NetCard::toString()
{
    return eth0_Mac;
}
