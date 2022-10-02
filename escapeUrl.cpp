#include "escapeUrl.h"
using namespace std;
short int hexChar2dec(char c) {
	if ('0' <= c && c <= '9') {
		return short(c - '0');
	}
	else if ('a' <= c && c <= 'f') {
		return (short(c - 'a') + 10);
	}
	else if ('A' <= c && c <= 'F') {
		return (short(c - 'A') + 10);
	}
	else {
		return -1;
	}
}

/*
	// '0' 48
	cout <<int('1'); // 1 49
	cout << endl;
	// 'a' -97 + 10 = -87
	// 'A' -65 + 10 = -55
	cout <<int('f'); // 16 102
	cout <<int('A'); // 10 65
*/
BYTES  hexStr2BytesStr(string hexStr)
{
	BYTES bytes = new BYTE[hexStr.length() / 2 + 1];
	int i;
	for (i = 0; i < hexStr.length(); i += 2)
	{
		bytes[i / 2] = int(hexChar2dec(hexStr[i]) * 16 + hexChar2dec(hexStr[i + 1]));
		//cout << int(bytes[i / 2]) << endl;
	}
	bytes[hexStr.length() / 2] = '\0';
	return bytes;
}

string decodeUrl(string url)
{
	char* ret = new char[url.length()+1];
	int index = 0;
	for (int i = 0; i < url.length(); i++, index++)
	{
		if (url[i] == '%')
		{
			//cout << "hex begin: " << endl;
			ret[index] = int(hexChar2dec(url[i+1]) * 16 + hexChar2dec(url[i + 2]));
			i += 2;
			//cout << "hex: " << ret[index]<<endl;
		}
		else
		{
			ret[index] = url[i];
			//cout << "non-hex: " << ret[index] << endl;
		}
	}
	ret[index] = '\0';
	//BYTES bytes = hexStr2BytesStr(url);
	////char s[256];
	////string ret;
	//stringstream ss;
	//for (int i = 0; i < strlen((const char*)bytes); i += 3)
	//{
	//	// c style
	//	//sprintf_s(s, "%c%c%c", bytes[i], bytes[i + 1], bytes[i + 2]);
	//	// cplusplus style
	//	ss << bytes[i] << bytes[i + 1] << bytes[i + 2];
	//}
	//cout << ss.str() << endl;
	//return ss.str();
	return string(ret);  
}
