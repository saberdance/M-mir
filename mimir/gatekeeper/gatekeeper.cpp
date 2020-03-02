#include "gatekeeper.h"
#include "../utils/mUtil.h"
#include <experimental/filesystem>
#include <unistd.h>
#include <sys/syscall.h>
#include <random>
#include <fcntl.h>
#include <linux/memfd.h>
#include <errno.h>

extern char** environ;

namespace fs = std::experimental::filesystem::v1;

void Gatekeeper::showHelp()
{
	logger.log("Gatekeeper Help",LHEADER);
	logger.log("There is nothing now");
	logger.log("", LFOOTER);
}

//gatekeeper -r/-run ./myfolder
GatekeeperResult Gatekeeper::Run(std::vector<std::string> args)
{
	if (args[0]=="-g"||args[0]=="-gen")
	{
		logger.log("Run:GenRSAKeyPair",LHEADER);
		GatekeeperResult ret= GenRSAKeyPair(args);
		logger.log("GenRSAKeyPair Result:" + ret.reason);
		logger.log("", LFOOTER);
		return ret;
	}
	if (args[0] == "-a" || args[0] == "-aes")
	{
		logger.log("Run:GenFileEncryptKey", LHEADER);
		GatekeeperResult ret = GenUserEncryptKey(args);
		logger.log("GenFileEncryptKey Result:" + ret.reason);
		logger.log("", LFOOTER);
		return ret;
	}
	if (args[0]=="-e"||args[0]=="-encrypt")
	{
		logger.log("Run:EncryptFile", LHEADER);
		GatekeeperResult ret = EncryptFile(args);
		logger.log("EncryptFile Result:" + ret.reason);
		logger.log("", LFOOTER);
		return ret;
	}
	if (args[0] == "-r" || args[0] == "-run")
	{
		logger.log("Run:RunFolder", LHEADER);
		GatekeeperResult ret = RunFolder(args);
		logger.log("RunFolder Result:" + ret.reason);
		logger.log("", LFOOTER);
		return ret;
	}
}

//gatekeeper -e/-encrypt ./myfile ./mykeys/fileAES.key ./mykeys/publickey.pem ./mykeys/privatekey/.pem
//gatekeeper -e ./myfile ./mykeys(use defalut key names as:"fileEncrypt.key publickey.pem privatekey.pem" in this folder)
GatekeeperResult Gatekeeper::EncryptFile(std::vector<std::string> args)
{
	if (args.size()!=5&&args.size()!=3)
	{
		return GatekeeperResult(-1, "EncryptFile", "Bad Arguments");
	}
	std::string sourceFile = args[1];
	std::string aeskeyFile = "";
	std::string publickeyFile = "";
	std::string privatekeyFile = "";
	if (args.size() == 3)
	{
		aeskeyFile = args[2] + "/fileEncrypt.key";
		publickeyFile = args[2] + "/publickey.pem";
		privatekeyFile = args[2] + "/privatekey.pem";
	}
	else
	{
		aeskeyFile = args[2];
		publickeyFile = args[3];
		privatekeyFile = args[4];
	}
	EncryptPack pack(sourceFile, aeskeyFile, publickeyFile, privatekeyFile);
	if (!pack.ValidateFiles())
	{
		return GatekeeperResult(-1, "EncryptFile", "File not exist");
	}
	FileCryptor cryptor(pack);
	bool succ=cryptor.EncryptFile();
	if (succ==false)
	{
		return GatekeeperResult(-1, "EncryptFile", "Encrypt Failed");
	}
	return GatekeeperResult(0, "EncryptFile", "File Encrypted:"+fs::path(sourceFile).parent_path().string()+"/"+fs::path(sourceFile).filename().string());
}

GatekeeperResult Gatekeeper::RunFolder(std::vector<std::string> args)
{
	if (args.size() != 2)
	{
		return GatekeeperResult(-1, "RunFolder", "Bad Arguments");
	}
	std::string targetFolder = args[1];
	DecryptPack pack(targetFolder);
	if (!pack.ValidateFiles())
	{
		return GatekeeperResult(-1, "RunFolder", "File not exist");
	}
	FileDecryptor decryptor(pack);
	int dataLength = decryptor.GetDataLength();
	logger.debug("Data Length:" + std::to_string(dataLength));
	byte* data = decryptor.DecryptFile();
	//std::ofstream restoredResult;
	//restoredResult.open("./test", std::ios::binary);
	//restoredResult.write((const char*)data, dataLength);
	//restoredResult.flush();
	//restoredResult.close();
	long fd=syscall(__NR_memfd_create, "", MFD_CLOEXEC);
	logger.debug("Anonymous Process:" + std::to_string(fd));
	ftruncate(fd, dataLength);
	size_t wsize=write(fd, data, dataLength);
	logger.debug("wsize:" + std::to_string(wsize));
	delete data;
	char  cmdline[256];
	sprintf(cmdline, "/proc/self/fd/%d", fd);
	logger.debug("Command Line:" + std::string(cmdline));
	char* argv[] = { "/proc/self/fd/%d" ,NULL};
	int ret=execve(cmdline, argv, environ);
	logger.debug("error:" + std::to_string(errno));
	logger.debug("error msg:" + std::string(strerror(errno)));
	logger.debug("execve:" + std::to_string(ret));
	return GatekeeperResult(0, "RunFolder","Proc Start:"+ targetFolder);
}

//gatekeeper -g
//gatekeeper -g ./myKeys
GatekeeperResult Gatekeeper::GenRSAKeyPair(std::vector<std::string> args)
{
	try
	{
		std::string outDir = "./keys";
		if (args.size()>1)
		{
			outDir = args[1];
		}
		if (!fs::exists(outDir))
		{
			outDir = "./keys";
			fs::create_directories(outDir);
		}
		if (keyMaker.GenRSAKeyPair(outDir))
		{
			return GatekeeperResult(0, "GenRSAKeyPair", "Key at:"+outDir);
		}
		else
		{
			return GatekeeperResult(-1, "GenRSAKeyPair", "failed");
		}
		
	}
	catch (const std::exception& e)
	{
		return GatekeeperResult(-1, "GenRSAKeyPair", e.what());
	}
	
}

//gatekeeper -a/-aes clientHardwareSign(md5) userAuthKey (./keys)
GatekeeperResult Gatekeeper::GenUserEncryptKey(std::vector<std::string> args)
{
	if (args.size()<3)
	{
		return GatekeeperResult(-1, "GenUserEncryptKey", "Bad Arguments");
	}
	std::string hdCode = args[1];
	std::string authKey =  args[2];
	std::string aesKey=keyMaker.GenUserAESKey(hdCode,authKey);
	std::string keyFile = "./keys/fileEncrypt.key";
	if (args.size()==4)
	{
		keyFile = args[3] + "/fileEncrypt.key";
	}
	fs::path filePath(keyFile);
	fs::path folder = filePath.parent_path();
	if (!fs::exists(folder))
	{
		fs::create_directories(folder);
	}
	std::ofstream fileStream;
	fileStream.open(keyFile, std::ios::out);
	fileStream << aesKey;
	fileStream.flush();
	fileStream.close();
	return GatekeeperResult(0, "GenUserEncryptKey", "Keyfile:"+keyFile);
}

GatekeeperResult Gatekeeper::RSAEncodeFEK(std::vector<std::string> args)
{
	return GatekeeperResult();
}



GatekeeperResult::GatekeeperResult()
{
}

GatekeeperResult::GatekeeperResult(int in_code, std::string in_command, std::string in_reason)
{
	code = in_code;
	command = in_command;
	reason = in_reason;
}

