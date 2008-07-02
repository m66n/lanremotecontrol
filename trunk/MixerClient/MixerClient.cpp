// Copyright (c) 2008 screamingtarget@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"
#include "MainDlg.h"


CAppModule _Module;


int Run( LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT )
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop( &theLoop );

	CMainDlg dlgMain;

	if ( dlgMain.Create( NULL ) == NULL )
	{
		ATLTRACE( _T("Main dialog creation failed!\n") );
		return 0;
	}

   dlgMain.PlaceWindow();

	dlgMain.ShowWindow( nCmdShow );

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();

	return nRet;
}


BOOL CALLBACK SearchForOther( HWND hWnd, LPARAM lParam )
{
   DWORD retval = 0;

   LRESULT result = SendMessageTimeout( hWnd, RWM_SINGLEINSTANCE,
      0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, &retval );

   if ( 0 == result )
   {
      return TRUE;
   }

   if ( RWM_SINGLEINSTANCE == retval )
   {
      HWND* other = reinterpret_cast< HWND* >( lParam );
      *other = hWnd;
      return FALSE;
   }

   return TRUE;
}


int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow )
{
   HANDLE hMutex = CreateMutex( NULL, FALSE, _T("MIXER_CLIENT__BA820706_601B_4530_9CA7_F21F07013FD8") );

   bool alreadyRunning = ( GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED );

   if ( alreadyRunning )
   {
      HWND hwndOther = NULL;
      EnumWindows( SearchForOther, reinterpret_cast< LPARAM >( &hwndOther ) );

      if ( NULL != hwndOther )
      {
         PostMessage( hwndOther, WM_COMMAND, MAKEWPARAM( ID_SHOW, 0 ), 0 );
         SetForegroundWindow( hwndOther );
      }

      return FALSE;
   }

	HRESULT hRes = ::CoInitialize( NULL );
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT( SUCCEEDED( hRes ) );

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc( NULL, 0, 0, 0L );

	AtlInitCommonControls( ICC_BAR_CLASSES );	// add flags to support other controls

	hRes = _Module.Init( NULL, hInstance );
	ATLASSERT( SUCCEEDED( hRes ) );

   WSADATA wsaData;
   WORD version = MAKEWORD( 2, 2 );

   int nRet = FALSE;

   if ( WSAStartup( version, &wsaData ) == 0 )
   {
      nRet = Run( lpstrCmdLine, nCmdShow );
      WSACleanup();
   }

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
