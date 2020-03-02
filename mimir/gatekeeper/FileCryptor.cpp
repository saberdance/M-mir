#include "FileCryptor.h"
#include "../utils/mUtil.h"
#include <experimental/filesystem>
#include "KeyMaker.h"
#include <fstream>
#include <string.h>

#define FILE_PART_NUM 16

namespace fs = std::experimental::filesystem::v1;

bool FileCryptor::EncryptFile()
{
	logger.log("EncryptFile Start");
	fs::path sourcePath(rawpack.sourcefile);
	logger.log("Source File:" + sourcePath.string());
	fs::path outFolder = fs::path(sourcePath.parent_path().string()+"/"+sourcePath.stem().string());
	targetfolder = outFolder.string();
	logger.log("Target Folder:" + targetfolder);
	if (!fs::exists(outFolder))
	{
		fs::create_directories(outFolder);
	}
	bool succ = EncodeSourceFile();
	if (succ)
	{
		succ = MakeKeyFiles();
	}
	return succ;

}

bool FileCryptor::EncodeSourceFile()
{
	logger.log("Encode Source File Start:"+rawpack.sourcefile);
	std::ifstream keyFile;
	keyFile.open(rawpack.aeskey);
	std::string key = { 0 };
	keyFile >> key;
	keyFile.close();
	logger.debug("Key Loaded:" + key);
	std::ifstream file;
	file.open(rawpack.sourcefile, std::ios::binary);
	int file_length;
	file.seekg(0, std::ios::end);
	file_length = file.tellg();
	file.seekg(0, std::ios::beg);
	file_length += 1;
	logger.debug("Source File Length:" + std::to_string(file_length));
	byte data[file_length];
	memset(data, 0, file_length);
	int outLength = file_length;
	byte out[outLength];
	memset(out, 0, outLength);
	file.read((char*)data, file_length);
	file.close();
	logger.debug("Source File Data Loaded:" + std::to_string(file_length));
	KeyMaker keymaker;
	bool succ=keymaker.aesEncrypt(data, (unsigned char*)(key.c_str()), out, file_length);
	if (!succ)
	{
		logger.error("AesEncrypt failed");
		return false;
	}
	std::ofstream fullResult;
	fullResult.open(targetfolder + "/elf_all", std::ios::binary);
	fullResult.write((const char*)out, outLength);
	fullResult.flush();
	fullResult.close();
	int splitLength = outLength / FILE_PART_NUM;
	logger.debug("splitLength:" + std::to_string(splitLength));
	for (size_t i = 0; i < FILE_PART_NUM; i++)
	{
		byte splitData[splitLength];
		memset(splitData, 0, splitLength);
		memcpy(splitData, out + i * splitLength, splitLength);
		std::ofstream outFile;
		outFile.open(targetfolder + "/elf_" + std::to_string(i),std::ios::binary);
		outFile.write((const char*)splitData, splitLength);
		outFile.flush();
		outFile.close();
		logger.debug(targetfolder + "/elf_" + std::to_string(i) + " generated");
	}
	return true;
}

bool FileCryptor::MakeKeyFiles()
{
	std::ifstream keyFile;
	keyFile.open(rawpack.aeskey);
	std::string key = { 0 };
	keyFile >> key;
	keyFile.close();
	fs::copy_file(rawpack.privatekey, targetfolder + "/elf_16");
	std::ofstream elf0;
	elf0.open(targetfolder + "/elf_0", std::ios::binary | std::ios::app);
	elf0.write(key.c_str(), key.length());
	elf0.flush();
	elf0.close();
	return true;
}

EncryptPack::EncryptPack(std::string sourceFile, std::string aesKey, std::string publicKey, std::string privateKey)
{
	sourcefile = sourceFile;
	aeskey = aesKey;
	publickey = publicKey;
	privatekey = privateKey;
}

bool EncryptPack::ValidateFiles()
{
	return fs::exists(sourcefile) && fs::exists(aeskey) && fs::exists(publickey) && fs::exists(privatekey);
}

DecryptPack::DecryptPack(std::string sourceFileFolder, std::string aesKey, std::string privateKey)
{
	sourcefolder = sourceFileFolder;
	aeskey = aesKey;
	privatekey = privateKey;
}

bool DecryptPack::ValidateFiles()
{
	if (!(fs::exists(sourcefolder) /*&& fs::exists(aeskey) && fs::exists(privatekey)*/))
	{
		return false;
	}
	for (size_t i = 0; i < 17; i++)
	{
		fs::path elffile(sourcefolder + "/elf_" + std::to_string(i));
		if (!fs::exists(elffile))
		{
			logger.error("Missing file:" + elffile.string());
			return false;
		}
	}
	return true;
}

byte* FileDecryptor::DecryptFile()
{
	logger.log("DecryptFile Start");
	fs::path sourcePath(rawpack.sourcefolder);
	logger.log("Source Path:" + sourcePath.string());
	std::ifstream firstfile;
	firstfile.open(rawpack.sourcefolder+"/elf_0", std::ios::binary);
	int file_length;
	firstfile.seekg(0, std::ios::end);
	file_length = firstfile.tellg();
	firstfile.seekg(0, std::ios::beg);
	//aes key
	file_length -= 32;
	byte* finalData=new byte[file_length * FILE_PART_NUM];
	memset(finalData, 0, file_length * FILE_PART_NUM);
	byte* firstData=new byte[file_length];
	memset(firstData, 0, file_length);
	byte* keyData=new byte[32];
	memset(firstData, 0, 32);
	firstfile.read((char*)firstData, file_length);
	firstfile.read((char*)keyData, 32);
	std::string key((char*)keyData);
	logger.debug("Key:" + key);
	firstfile.close();
	memcpy(finalData, firstData, file_length);
	for (size_t i = 1; i < FILE_PART_NUM; i++)
	{
		std::ifstream onefile;
		onefile.open(rawpack.sourcefolder + "/elf_"+std::to_string(i), std::ios::binary);
		byte oneData[file_length];
		memset(oneData, 0, file_length);
		onefile.read((char*)oneData, file_length);
		onefile.close();
		memcpy(finalData + i * file_length, oneData, file_length);
	}
	logger.log("Gen Encrypted Elf Done");
	byte restored[file_length* FILE_PART_NUM];
	memset(restored, 0, file_length* FILE_PART_NUM);
	//decrypt data
	KeyMaker keymaker;
	bool succ = keymaker.aesDecrypt(finalData, (unsigned char*)(key.c_str()), restored, file_length* FILE_PART_NUM);
	//std::ofstream restoredResult;
	//restoredResult.open("./decrypteddata", std::ios::binary);
	//restoredResult.write((const char*)restored, file_length* FILE_PART_NUM);
	//restoredResult.flush();
	//restoredResult.close();
	byte* ret = new byte[file_length * FILE_PART_NUM];
	memset(ret, 0, file_length * FILE_PART_NUM);
	memcpy(ret, restored, file_length * FILE_PART_NUM);
	delete finalData;
	delete firstData;
	delete keyData;
	return ret;
}

int FileDecryptor::GetDataLength()
{
	return outDataLength;
}

bool FileDecryptor::LinkElf()
{
	return false;
}

bool FileDecryptor::RestoreSourceBinary()
{
	return false;
}

bool FileDecryptor::GetRunInfo()
{
	return false;
}

void FileDecryptor::GetOutDataLength()
{
	std::ifstream firstfile;
	firstfile.open(rawpack.sourcefolder + "/elf_0", std::ios::binary);
	int file_length;
	firstfile.seekg(0, std::ios::end);
	file_length = firstfile.tellg();
	firstfile.seekg(0, std::ios::beg);
	file_length -= 32;
	firstfile.close();
	outDataLength = file_length * FILE_PART_NUM;
}
