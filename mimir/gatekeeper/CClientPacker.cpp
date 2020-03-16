#include "CClientPacker.h"
#include <experimental/filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem::v1;

bool CClientPacker::Pack()
{
	std::string productSettingFile = "./packSettings/"+productname+"/productSetting.txt";
	if (!fs::exists(productSettingFile))
	{
		logger.error("Load C Client Product Setting Failed");
		return false;
	}
	std::vector<std::string> lines = MUtil::getFileLines(productSettingFile);
	for (auto line : lines)
	{
		//cclient=/root/sourceFiles/cclient
		if (line.substr(0, 6) == "cclient")
		{
			auto productFileFolder = line.substr(8);
			if (!fs::exists(productFileFolder))
			{
				return false;
			}
			clientpath = productFileFolder;
			break;
		}
	}
	logger.log("Found Client Folder:" + clientpath);	
	return PackFolder();
}

bool CClientPacker::PackFolder()
{
	if (clientpath==""||!fs::exists(clientpath))
	{
		logger.error("Invalid CClient Pack Folder:"+clientpath);
		return false;
	}
	bool succ = GenResourceConfigFile();
	if (!succ)
	{
		logger.error("GenResourceConfigFile Failed");
		return false;
	}
	succ = GenUserInfoFile();
	if (!succ)
	{
		logger.error("GenUserInfoFile Failed");
		return false;
	}
	else
	{
		logger.log("Pack Success");
		return true;
	}
	
}

/*
<Configure>
	<InnerIP></InnerIP>
	<ServerAddr>52.83.143.22:8889</ServerAddr>
	<Port>6666</Port>
	<Priority>1000</Priority>
	<UploadConLimit>50</UploadConLimit>
	<DownloadDeadline>0</DownloadDeadline>
	<UploadPageSize>5242880</UploadPageSize>
	<SrcName></SrcName>
	<Groups>
		<Name>AWSTest</Name>
		<FilePath>Datas</FilePath>
		<DownloadLimit>5454880</DownloadLimit>
		<UploadLimit>5454880</UploadLimit>
		<ThreadLimit>50</ThreadLimit>
	</Groups>
</Configure>
*/

bool CClientPacker::GenResourceConfigFile()
{
	fs::path resourceConfigurePath(clientpath + "/resourceConfigure.xml");
	if (!fs::exists(resourceConfigurePath))
	{
		logger.error("Invalid resourceConfigure Path:" + clientpath);
		return false;
	}
	//tinyxml2::XMLDocument doc;
	//tinyxml2::XMLElement* Configure = doc.NewElement("Configure");
	//doc.InsertEndChild(Configure);
	//tinyxml2::XMLElement* InnerIP = doc.NewElement("InnerIP");
	//Configure->InsertEndChild(InnerIP);
	//tinyxml2::XMLElement* ServerAddr = doc.NewElement("ServerAddr");
	//tinyxml2::XMLText* ServerAddrTxt = doc.NewText((serverip+":"+serverport).c_str());
	//ServerAddr->InsertEndChild(ServerAddrTxt);
	//Configure->InsertEndChild(ServerAddrTxt);
	std::ofstream ofs;
	ofs.open(resourceConfigurePath);
	std::string resourceInfo = R"(<Configure>
	<InnerIP></InnerIP>
	<ServerAddr>)"+serverip+R"(</ServerAddr>
	<Port>)"+serverport+R"(</Port>
	<Priority>1000</Priority>
	<UploadConLimit>50</UploadConLimit>
	<DownloadDeadline>0</DownloadDeadline>
	<UploadPageSize>5242880</UploadPageSize>
	<SrcName></SrcName>
	<Groups>
		<Name>Local</Name>
		<FilePath>Datas</FilePath>
		<DownloadLimit>5454880</DownloadLimit>
		<UploadLimit>5454880</UploadLimit>
		<ThreadLimit>50</ThreadLimit>
	</Groups>
</Configure>)";
	ofs << resourceInfo;
	ofs.flush();
	ofs.close();


}

bool CClientPacker::GenUserInfoFile()
{
	fs::path userInfoPath(clientpath + "/getUserInfo/information.json");
	if (!fs::exists(userInfoPath))
	{
		logger.error("Invalid userInfo Path:" + clientpath);
		return false;
	}
	std::string userInfo = R"({"Userinfo":{"user_name":"LocalUser","phone_number" : ")"+username+
		R"(","auth_key" : ")"+authkey+
		R"("},"Lastresult" : {"result":""},"Aboutrendermax" : {"arg":"","dir" : "","reder_max" : ""},"Tasknum" : {"task":1},"Market" : {"market_ip":")"+serverip+
		R"(","market_port" : ")"+serverport+R"("},"Upload" : {"up_ip":")"+serverip+R"(","up_port" : "8082"} })";
	logger.log("Gen UserInfo:");
	logger.log(userInfo);
	std::ofstream ofs;
	ofs.open(userInfoPath);
	ofs << userInfo;
	ofs.flush();
	ofs.close();
	return true;
}

