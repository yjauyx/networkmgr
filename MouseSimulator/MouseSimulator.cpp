// MouseSimulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MouseSimulator.h"
#include "MouseManager.h"
#include "SocketManager.h"
#include "KeyboardManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
void initMouse();
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
			initMouse();
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

void initMouse(){
	MouseManager mouseMgr;
	SocketManager socketMgr;
	socketMgr.StartConnect(8080,"127.0.0.1");
	char eof;
	cin >> eof;
}