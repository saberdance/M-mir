#include <iostream>
#include "./utils/mUtil.h"
#include "Sysinfo.h"

int main()
{
	//logger.log("Mimir Start");
	Sysinfo sysinfo;
	std::string ret = sysinfo.GetSysFingerPrint();
	//logger.log("System FingerPrint:"+ ret);
	if (ret=="")
	{
		std::cout << "ERROR" << std::endl;
	}
	else
	{
		std::cout << ret << std::endl;
	}
	
}

//643aead147dc959a5b847d196c563a87
