#pragma once
#include <string>
#include <string.h>
#include "../utils/mUtil.h"

class EncryptPack
{
public:
	EncryptPack(std::string sourceFile, std::string aesKey, std::string publicKey, std::string privateKey,std::string outputFolder);
public:
	//test if files exists;
	bool ValidateFiles();
public:
	std::string sourcefile = "";
	std::string aeskey = "";
	std::string publickey = "";
	std::string privatekey = "";
	std::string outputfolder = "";
};

class DecryptPack
{
public:
	DecryptPack(std::string sourceFileFolder, std::string aesKey="",std::string privateKey="");
public:
	//test if files exists;
	bool ValidateFiles();
public:
	std::string sourcefolder = "";
	std::string aeskey = "";
	std::string privatekey = "";
};

class FileCryptor
{
public:
	FileCryptor(EncryptPack pack) : rawpack(pack) {}
public:
	bool EncryptFile();
private:
	bool EncodeSourceFile();
	bool MakeKeyFiles();
	bool CopyFilesToTargetFolder();
private:
	EncryptPack rawpack;
	std::string targetfolder;
};

class FileDecryptor
{
public:
	FileDecryptor(DecryptPack pack) : rawpack(pack) {
		GetOutDataLength();
	}
public:
	byte* DecryptFile();
	int GetDataLength();
private:
	bool LinkElf();
	bool RestoreSourceBinary();
	bool GetRunInfo();
	void GetOutDataLength();
private:
	DecryptPack rawpack;
	int outDataLength;
};

