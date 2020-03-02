#pragma once
#include <openssl/rsa.h>  
#include <openssl/err.h>  
#include <openssl/pem.h>  
#include <openssl/aes.h>
#include <string>

class KeyMaker
{
public:
	std::string GenUserAESKey(std::string hardwareSign, std::string userAuthKey);
	bool GenRSAKeyPair(std::string outDir);
	std::string base64Decode(std::string s);
	std::string base64Encode(std::string s);
public:
	RSA* loadRSAPublicKey(const std::string& filePath);
	RSA* loadRSAPrivateKey(const std::string& filePath);
	void rsaEnviromentCleanup(RSA* pubKey, RSA* privateKey);
	std::string encodeRSA(RSA* rsaPublicKey, const std::string& encodeData);
	std::string decodeRSA(RSA* rsaPrivateKey, const std::string& strData);
	bool aesEncrypt(unsigned char* in, unsigned char* key, unsigned char* out, size_t len);
	bool aesDecrypt(unsigned char* in, unsigned char* key, unsigned char* out, size_t len);
};

