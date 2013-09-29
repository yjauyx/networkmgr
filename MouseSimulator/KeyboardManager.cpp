#include "stdafx.h"
#include "KeyboardManager.h"

KeyboardManager::KeyboardManager()
{
}


KeyboardManager::~KeyboardManager()
{
}

void KeyboardManager::Input(string str){
	HWND hWnd;
	hWnd = GetForegroundWindow(); // 得到当前窗口
	DWORD FormThreadID = GetCurrentThreadId();
	DWORD CWndThreadID = GetWindowThreadProcessId(hWnd, NULL);
	// 附加输入线程
	AttachThreadInput(CWndThreadID, FormThreadID, true);
	hWnd = GetFocus(); // 得到当前键盘光标所在的窗口
	AttachThreadInput(CWndThreadID, FormThreadID, false); // 取消
	if (hWnd == NULL) return;
	// 模拟键盘按键输入文本
	/*while (*buf != '\0')
	PostMessage(hWnd, WM_CHAR, (WPARAM)(unsigned char)*buf++, 0);*/

}
