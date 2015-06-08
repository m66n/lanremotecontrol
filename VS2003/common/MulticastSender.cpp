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
#include "MulticastSender.h"

#include <WS2tcpip.h>


MulticastSender::MulticastSender( LPCSTR address, USHORT port ) :
   address_( address ),
   port_( port ),
   socket_( INVALID_SOCKET )
{
}


MulticastSender::~MulticastSender()
{
   if ( INVALID_SOCKET != socket_ )
   {
      shutdown( socket_, 0x01 );
      closesocket( socket_ );
   }
}


bool MulticastSender::Initialize()
{
   if ( ( socket_ = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET )
   {
      return false;
   }

   SOCKADDR_IN sinSource;
   sinSource.sin_family = AF_INET;
   sinSource.sin_port = htons( 0 );
   sinSource.sin_addr.s_addr = htonl( INADDR_ANY );

   if ( bind( socket_, (struct sockaddr FAR*)&sinSource,
              sizeof(sinSource) ) == SOCKET_ERROR )
   {
      closesocket( socket_ );
      socket_ = INVALID_SOCKET;
      return false;
   }

   int ttl = 64;

   if ( setsockopt( socket_, IPPROTO_IP, IP_MULTICAST_TTL,
                    (char FAR*)&ttl, sizeof( int ) ) == SOCKET_ERROR )
   {
      closesocket( socket_ );
      socket_ = INVALID_SOCKET;
      return false;
   }

   sinDest_.sin_family = AF_INET;
   sinDest_.sin_port = htons( port_ );
   sinDest_.sin_addr.s_addr = inet_addr( address_ );

   return true;
}


bool MulticastSender::Send( Message& msg )
{
   return ( sendto( socket_, msg.GetBuffer(), msg.GetSize(), 0, (struct sockaddr FAR*)&sinDest_,
            sizeof(sinDest_) ) != SOCKET_ERROR );
}