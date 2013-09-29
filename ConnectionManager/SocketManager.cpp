#include "stdafx.h"
#include "SocketManager.h"
#include "json/json.h"
#include <string.h>

#define OP_CMD "op_CMD"
#define OP_TARGET "op_target"
#define ID_NAME "id_name"
#define OP_MOVE "op_move"
#define MOVE_POS "move_pos"
#define OP_DATA "op_data"

using namespace std;
using namespace Json;


SocketManager::SocketManager()
{
}


SocketManager::~SocketManager()
{
}

int SocketManager::initServer(char* port){
	WSADATA wsaData;



	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
}

int SocketManager::startAccept(){
	cout << "start listening on port " << DEFAULT_PORT << endl;
	int iResult;
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Create a mutex with no initial owner
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	DWORD ThreadID;
	CreateThread(NULL, 0, dispatherThreadFun, NULL, 0, &ThreadID);

	// Accept a client socket
	while (true){
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			//WSACleanup();
		}
		else{
			DWORD ThreadID;
			CreateThread(NULL, 0, workerThreadFun, (void*) &ClientSocket, 0, &ThreadID);
			mSocket_List.insertSocket(ClientSocket);
		}
	}
	return 0;
}
SocketList SocketManager::mSocket_List;
std::list<char*> SocketManager::cmd_list;
HANDLE SocketManager::ghMutex;

DWORD WINAPI SocketManager::dispatherThreadFun(void* Param){
	cout << "create thread for tranmiting command" << endl;

	//transit received message to corresponding target
	DWORD dwWaitResult;
	while (true){
		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			INFINITE);  // no time-out interval

		std::list<char*>::iterator iterm;
		switch (dwWaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			if (cmd_list.empty()){
				Sleep(100);
				break;
			}
			for (iterm = cmd_list.begin(); iterm != cmd_list.end();){
				char* recvbuf = *iterm;
				string cmd_str = recvbuf;
				int iResult = cmd_str.length();
				Reader reader;
				Value value;
				boolean isParsed(false);
				isParsed = reader.parse(recvbuf, value, false);
				if (isParsed){
					int i = 0;
					if (!value[OP_TARGET].isNull()){
						SOCKET targetSocket = SocketManager::mSocket_List.getSocketById(value[OP_TARGET].asString());
						//transit cmd to target socket
						if (targetSocket != NULL && !value[OP_DATA].isNull()){

							Value op_data = value[OP_DATA];
							FastWriter fastWriter;
							string op_data_str = fastWriter.write(op_data);
							//cout << op_data_str << endl;
							const char* op_cmd_char = op_data_str.c_str();
							int iSendResult(0);
							iSendResult = send(targetSocket, op_cmd_char, op_data_str.length(), 0);
							if (iSendResult == SOCKET_ERROR) {
								printf("transit to target socket failed with error: %d\n", WSAGetLastError());
								SocketManager::mSocket_List.removeSocket(targetSocket);
								closesocket(targetSocket);
								WSACleanup();
								if (!ReleaseMutex(ghMutex))
								{
									// Deal with error.
								}
								return 1;
							}
						}
					}
				}
				delete [](*iterm);
				iterm = cmd_list.erase(iterm);
			}

			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Deal with error.
			}
			break;

			// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED:
			return FALSE;
		}
		if (!ReleaseMutex(ghMutex))
		{
			// Deal with error.
		}
	}
}

DWORD WINAPI SocketManager::workerThreadFun(void* Param){
	cout << "new thread is created for new client" << endl;
	SOCKET socket = *(SOCKET*) Param;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN - 1;
	cout << "wating for data..." << endl;
	boolean isRegistered(false);
	while (!isRegistered){
		int iResult = recv(socket, recvbuf, recvbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			closesocket(socket);
			return 0;
		}
		recvbuf[iResult] = '\0';
		cout << "data received" << recvbuf << endl;

		if (iResult > 0){
			Reader reader;
			Value value;
			if (reader.parse(recvbuf, value, false)){
				int i = 0;
				if (!value[ID_NAME].isNull()){
					SOCKETNODE* selfNode = SocketManager::mSocket_List.getSocketNodeBySocket(socket);
					if (selfNode != NULL){
						string id = value[ID_NAME].asString();
						selfNode->socket_id = id;
						cout << selfNode->socket_id << " is registered" << endl;
						isRegistered = true;
					}
				}
			}
		}
		int offset(0);
		char buffer[512];
		while (true){
			char recvbuf[DEFAULT_BUFLEN];
			int recvbuflen = DEFAULT_BUFLEN - 1;
			int iResult = recv(socket, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR || iResult == 0){
				SocketManager::mSocket_List.removeSocket(socket);
				closesocket(socket);
				return 0;
			}
			recvbuf[DEFAULT_BUFLEN - 1] = '\n';
			int index(0);
			int recvbuf_offset(0);
			for (int i = 0; i < iResult;){
				if (recvbuf[i] == '\0' && recvbuf[i + 1] == '\n'){
					char* cmd_msg = new char[offset + i + 2];
					if (offset > 0){
						strncpy(cmd_msg, buffer, offset);
					}
					if (recvbuf[0] == '\n' && recvbuf_offset == 0){
						recvbuf_offset = 1;
					}
					strncpy(cmd_msg + offset, recvbuf + recvbuf_offset, i - recvbuf_offset + 2);

					if (cmd_msg[0] != '{'){
						i = i + 2;
						recvbuf_offset = i;
						offset = 0;
						continue;
					}
					i = i + 2;
					recvbuf_offset = i;
					offset = 0;
					DWORD dwWaitResult;
					dwWaitResult = WaitForSingleObject(
						ghMutex,    // handle to mutex
						INFINITE);  // no time-out interval
					switch (dwWaitResult)
					{
						// The thread got ownership of the mutex
					case WAIT_OBJECT_0:
						cmd_list.push_back(cmd_msg);
						if (!ReleaseMutex(ghMutex)){
						}
						break;
						// The thread got ownership of an abandoned mutex
					case WAIT_ABANDONED:
						return FALSE;
					}
				}
				else{
					i++;
				}
			}
			if (recvbuf_offset < iResult){
				strncpy(buffer, recvbuf + recvbuf_offset, iResult - recvbuf_offset);
				offset += iResult - recvbuf_offset;
			}

		}
	}
}

SocketList::SocketList(){
	mHead = new SOCKETNODE();
	cur = mHead;
}

SocketList::~SocketList(){
	if (mHead != NULL){
		cur = mHead->next;
		while (cur != NULL){
			delete cur->pre;
			cur->pre = NULL;
			cur = cur->next;
		}
		delete mHead;
		mHead = NULL;
	}
}

bool SocketList::insertSocket(SOCKET socket){
	if (mHead == NULL){
		mHead = new SOCKETNODE();
		cur = mHead;
	}
	SOCKETNODE* new_socket = new SOCKETNODE();
	new_socket->socket = socket;
	new_socket->pre = cur;
	cur->next = new_socket;
	cur = new_socket;
	cur->next = NULL;
	return true;
}

bool SocketList::removeSocket(SOCKET socket){
	SOCKETNODE* tmp;
	tmp = mHead;
	while (tmp != NULL){
		if (tmp->socket == socket){
			if (tmp->pre != NULL){
				tmp->pre->next = tmp->next;
			}
			if (tmp->next != NULL){
				tmp->next->pre = tmp->pre;
			}
			delete tmp;
			return true;
		}
		else{
			tmp = tmp->next;
		}
	}
	return false;
}

SOCKET SocketList::getSocketById(string socket_id){
	SOCKETNODE* tmp;
	tmp = mHead;
	while (tmp != NULL){
		if (tmp->socket_id.compare(socket_id) == 0){
			return tmp->socket;
		}
		else{
			tmp = tmp->next;
		}
	}
	return NULL;
}

SOCKETNODE* SocketList::getSocketNodeBySocket(SOCKET socket){
	SOCKETNODE* tmp;
	tmp = mHead;
	while (tmp != NULL){
		if (tmp->socket == socket){
			return tmp;
		}
		else{
			tmp = tmp->next;
		}
	}
	return NULL;
}
