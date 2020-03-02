#include <iostream>
#include <vector>
#include <string>
#include "../utils/mUtil.h"
#include "gatekeeper.h"

//gatekeeper -gen keyFileFolder
int main(int argc, char* argv[])
{
	logger.log("GateKeeper Start",LHEADER);
	Gatekeeper gatekeeper;
	if (argc<2)
	{
		gatekeeper.showHelp();
		exit(0);
	}
	std::vector<std::string> args;
	//Ìø¹ýexe±¾Éí
	for (size_t i = 1; i < argc; i++)
	{
		args.push_back(std::string(argv[i]));
	}
	GatekeeperResult ret=gatekeeper.Run(args);
	logger.log("GateKeeper End",LFOOTER);
}
