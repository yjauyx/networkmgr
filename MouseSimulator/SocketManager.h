#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define DEFAULT_BUFLEN 50
#define DEFAULT_PORT "8080"

class SocketManager
{
public:
	SocketManager();
	~SocketManager();
	void StartConnect(int port, const char* address);
private:
	static DWORD WINAPI ReceiverThreadFun(void* Param);
	int Connect(int port, const char* address);
	SOCKET mSocket;
};

