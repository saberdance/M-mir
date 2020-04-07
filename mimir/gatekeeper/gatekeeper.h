#pragma once
#include <string>
#include <vector>
#include "KeyMaker.h"
#include "FileCryptor.h"

class GatekeeperResult
{
public:
	GatekeeperResult();
	GatekeeperResult(int in_code, std::string in_command,std::string in_reason);
public:
	int code = 0;
	std::string command = "none";
	std::string reason = "none";
};

class Gatekeeper
{
public:
	void showHelp();
	GatekeeperResult Run(std::vector<std::string> args);
private:
	GatekeeperResult MakeSetupFile(std::vector<std::string> args);
	GatekeeperResult EncryptFile(std::vector<std::string> args);	
	GatekeeperResult RunFolder(std::vector<std::string> args);	
	GatekeeperResult GenRSAKeyPair(std::vector<std::string> args);
	GatekeeperResult GenUserEncryptKey(std::vector<std::string> args);	
	GatekeeperResult RSAEncodeFEK(std::vector<std::string> args);
	std::vector<std::string> LoadTargetConfigFile();
private:
	GatekeeperResult Raw_EncryptFile(std::string sourceFile, std::string aeskeyFile, 
		std::string publickeyFile, std::string privatekeyFile,std::string outputFolder="");
	GatekeeperResult Raw_RunFolder(std::string targetFolder);
	GatekeeperResult Raw_GenRSAKeyPair(std::string outDir);
	GatekeeperResult Raw_GenUserEncryptKey(std::string hdCode, std::string authKey, std::string keyFile);
private:
	KeyMaker keyMaker;
};

