#pragma once
#include <string>
#include "../utils/mUtil.h"

class CClientPacker
{
public:
	CClientPacker(std::string serverIp, std::string serverPort,std::string productName,std::string authKey) : 
		serverip(serverIp),serverport(serverPort),productname(productName),authkey(authKey){}
public:
	bool Pack();
private:
	bool PackFolder();
	bool GenResourceConfigFile();
	bool GenUserInfoFile();
private:
	std::string serverip;
	std::string serverport;
	std::string authkey;
	std::string username;
	std::string productname;
	std::string clientpath="";
};

