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


#pragma once

#define REG_ROOT _T("Software\\MixerClient")
#define REG_ALWAYSONTOP _T("AlwaysOnTop")
#define REG_WINDOWPLACEMENT _T("WindowPlacement")

#define TIMER_DISCOVERY 1
#define TIMEOUT_DISCOVERY 1000

#define STEPS 255
#define RATIO 257

#define SNAP_GAP 10

#include "..\common\MulticastReceiver.h"
#include "..\common\MulticastSender.h"
#include "..\common\TrayIconImpl.h"
#include <memory>


const UINT RWM_SINGLEINSTANCE = RegisterWindowMessage( _T("RWM_SINGLE_INSTANCE__2FDEE7D4_83E3_4b68_BD0D_61F0901E9F6D") );
const UINT RWM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );


class CMainDlg : public CDialogImpl< CMainDlg >, public CUpdateUI< CMainDlg >,
		public CMessageFilter, public CIdleHandler, public CTrayIconImpl< CMainDlg >
{
public:

   CMainDlg() : receiver_( CLIENT_LISTEN_IP, CLIENT_LISTEN_PORT ),
      sender_( SERVER_LISTEN_IP, SERVER_LISTEN_PORT ) {}


	enum { IDD = IDD_MAINDLG };


	virtual BOOL PreTranslateMessage( MSG* pMsg )
	{
		return CWindow::IsDialogMessage( pMsg );
	}


	virtual BOOL OnIdle()
	{
		return FALSE;
	}


   void PlaceWindow()
   {
      CRegKey key;
      HRESULT hr = key.Create( HKEY_CURRENT_USER, REG_ROOT );

      if ( !SUCCEEDED( hr ) )
      {
         return;
      }

      WINDOWPLACEMENT wp;
      ULONG wpSize = sizeof( WINDOWPLACEMENT );

      if ( key.QueryBinaryValue( REG_WINDOWPLACEMENT, &wp, &wpSize ) == ERROR_SUCCESS )
      {
         wp.length = sizeof( WINDOWPLACEMENT );
         wp.flags = 0;
         wp.showCmd = ( wp.showCmd == SW_SHOWMINIMIZED ? SW_SHOWNORMAL : wp.showCmd );

         SetWindowPlacement( &wp );
      }
   }


	BEGIN_UPDATE_UI_MAP( CMainDlg )
	END_UPDATE_UI_MAP()


	BEGIN_MSG_MAP( CMainDlg )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
      MESSAGE_HANDLER( WM_NCHITTEST, OnNcHitTest )
      MESSAGE_HANDLER( WM_NCRBUTTONUP, OnNcRButtonUp )
      MESSAGE_HANDLER( WM_WINDOWPOSCHANGING, OnWindowPosChanging )
      MESSAGE_HANDLER( RWM_SINGLEINSTANCE, OnSingleInstance )
      MESSAGE_HANDLER( MulticastReceiver::RWM_RECEIVED, OnMessageReceived )
      MESSAGE_HANDLER( WM_TIMER, OnTimer )
      MESSAGE_HANDLER( WM_VSCROLL, OnVScroll )
      MESSAGE_HANDLER( RWM_TASKBARCREATED, OnTaskbarCreated )
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
      COMMAND_ID_HANDLER( ID_HIDE, OnHide )
      COMMAND_ID_HANDLER( ID_SHOW, OnShow )
      COMMAND_ID_HANDLER( ID_ALWAYSONTOP, OnAlwaysOnTop )
      COMMAND_HANDLER( IDC_MUTE, BN_CLICKED, OnMuted )
      CHAIN_MSG_MAP( CTrayIconImpl< CMainDlg > )
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
//	LRESULT CommandHandler( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
//	LRESULT NotifyHandler( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ )

private:

   CIcon smallIcon_;
   CMenu contextMenu_;
   ATL::CString toolTip_;

   MulticastReceiver receiver_;
   MulticastSender sender_;

	LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDR_MAINFRAME ), 
			IMAGE_ICON, ::GetSystemMetrics( SM_CXICON ), ::GetSystemMetrics( SM_CYICON ), LR_DEFAULTCOLOR );
		SetIcon( hIcon, TRUE );


		HICON hIconSmall = (HICON)::LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDR_MAINFRAME ), 
			IMAGE_ICON, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ), LR_DEFAULTCOLOR );
		SetIcon( hIconSmall, FALSE );
      smallIcon_.Attach( hIconSmall );

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT( pLoop != NULL );
		pLoop->AddMessageFilter( this );
		pLoop->AddIdleHandler( this );

		UIAddChildWindowContainer( m_hWnd );

      CRegKey key;

      if ( key.Open( HKEY_CURRENT_USER, REG_ROOT ) == ERROR_SUCCESS )
      {
         DWORD alwaysOnTop;

         key.QueryDWORDValue( REG_ALWAYSONTOP, alwaysOnTop );

         SetWindowPos( ( alwaysOnTop == TRUE ) ? HWND_TOPMOST : HWND_NOTOPMOST,
            -1, -1, -1, -1, SWP_NOSIZE | SWP_NOMOVE );

         key.Close();
      }

      contextMenu_.LoadMenu( IDR_CONTEXTMENU );

      ResetToolTip();

      CTrackBarCtrl trackBar( GetDlgItem( IDC_VOLUME ) );

      trackBar.SetRange( 0, STEPS );

      sender_.Initialize();

      if ( receiver_.Initialize() )
      {
         receiver_.Listen( m_hWnd );
      }

      SetTimer( TIMER_DISCOVERY, TIMEOUT_DISCOVERY );

		return TRUE;
	}


	LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
	{
      PersistWindowPlacement();
      PersistAlwaysOnTop();

		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT( pLoop != NULL );
		pLoop->RemoveMessageFilter( this );
		pLoop->RemoveIdleHandler( this );

		return 0;
	}


	LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
	{
		CloseDialog( wID );
		return 0;
	}


	void CloseDialog( int nVal )
	{
		DestroyWindow();
		::PostQuitMessage( nVal );
	}


   LRESULT OnNcRButtonUp( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ )
   {
      CMenuHandle menu = contextMenu_.GetSubMenu( 0 );
      menu.CheckMenuItem( ID_ALWAYSONTOP, IsAlwaysOnTop() ? MF_CHECKED : MF_UNCHECKED );
      menu.TrackPopupMenu( TPM_RIGHTBUTTON, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ), m_hWnd );

      return 0;
   }


   LRESULT CMainDlg::OnNcHitTest( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      return HTCAPTION;
   }


   BOOL IsAlwaysOnTop()
   {
      DWORD dwStyleEx = GetWindowLong( GWL_EXSTYLE );
      return ( ( dwStyleEx & WS_EX_TOPMOST ) == WS_EX_TOPMOST );
   }


   LRESULT OnHide( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
   {
      ShowWindow( SW_HIDE );
      InstallIcon( toolTip_, smallIcon_, IDR_SYSTRAYMENU );
      return 0;
   }


   LRESULT OnShow( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
   {
      RemoveIcon();
      ShowWindow( SW_SHOW );
      return 0;
   }


   LRESULT OnAlwaysOnTop( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
   {
      if ( IsAlwaysOnTop() )
      {
         SetWindowPos( HWND_NOTOPMOST, -1, -1, -1, -1, SWP_NOSIZE | SWP_NOMOVE );
      }
      else
      {
         SetWindowPos( HWND_TOPMOST, -1, -1, -1, -1, SWP_NOSIZE | SWP_NOMOVE );
      }

      return 0;
   }


   LRESULT OnWindowPosChanging( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ )
   {
      WINDOWPOS* pPos = reinterpret_cast< WINDOWPOS* >( lParam );

      HMONITOR hMonitor = MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTONEAREST );
      MONITORINFO monitorInfo;
      monitorInfo.cbSize = sizeof( MONITORINFO );
      GetMonitorInfo( hMonitor, &monitorInfo );

      RECT rcScreen = monitorInfo.rcWork;

      if ( abs( pPos->x - rcScreen.left ) <= SNAP_GAP )
      {
         pPos->x = rcScreen.left;
      }
      else if ( abs( pPos->x + pPos->cx - rcScreen.right ) <= SNAP_GAP )
      {
         pPos->x = rcScreen.right - pPos->cx;
      }

      if ( abs( pPos->y - rcScreen.top ) <= SNAP_GAP )
      {
         pPos->y = rcScreen.top;
      }
      else if ( abs( pPos->y + pPos->cy - rcScreen.bottom ) <= SNAP_GAP )
      {
         pPos->y = rcScreen.bottom - pPos->cy;
      }

      return 0;
   }


   void PersistWindowPlacement()
   {
      CRegKey key;
      HRESULT hr = key.Create( HKEY_CURRENT_USER, REG_ROOT );

      if ( !SUCCEEDED( hr ) )
      {
         return;
      }

      WINDOWPLACEMENT wp;
      wp.length = sizeof( WINDOWPLACEMENT );

      GetWindowPlacement( &wp );

      key.SetBinaryValue( REG_WINDOWPLACEMENT, &wp, sizeof( WINDOWPLACEMENT ) );
   }


   void PersistAlwaysOnTop()
   {
      CRegKey key;
      HRESULT hr = key.Create( HKEY_CURRENT_USER, REG_ROOT );

      if ( !SUCCEEDED( hr ) )
      {
         return;
      }

      key.SetDWORDValue( REG_ALWAYSONTOP, IsAlwaysOnTop() );
   }


   LRESULT OnSingleInstance( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ )
   {
      return RWM_SINGLEINSTANCE;
   }


   LRESULT OnMessageReceived( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      std::auto_ptr< Message > pMsg( reinterpret_cast< Message* >( wParam ) );

      switch ( pMsg->GetType() )
      {
      case Message::Discovery:

         KillTimer( TIMER_DISCOVERY );

         if ( pMsg->GetPayloadSize() > 0 )
         {
            char szServerName[Message::MAX_PAYLOAD+1];

            UINT size = pMsg->GetPayloadSize();

            memcpy( szServerName, pMsg->GetPayload(), pMsg->GetPayloadSize() );
            szServerName[pMsg->GetPayloadSize()] = NULL;

            ATL::CString strServerName( szServerName );

            toolTip_.Format( _T("MixerClient - %s"), strServerName );
         }

         break;

      case Message::Volume:

         {
            CTrackBarCtrl trackBar( GetDlgItem( IDC_VOLUME ) );

            int payloadSize = pMsg->GetPayloadSize();

            DWORD mapped = 0;

            for ( int i = 0; i < payloadSize; ++i )
            {
               mapped |= ( (int)( pMsg->GetPayload()[i] ) & 0xff ) << ( ( ( payloadSize - i ) - 1 ) * 8 );
            }

            int pos = STEPS - ( mapped / RATIO );

            trackBar.SetPos( pos );
         }

         break;

      case Message::Mute:

         {
            CButton checkBox( GetDlgItem( IDC_MUTE ) );
            checkBox.SetCheck( pMsg->GetPayload()[0] );
         }

         break;

      case Message::ServerShutdown:

         ResetToolTip();
         SetTimer( TIMER_DISCOVERY, TIMEOUT_DISCOVERY );

         break;
      }

      return 0;
   }


   LRESULT OnTimer( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      if ( wParam == TIMER_DISCOVERY )
      {
         Message msg( Message::Discovery );
         sender_.Send( msg );
      }

      return 0;
   }


   LRESULT OnMuted( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
   {
      CButton checkBox( GetDlgItem( IDC_MUTE ) );

      char buffer[1];
      buffer[0] = ( checkBox.GetCheck() == BST_CHECKED ) ? TRUE : FALSE;

      Message msg( Message::Mute, 1, buffer );

      sender_.Send( msg );

      return 0;
   }


   LRESULT OnVScroll( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      CTrackBarCtrl trackBar( GetDlgItem( IDC_VOLUME ) );

      int pos = STEPS - trackBar.GetPos();

      DWORD mapped = pos * RATIO;

      char buffer[4];
      buffer[0] = static_cast< char >( mapped >> 24 & 0xff );
      buffer[1] = static_cast< char >( mapped >> 16 & 0xff );
      buffer[2] = static_cast< char >( mapped >> 8 & 0xff );
      buffer[3] = static_cast< char >( mapped & 0xff );

      Message msg( Message::Volume, 4, buffer );

      sender_.Send( msg );

      return 0;
   }

   
   void ResetToolTip()
   {
      toolTip_ = _T("MixerClient");
      SetTooltipText( toolTip_ );
   }


   LRESULT OnTaskbarCreated( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      if ( !IsVisible() )
      {
         InstallIcon( toolTip_, smallIcon_, IDR_SYSTRAYMENU );
      }

      return 0;
   }


   BOOL IsVisible()
   {
      DWORD dwStyle = GetWindowLong( GWL_STYLE );
      return ( ( dwStyle & WS_VISIBLE ) == WS_VISIBLE );
   }
};
