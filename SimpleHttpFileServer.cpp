#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <iostream>
#include <sstream> // stringstream
#include <WinSock2.h>
#include <io.h> // _findfirst _findnext _findclose
#include <vector>
#include <direct.h>
#include <fstream>
#include <string>
#include <locale>
#include "escapeUrl.h"
// 告知连接器连接的时候要找ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#define IP "0.0.0.0"
#define PORT 8080
using namespace std;

// logger
void log(string s)
{
    time_t now = time(NULL);
    tm t;
    localtime_s(&t, &now);
    cout << "[LOG " << t.tm_year+1900<<"/" << t.tm_mon + 1 << "/" << t.tm_mday << " " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << "]>" << s << endl;
}

// version info
void versionInfo()
{
    string info = "SimpleHttpServer v0.0.1\n\tcreated by newbieking on 2022.8.18\nServer is running on 0.0.0.0:8080...\n\n";
    cout << info;
}

void assert(int ret, int correct_code)
{
    if (ret != correct_code)
    {
        exit(ret);
    }
}

bool checkDir(string parentPath, string childPath)
{

    bool ret = false;
    if (childPath[childPath.length() -1] == '/')
    {
        return true;
    }
    string path = parentPath + childPath;
    //文件句柄  
    intptr_t   hFile = 0;
    //文件信息  
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(path.c_str(), &fileinfo)) != -1)
    {
        if (fileinfo.attrib == _A_SUBDIR)
            ret = true;
    }
    _findclose(hFile);
    return ret;
}

// path: d:/*
void getFiles(string path, vector<_finddata_t>& files)
{
    //文件句柄  
    intptr_t   hFile = 0;
    //文件信息  
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(path.c_str(), &fileinfo)) != -1)
    {
        do
        {
            files.push_back(fileinfo);
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

string getFileListString(string parentPath, string childPath)
{
    //cout  <<"log: childPath: " << childPath << childPath._Equal("/") << endl;
    string path = parentPath + childPath + "/*";
    //cout << "path: " << path << endl;
    vector<_finddata_t> files;
    getFiles(path, files);
    string body;
    for (int i = 0; i < files.size(); i++)
    {
        stringstream ss;
        __time64_t s = files[i].time_write;
        int year = s / 60 / 60 / 24 / 365 + 1970;
        int month = (s - (year - 1970) * 365 * 24 * 60 * 60) / 60 / 60 / 24 / 30;
        string href;
        string name = string(files[i].name);
        if (childPath._Equal("/") && name._Equal("."))
        {
            href = "./";
        }
        // escape ./../
        else if (childPath._Equal("/") && name._Equal(".."))
        {
            continue;
        }
        else if (childPath._Equal("/"))
        {
            href = childPath + string(files[i].name);
        }
        else if(files[i].name == "..")
        {
            href = childPath + "/../";
        }
        else
        {
            if (childPath[childPath.length() - 1] == '/')
            {
                href = childPath + string(files[i].name);
            }
            else
            {
                href = childPath + "/" + string(files[i].name);
            }
        }

        ss << "<a href='" << href << "'>" << files[i].name << "</a>" << "<span class='file-size'>\t" << "\t" << year << "-" << month << "\t" << files[i].size / 1024 << "KB</span>" << "<br>";
        body += ss.str();
    }
    return body;
}

void sendFile(SOCKET clientSock, string filePath)
{
    //cout << "log: sendFile: " <<filePath<< endl;
    int i = filePath.find("\\");
    while (i < filePath.length())
    {
        filePath = filePath.replace(i,1,"/");
        i = filePath.find("\\");
    }
    i = filePath.find_last_of("/");
    // header
    string header = "HTTP/1.1 200 OK\ncontent-type: application/octet-stream\n\n";
    send(clientSock, header.c_str(), header.length(), 0);
    //cout << header.c_str() << endl;
    // body
    char buf[256];
    fstream fs(filePath, ios::in|ios::binary);
    if (!fs.is_open()) return;
    while (!fs.eof())
    {
        fs.read(buf, sizeof(buf));
        send(clientSock, buf, fs.gcount(), 0);
    }
    fs.close();
}

void sendDirHtml(SOCKET clientSock, string parentPath, string childPath)
{
    string buf;
    string header = "HTTP/1.1 200 OK\ncontent-type: text/html; charset=gbk\n\n";
    // folder path
    string body = getFileListString(parentPath , childPath);
    buf = header + "<pre>" + body + "</pre>";
    send(clientSock, buf.c_str(), buf.length(), 0);
    //closesocket(clientSock);
}

// multiByte gbk to multiByte utf8 by wchat_t
string Utf8ToGbk(const char* src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}

int main(int argc, char* argv[])
{
    versionInfo();
    //system("chcp 65001");
    //cout << decodeUrl("/%E6%BA%90.cpp") << endl;
    string rootDir;
    if (argc >= 2)
    {
        rootDir = argv[1];
    }
    else
    {
        rootDir = _getcwd(NULL, 0);
    }
    int ret;
    WSAData data;
    ret = WSAStartup(MAKEWORD(2, 2), &data);
    assert(ret, 0);

    // sockaddr_in
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.S_un.S_addr = inet_addr(IP);

    // socket
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);

    // bind
    ret = bind(serverSock, (sockaddr*)&sa, sizeof(sa));
    assert(ret,0);

    // listen
    ret = listen(serverSock, 5);
    assert(ret, 0);

    // accept
    sockaddr_in clientSa;
    int len = sizeof(clientSa);
    while (true)
    {
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientSa, &len);
        // console log
        // cout <<"client addr: " << inet_ntoa(clientSa.sin_addr) << endl ;
        
        int dataLen = 0;
        char buf[1024];
        do
        {
            dataLen = recv(clientSock, buf, sizeof(buf), 0);
            //cout << "waiting for data\t" << dataLen << endl;
            if (dataLen > 0)
            {        
                char* p = buf;
                char dir[128];
                // GET /path HTTP/1.1
                for (; *p != ' '; p++);
                p++;
                int i = 0;
                for (; *p != ' '; p++)
                {  
                    //cout << *p;
                    dir[i++] = *p;
                }
                dir[i] = '\0';
                string urlPath = string(dir);
                if (!urlPath._Equal("/favicon.ico"))
                {
                    //cout << checkDir(rootDir, urlPath) << endl;
                    if (checkDir(rootDir, urlPath))
                    {
                        //cout << "log: dir: (" <<urlPath<< ")" << rootDir << Utf8ToGbk(decodeUrl(urlPath).c_str()) << "/*" << endl;
                        log(rootDir + Utf8ToGbk(decodeUrl(urlPath).c_str()) + "/*");
                        sendDirHtml(clientSock, rootDir, Utf8ToGbk(decodeUrl(urlPath).c_str()));
                    }
                    else
                    {
                        //cout <<"log: file: (" << urlPath << ")" << rootDir << Utf8ToGbk(decodeUrl(urlPath).c_str()) << endl;
                        log(rootDir + Utf8ToGbk(decodeUrl(urlPath).c_str()));
                        sendFile(clientSock, rootDir + Utf8ToGbk(decodeUrl(urlPath).c_str()));
                    }
                }
                
            }
            else if (dataLen == 0)
            {
                //printf("Connection closed\n");
            }
            else
            {
                //printf("recv failed: %d\n", WSAGetLastError());
            } 
        } while (dataLen == 1024);

        
        // close socket
        shutdown(clientSock, SD_BOTH);
        //cout << "log: colse" << endl;
    }
    WSACleanup();
}
