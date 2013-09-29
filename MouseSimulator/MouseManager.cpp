#include "stdafx.h"
#include "MouseManager.h"
#include <Windows.h>
#include <math.h>
using namespace std;

MouseManager::MouseManager()
{
}


MouseManager::~MouseManager()
{
}

void MouseManager::MouseMove(int dx, int dy){

	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
	
	double fx = dx*(65535.0f / fScreenWidth);
	double fy = dy*(65535.0f / fScreenHeight);
	
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE /*| MOUSEEVENTF_ABSOLUTE*/;
	Input.mi.dx = dx ;
	Input.mi.dy = dy ;
	//cout << Input.mi.dx << " " << Input.mi.dy<<endl;

	::SendInput(1, &Input, sizeof(INPUT));
}

void MouseManager::LeftClick(){
	INPUT    Input = { 0 };
	// left down 
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	// left up
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void MouseManager::RightClick(){
	INPUT    Input = { 0 };
	// left down 
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	// left up
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}
