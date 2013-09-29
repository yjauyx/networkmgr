// ConnectionManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ConnectionManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
using namespace Json;

void initServer();

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			initServer();
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

void initServer(){
	SocketManager socketMgr;
	socketMgr.initServer(NULL);
	socketMgr.startAccept();

	//string test = "{\"test\" : 5}";        //保存上文的json类型的数据

	////以下涉及json的操作，将在后文中涉及，此处为简单应用，不解释，你懂的
	//Reader reader;
	//Value value;

	//if (reader.parse(test, value))
	//{
	//	int i = 0;

	//	if (!value["test"].isNull())
	//	{
	//		i = value["test"].asInt();
	//		cout << i << endl;
	//	}
	//}
	//cout << "input any world to exit";
	char eof;
	cin >> eof;

}

