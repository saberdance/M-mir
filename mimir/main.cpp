#include <iostream>
#include "./utils/mUtil.h"
#include "Sysinfo.h"

int main()
{
	logger.log("Mimir Start");
	Sysinfo sysinfo;
	logger.log("System FingerPrint:"+sysinfo.GetSysFingerPrint());
}

//643aead147dc959a5b847d196c563a87
