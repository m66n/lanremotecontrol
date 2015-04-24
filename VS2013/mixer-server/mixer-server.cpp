// mixer-server.cpp : main source file for mixer-server.exe
//

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
  HANDLE hMutex = CreateMutexW(NULL, FALSE, L"MIXER_SERVER__E19BF9E7_15EE_438A_AFD9_219768F38C23");

  bool alreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED);

  if (alreadyRunning) { return FALSE; }

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

  WSADATA wsaData;
  WORD version = MAKEWORD(2, 2);

  int nRet = FALSE;

  if (WSAStartup(version, &wsaData) == 0)
  {
    nRet = Run(lpstrCmdLine, /*SW_MINIMIZE*/ nCmdShow);
    WSACleanup();
  }

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
