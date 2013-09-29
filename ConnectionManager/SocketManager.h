#pragma once
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <list>

#define DEFAULT_BUFLEN 100
#define DEFAULT_PORT "8080"
using namespace std;

struct SOCKETNODE{
	SOCKETNODE* pre;
	SOCKETNODE* next;
	SOCKET socket;
	string socket_id;
};

class SocketList
{
public:
	SocketList();
	~SocketList();
	bool insertSocket(SOCKET socket);
	bool removeSocket(SOCKET socket);
	SOCKET getSocketById(string socket_id);
	SOCKETNODE* getSocketNodeBySocket(SOCKET socket);

private:
	SOCKETNODE* cur;
	SOCKETNODE* mHead;
};

class SocketManager
{
public:
	SocketManager();
	~SocketManager();
	int initServer(char* port);
	int startAccept();
	static DWORD WINAPI workerThreadFun(void* Param);
	static DWORD WINAPI dispatherThreadFun(void* Param);
	static SocketList mSocket_List;
	static std::list<char*> cmd_list;
	static HANDLE ghMutex;
	
private:
	SOCKET ListenSocket;
};



