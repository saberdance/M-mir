#pragma once
#include <string>
class BClientPacker
{
public:
	BClientPacker(std::string serverIp, std::string productName,std::string serverPort) : serverip(serverIp), productname(productName) ,serverport(serverPort){}
public:
	bool Pack();
private:
	std::string serverip;
	std::string serverport;
	std::string productname;
};