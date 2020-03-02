#pragma once
#include <fcntl.h>
#include <string>

class Cpuinfo
{
public:
	Cpuinfo();
	Cpuinfo(std::string processorIndex, std::string vendorId,
		std::string cpuFamily, std::string Model, std::string modelName) : processor_index(processorIndex),vendor_id(vendorId),
																		cpu_family(cpuFamily),model(Model),model_name(modelName){}
public:
	void tryLoad(std::string cpuInfoLine);
	void print();
	std::string toString();
private:
	std::pair<std::string, std::string> splitCpuInfo(std::string cpuInfoLine);
public:
	std::string processor_index="";
	std::string vendor_id="";
	std::string cpu_family="";
	std::string model="";
	std::string model_name="";
};

class NetCard
{
public:
	NetCard(std::string eth0Mac) : eth0_Mac(eth0Mac) {}
public:
	std::string toString();
private:
	std::string eth0_Mac="";
};

class Sysinfo
{
public:
	Sysinfo();
	~Sysinfo();
public:
	std::string GetSysFingerPrint();
private:
	bool Load();
	bool LoadCpuInfo();
	bool LoadNetcardInfo();
	void release();
	
public:
	Cpuinfo* cpu=nullptr;
	NetCard* netcard= nullptr;
};

