#pragma once
#include <iostream>
#include <string>
#include <sstream>
typedef unsigned char BYTE;
typedef unsigned char* BYTES;
BYTES  hexStr2BytesStr(std::string hexStr);
short int hexChar2dec(char c);
std::string decodeUrl(std::string url);

