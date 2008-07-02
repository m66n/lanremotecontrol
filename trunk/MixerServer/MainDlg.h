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

#include "..\common\MulticastReceiver.h"
#include "..\common\MulticastSender.h"
#include "Mixer.h"
#include "..\common\TrayIconImpl.h"
#include <memory>

#define FEEDBACK_TIMER 10
#define FEEDBACK_TIMEOUT 250


class CMainDlg : public CDialogImpl< CMainDlg >, public CUpdateUI< CMainDlg >,
		public CMessageFilter, public CIdleHandler, public CTrayIconImpl< CMainDlg >
{
public:

   CMainDlg() : receiver_( SERVER_LISTEN_IP, SERVER_LISTEN_PORT ),
      sender_( CLIENT_LISTEN_IP, CLIENT_LISTEN_PORT ), suppressFeedback_( false ) {}

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage( MSG* pMsg )
	{
		return CWindow::IsDialogMessage( pMsg );
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP( CMainDlg )
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP( CMainDlg )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
      MESSAGE_HANDLER( WM_WINDOWPOSCHANGING, OnWindowPosChanging )
      MESSAGE_HANDLER( MulticastReceiver::RWM_RECEIVED, OnMessageReceived )
      MESSAGE_HANDLER( MM_MIXM_CONTROL_CHANGE, OnMixmControlChange )
      MESSAGE_HANDLER( WM_TIMER, OnTimer )
		COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
      CHAIN_MSG_MAP( CTrayIconImpl< CMainDlg > )
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
//	LRESULT CommandHandler( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
//	LRESULT NotifyHandler( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ )

private:

   MulticastReceiver receiver_;
   MulticastSender sender_;

   Mixer mixer_;

   bool suppressFeedback_;


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

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT( pLoop != NULL );
		pLoop->AddMessageFilter( this );
		pLoop->AddIdleHandler( this );

		UIAddChildWindowContainer( m_hWnd );

      mixer_.Initialize( m_hWnd );

      sender_.Initialize();

      if ( receiver_.Initialize() )
      {
         receiver_.Listen( m_hWnd );
      }

      InstallIcon( _T("MixerServer"), hIconSmall, IDR_SYSTRAYMENU );

		return TRUE;
	}


	LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
	{
      SendServerShutdown();

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


   LRESULT OnWindowPosChanging( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled )
   {
      bHandled = FALSE;

      WINDOWPOS* pWinPos = reinterpret_cast< WINDOWPOS* >( lParam );
      pWinPos->flags &= ~SWP_SHOWWINDOW;

      return 0;
   }


   LRESULT OnMessageReceived( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      std::auto_ptr< Message > pMsg( reinterpret_cast< Message* >( wParam ) );

      switch ( pMsg->GetType() )
      {
      case Message::Discovery:

         SendDiscoveryMessage();
         SendVolumeToClient();
         SendMuteToClient();

         break;

      case Message::Volume:

         {
            int payloadSize = pMsg->GetPayloadSize();

            DWORD mapped = 0;

            for ( int i = 0; i < payloadSize; ++i )
            {
               mapped |= ( (int)(pMsg->GetPayload()[i]) & 0xff ) << ( ( ( payloadSize - i ) - 1 ) * 8 );
            }

            // This fixes the slider bouncing around on the client side
            //
            suppressFeedback_ = true;
            SetTimer( FEEDBACK_TIMER, FEEDBACK_TIMEOUT );

            mixer_.SetMasterVolume( mapped );
         }

         break;

      case Message::Mute:

         mixer_.SetMasterMute( pMsg->GetPayload()[0] );

         break;
      }

      return 0;
   }


   LRESULT OnMixmControlChange( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ )
   {
      if ( mixer_.IsMasterVolume( lParam ) )
      {
         SendVolumeToClient();

         // bug in Windows mixer doesn't unmute on volume change
         //
         LONG mute;
         mixer_.GetMasterMute( mute );

         if ( mute )
         {
            mixer_.SetMasterMute( 0 );
         }
      }
      else if ( mixer_.IsMasterMute( lParam ) )
      {
         SendMuteToClient();
      }

      return 0;
   }


   void SendDiscoveryMessage()
   {
      char name[Message::MAX_PAYLOAD];

      if ( gethostname( name, Message::MAX_PAYLOAD ) != SOCKET_ERROR )
      {
         Message msg( Message::Discovery, strlen( name ), name );
         sender_.Send( msg );
      }
   }


   void SendVolumeToClient()
   {
      if ( !suppressFeedback_ )
      {
         DWORD volume = 0;
         mixer_.GetMasterVolume( volume );

         char buffer[4];
         buffer[0] = static_cast< char >( volume >> 24 & 0xff );
         buffer[1] = static_cast< char >( volume >> 16 & 0xff );
         buffer[2] = static_cast< char >( volume >> 8 & 0xff );
         buffer[3] = static_cast< char >( volume & 0xff );

         Message msg( Message::Volume, 4, buffer );

         sender_.Send( msg );
      }
   }


   void SendMuteToClient()
   {
      LONG mute = 0;
      mixer_.GetMasterMute( mute );

      char buffer[1];
      buffer[0] = ( mute ) ? 1 : 0;

      Message msg( Message::Mute, 1, buffer );

      sender_.Send( msg );
   }


   void SendServerShutdown()
   {
      Message msg( Message::ServerShutdown );
      sender_.Send( msg );
   }


   LRESULT OnTimer( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
   {
      if ( wParam == FEEDBACK_TIMER )
      {
         KillTimer( FEEDBACK_TIMER );
         suppressFeedback_ = false;
         SendVolumeToClient();
      }

      return 0;
   }
};
