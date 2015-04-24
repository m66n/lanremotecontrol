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
#include "MulticastReceiver.h"
#include "Comm.h"

#include <ws2tcpip.h>


UINT MulticastReceiver::RWM_RECEIVED = RegisterWindowMessage( _T("RWM_RECEIVED__2629ED33_74F9_4c76_86BA_9E6C333BCF1A") );


MulticastReceiver::MulticastReceiver( LPCSTR address, USHORT port ) :
   address_( address ),
   port_( port ),
   socket_( INVALID_SOCKET ),
   hwndListener_( NULL ),
   hThread_( NULL )
{
}


MulticastReceiver::~MulticastReceiver()
{
   if ( INVALID_SOCKET != socket_ )
   {
      shutdown( socket_, 0x00 );
      closesocket( socket_ );
   }

   if ( NULL != hThread_ )
   {
      WaitForSingleObject( hThread_, INFINITE );
      CloseHandle( hThread_ );
   }
}


bool MulticastReceiver::Initialize()
{
   if ( ( socket_ = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET )
   {
      return false;
   }

   SOCKADDR_IN sinLocal;
   sinLocal.sin_family = AF_INET;
   sinLocal.sin_port = htons( port_ );
   sinLocal.sin_addr.s_addr = htonl( INADDR_ANY );
   
   if ( bind( socket_, (struct sockaddr FAR*)&sinLocal,
              sizeof(sinLocal) ) == SOCKET_ERROR )
   {
      closesocket( socket_ );
      socket_ = INVALID_SOCKET;
      return false;
   }

   struct ip_mreq mreq;
   mreq.imr_multiaddr.s_addr = inet_addr( address_ );
   mreq.imr_interface.s_addr = INADDR_ANY;

   if ( setsockopt( socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (char FAR*)&mreq, sizeof( mreq ) ) == SOCKET_ERROR )
   {
      closesocket( socket_ );
      socket_ = INVALID_SOCKET;
      return false;
   }

   return true;
}


bool MulticastReceiver::Listen( HWND hWndListener )
{
   hwndListener_ = hWndListener;

   DWORD threadId = 0;

   return ( ( hThread_ = CreateThread( NULL, 0, ListenProc, this, 0, &threadId ) ) != NULL );
}


DWORD WINAPI MulticastReceiver::ListenProc( LPVOID lpArg )
{
   if ( NULL == lpArg )
   {
      return -1;
   }

   MulticastReceiver* pMe = reinterpret_cast< MulticastReceiver* >( lpArg );
   
   char incoming[Message::MAX_SIZE];

   SOCKADDR_IN sinRecv;
   int sinRecvSize = sizeof( sinRecv );

   while ( true )
   {
      if ( recvfrom( pMe->socket_, incoming, Message::MAX_SIZE, 0,
                     (struct sockaddr FAR*)&sinRecv, &sinRecvSize ) == SOCKET_ERROR )
      {
         break;
      }
      else
      {
         Message* pMsg = new Message( incoming );
         PostMessage( pMe->hwndListener_, RWM_RECEIVED, reinterpret_cast< WPARAM >( pMsg ), 0 );
      }
   }

   return 0;
}
