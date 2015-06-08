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
#include "Mixer.h"


Mixer::Mixer() : m_hMixer( NULL ),
   m_volumeCtrlId( -1 ),
   m_muteCtrlId( -1 )
{
}


Mixer::~Mixer()
{
   Cleanup();
}


bool Mixer::Initialize( HWND hwndParent /* = NULL */ )
{
   Cleanup();

   if ( mixerGetNumDevs() > 0 )
   {
      if ( mixerOpen( &m_hMixer, 0, reinterpret_cast<DWORD_PTR>(hwndParent), NULL,
                      ( hwndParent == NULL ) ? MIXER_OBJECTF_MIXER : MIXER_OBJECTF_MIXER | CALLBACK_WINDOW ) == MMSYSERR_NOERROR )
      {
         return ( GetMasterVolumeControl() & GetMasterMuteControl() );
      }
   }

   return false;
}


void Mixer::Cleanup()
{
   if ( NULL != m_hMixer )
   {
      mixerClose( m_hMixer );
      m_hMixer = NULL;
      m_volumeCtrlId = -1;
      m_muteCtrlId = -1;
   }
}


bool Mixer::GetMasterVolumeControl()
{
   if ( NULL == m_hMixer )
   {
      return false;
   }

   MIXERLINE mxl;
   mxl.cbStruct = sizeof(MIXERLINE);
   mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

   if ( mixerGetLineInfo( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   MIXERCONTROL mxc;
   MIXERLINECONTROLS mxlc;
   mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
   mxlc.dwLineID = mxl.dwLineID;
   mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
   mxlc.cControls = 1;
   mxlc.cbmxctrl = sizeof(MIXERCONTROL);
   mxlc.pamxctrl = &mxc;

   if ( mixerGetLineControls( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   m_volumeCtrlId = mxc.dwControlID;

   return true;
}


bool Mixer::GetMasterMuteControl()
{
   if ( NULL == m_hMixer )
   {
      return false;
   }

   MIXERLINE mxl;
   mxl.cbStruct = sizeof(MIXERLINE);
   mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

   if ( mixerGetLineInfo( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   MIXERCONTROL mxc;
   MIXERLINECONTROLS mxlc;
   mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
   mxlc.dwLineID = mxl.dwLineID;
   mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
   mxlc.cControls = 1;
   mxlc.cbmxctrl = sizeof(MIXERCONTROL);
   mxlc.pamxctrl = &mxc;

   if ( mixerGetLineControls( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR )
   {
      return false;
   }

   m_muteCtrlId = mxc.dwControlID;

   return true;
}


bool Mixer::GetMasterVolume( DWORD& value )
{
   if ( NULL == m_hMixer )
   {
      return false;
   }

   MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
   MIXERCONTROLDETAILS mxcd;

   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_volumeCtrlId;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   mxcd.paDetails = &mxcdVolume;

   if ( mixerGetControlDetails(  reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   value = mxcdVolume.dwValue;

   return true;
}


bool Mixer::SetMasterVolume( DWORD value )
{
   if ( NULL == m_hMixer)
   {
      return false;
   }

   MIXERCONTROLDETAILS_UNSIGNED mxcdVolume = { value };
   MIXERCONTROLDETAILS mxcd;
   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_volumeCtrlId;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   mxcd.paDetails = &mxcdVolume;

   if ( mixerSetControlDetails(  reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   return true;
}


bool Mixer::GetMasterMute( LONG& value )
{
   if ( NULL == m_hMixer )
   {
      return false;
   }

   MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
   MIXERCONTROLDETAILS mxcd;
   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_muteCtrlId;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
   mxcd.paDetails = &mxcdMute;

   if ( mixerGetControlDetails( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   value = mxcdMute.fValue;

   return true;
}


bool Mixer::SetMasterMute( LONG value )
{
   if ( NULL == m_hMixer )
   {
      return false;
   }

   MIXERCONTROLDETAILS_BOOLEAN mxcdMute = { value };
   MIXERCONTROLDETAILS mxcd;
   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_muteCtrlId;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
   mxcd.paDetails = &mxcdMute;

   if ( mixerSetControlDetails( reinterpret_cast< HMIXEROBJ >( m_hMixer ), &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
   {
      return false;
   }

   return true;
}