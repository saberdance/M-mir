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
	if (args[0]=="-m"||args[0]=="-make")
	{
		logger.log("Run:Make Setup File", LHEADER);
		logger.output("working", 0);
		GatekeeperResult ret = MakeSetupFile(args);
		logger.log("MakeSetupFile Result:" + ret.reason);
		logger.log("", LFOOTER);
		if (ret.code==0)
		{
			logger.output("success", 100);
		}
		else
		{
			logger.output("failed", 100, ret.reason);
		}
		return ret;
	}
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


//gatekeeper -m/-make key deviceSig serverAddr productType outputFolder
GatekeeperResult Gatekeeper::MakeSetupFile(std::vector<std::string> args)
{
	if (args.size() != 6)
	{
		return GatekeeperResult(-1, "MakeSetupFile", "Bad Arguments");
	}
	std::string key = args[1];
	std::string sig = args[2];
	std::string serverAddr = args[3];
	std::string productType = args[4];
	std::string outputFolder = args[5];
	if (!fs::exists(outputFolder))
	{
		fs::create_directories(outputFolder);
	}
	std::string keyFolder = "./keys/" + key;
	fs::create_directories(keyFolder);
	logger.output("working", 10);
	GatekeeperResult genAESKeyRet=Raw_GenRSAKeyPair(keyFolder);
	logger.output("working", 15);
	GatekeeperResult genFileKeyRet = Raw_GenUserEncryptKey(sig, key, keyFolder + "/fileEncrypt.key");
	logger.output("working", 20);
	std::string sourceFolder = "/home/ubuntu/cotdocker";
	fs::copy(keyFolder, fs::path(sourceFolder + "/mimir/keys"), fs::copy_options::overwrite_existing);
	logger.output("working", 25);
	std::string cmd = std::string("cd /home/ubuntu/cotdocker/images && ./makelocal.sh ") + outputFolder + " " + key+" "+ serverAddr;
	logger.log("Command:" + cmd);
	int ret=system(cmd.c_str());
	logger.log("Command Ret:" + ret);
	if (ret != 0)
	{
		return GatekeeperResult(-1, "MakeSetupFile", "sh failed");
	}
	logger.output("working",91);
	return GatekeeperResult(0,"MakeSetupFile","success");
}

//gatekeeper -e/-encrypt ./myfile ./mykeys/fileEncrypt.key ./mykeys/publickey.pem ./mykeys/privatekey/.pem outputfolder
//gatekeeper -e ./myfile ./mykeys(use defalut key names as:"fileEncrypt.key publickey.pem privatekey.pem" in this folder)
GatekeeperResult Gatekeeper::EncryptFile(std::vector<std::string> args)
{
	if (args.size()!=6&&args.size()!=3)
	{
		return GatekeeperResult(-1, "EncryptFile", "Bad Arguments");
	}
	std::string sourceFile = args[1];
	std::string aeskeyFile = "";
	std::string publickeyFile = "";
	std::string privatekeyFile = "";
	std::string outputFolder = "./";
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
		outputFolder = args[5];
	}

	return Raw_EncryptFile(sourceFile, aeskeyFile, publickeyFile, privatekeyFile,outputFolder);
}

GatekeeperResult Gatekeeper::Raw_EncryptFile(std::__cxx11::string sourceFile, std::__cxx11::string aeskeyFile,
	std::__cxx11::string publickeyFile, std::__cxx11::string privatekeyFile,std::__cxx11::string outputFolder)
{
	EncryptPack pack(sourceFile, aeskeyFile, publickeyFile, privatekeyFile,outputFolder);
	if (!pack.ValidateFiles())
	{
		logger.error("Validate Encrypt Pack Error");
		logger.error(sourceFile);
		logger.error(aeskeyFile);
		logger.error(publickeyFile);
		logger.error(privatekeyFile);
		logger.error(outputFolder);
		return GatekeeperResult(-1, "EncryptFile", "File not exist");
	}
	FileCryptor cryptor(pack);
	bool succ = cryptor.EncryptFile();
	if (succ == false)
	{
		return GatekeeperResult(-1, "EncryptFile", "Encrypt Failed");
	}
	return GatekeeperResult(0, "EncryptFile", "File Encrypted:" + fs::path(sourceFile).parent_path().string() + "/" + fs::path(sourceFile).filename().string());
}

GatekeeperResult Gatekeeper::RunFolder(std::vector<std::string> args)
{
	if (args.size() != 2)
	{
		return GatekeeperResult(-1, "RunFolder", "Bad Arguments");
	}
	std::string targetFolder = args[1];
	return Raw_RunFolder(targetFolder);
}

GatekeeperResult Gatekeeper::Raw_RunFolder(std::__cxx11::string targetFolder)
{
	DecryptPack pack(targetFolder);
	if (!pack.ValidateFiles())
	{
		return GatekeeperResult(-1, "RunFolder", "File not exist");
	}
	FileDecryptor decryptor(pack);
	int dataLength = decryptor.GetDataLength();
	logger.debug("Data Length:" + std::to_string(dataLength));
	byte* data = decryptor.DecryptFile();
	long fd = syscall(__NR_memfd_create, "", MFD_CLOEXEC);
	logger.debug("Anonymous Process:" + std::to_string(fd));
	ftruncate(fd, dataLength);
	size_t wsize = write(fd, data, dataLength);
	logger.debug("wsize:" + std::to_string(wsize));
	delete data;
	char  cmdline[256];
	sprintf(cmdline, "/proc/self/fd/%d", fd);
	logger.debug("Command Line:" + std::string(cmdline));
	char* argv[] = { "/proc/self/fd/%d" ,NULL };
	int ret = execve(cmdline, argv, environ);
	return GatekeeperResult(0, "RunFolder", "Proc Start:" + targetFolder);
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
		return Raw_GenRSAKeyPair(outDir);
		
	}
	catch (const std::exception& e)
	{
		return GatekeeperResult(-1, "GenRSAKeyPair", e.what());
	}
	
}

GatekeeperResult Gatekeeper::Raw_GenRSAKeyPair(std::__cxx11::string outDir)
{
	if (keyMaker.GenRSAKeyPair(outDir))
	{
		return GatekeeperResult(0, "GenRSAKeyPair", "Key at:" + outDir);
	}
	else
	{
		return GatekeeperResult(-1, "GenRSAKeyPair", "failed");
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
	std::string keyFile = "./keys/fileEncrypt.key";
	if (args.size() == 4)
	{
		keyFile = args[3] + "/fileEncrypt.key";
	}
	return Raw_GenUserEncryptKey(hdCode, authKey, keyFile);
}

GatekeeperResult Gatekeeper::Raw_GenUserEncryptKey(std::__cxx11::string hdCode, std::__cxx11::string authKey, std::__cxx11::string keyFile)
{
	std::string aesKey = keyMaker.GenUserAESKey(hdCode, authKey);
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
	return GatekeeperResult(0, "GenUserEncryptKey", "Keyfile:" + keyFile);
}

GatekeeperResult Gatekeeper::RSAEncodeFEK(std::vector<std::string> args)
{
	return GatekeeperResult();
}

std::vector<std::string> Gatekeeper::LoadTargetConfigFile()
{
	return MUtil::getFileLines("./packSettings/target.txt");
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

