#include "stdafx.h"
#include "SocketManager.h"
#include <iostream>
#include <string.h>
#include <errno.h>
#include "json/json.h"
#include "json/writer.h"
#include "MouseManager.h"
#include "KeyboardManager.h"

#define OP_CMD "op_CMD"
#define OP_TARGET "op_target"
#define ID_NAME "id_name"
#define OP_MOVE "op_move"
#define OP_LEFT_CLICK "op_left_click"
#define OP_RIGHT_CLICK "op_right_click"
#define OP_DOUBLE_CLICK "op_double_click"
#define OP_KEYBOARD_EVENT "op_kb_event" 
#define MOVE_POS "move_pos"
#define INPUT_STR "input_str"
#define OP_DATA "op_data"


using namespace std;
using namespace Json;

SocketManager::SocketManager()
{
}


SocketManager::~SocketManager()
{
}

void SocketManager::StartConnect(int port, const char* address){
	int ret = Connect(port, address);
	if (ret == 0){
		DWORD ThreadID;
		CreateThread(NULL, 0, ReceiverThreadFun, (void*) &mSocket, 0, &ThreadID);
	}
}

int SocketManager::Connect(int port, const char* address){
	int rlt = 0;

	//用于记录错误信息并输出
	int iErrMsg;
	//启动WinSock
	WSAData wsaData;
	iErrMsg = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iErrMsg != NO_ERROR)
		//有错误
	{
		printf("failed with wsaStartup error : %d\n", iErrMsg);

		rlt = 1;
		return rlt;
	}

	//创建Socket
	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET)
		//创建Socket失败
	{
		printf("socket failed with error : %d\n", WSAGetLastError());

		rlt = 2;
		return rlt;
	}

	//目标服务器数据
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = htons(port);
	sockaddrServer.sin_addr.s_addr = inet_addr(address);

	//连接,sock与目标服务器连接
	iErrMsg = connect(mSocket, (sockaddr*) &sockaddrServer, sizeof(sockaddrServer));
	if (iErrMsg < 0)
	{
		printf("connect failed with error : %d\n", WSAGetLastError());

		rlt = 3;
		return rlt;
	}
	cout << "Connect to server successfully." << endl;

	return rlt;
}

DWORD WINAPI SocketManager::ReceiverThreadFun(void* Param){
	cout << "new thread is created for handling socket data" << endl;
	SOCKET socket = *(SOCKET*) Param;
	MouseManager mouseMgr;
	KeyboardManager keyBoardMgr;

	//string cmd_str = "{";
	//cmd_str.append("\"");
	//cmd_str.append(ID_NAME);
	//cmd_str.append("\"");
	//cmd_str.append(" : \"mouse\"}");

	Value cmd_json;
	cmd_json[ID_NAME] = Value("mouse");
	FastWriter fastWriter;

	cout << fastWriter.write(cmd_json) << endl;
	string cmd_str = fastWriter.write(cmd_json);
	cmd_str.append("\0");
	const char* cmd = cmd_str.c_str();
	int sentNum = send(socket, cmd, cmd_str.length(), 0);
	cout << "sent char num" << sentNum << endl;

	int offset(0);
	char buffer[512];
	while (true){
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN - 1;
		int iResult = recv(socket, recvbuf, recvbuflen, 0);
		if (iResult > 0){
			int index(0);
			int recvbuf_offset(0);
			for (int i = 0; i < iResult;){
				if (recvbuf[i] == '\n'){
					char* cmd_msg = new char[offset + i + 1];
					strncpy(cmd_msg, buffer, offset);
					strncpy(cmd_msg + offset, recvbuf + recvbuf_offset, i - recvbuf_offset);
					i = i + 1;
					recvbuf_offset = i;
					offset = 0;
					//cmd_list.push_back(cmd_msg);
					Reader reader;
					Value value;
					if (reader.parse(cmd_msg, value)){
						if (!value[OP_CMD].isNull()){
							string op_type = value[OP_CMD].asString();
							if (op_type.compare(OP_MOVE) == 0){
								if (!value[MOVE_POS].isNull()){
									Value mov_pos = value[MOVE_POS];
									mouseMgr.MouseMove(mov_pos["x"].asInt(), mov_pos["y"].asInt());
									//keyBoardMgr.Input("abc");
								}
							}
							else if (op_type.compare(OP_KEYBOARD_EVENT) == 0){
								if (!value[INPUT_STR].isNull()){
									keyBoardMgr.Input(value[INPUT_STR].asString());
								}
							}
							else if (op_type.compare(OP_LEFT_CLICK) == 0){
								mouseMgr.LeftClick();
							}
							else if (op_type.compare(OP_RIGHT_CLICK) == 0){
								mouseMgr.RightClick();
							}
						}
					}
				}
				else{
					i++;
				}
			}
			if (recvbuf_offset < iResult){
				strncpy(buffer, recvbuf + recvbuf_offset, iResult - recvbuf_offset);
				offset = iResult - recvbuf_offset;
			}
		}
		else{
			return 0;
		}
	}

	return 0;
}
