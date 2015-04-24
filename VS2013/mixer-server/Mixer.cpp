#include "stdafx.h"
#include "Mixer.h"


Mixer::Mixer() : hr_(NOERROR)
{
}


Mixer::~Mixer()
{
  if (volume_)
  {
    volume_->UnregisterControlChangeNotify(
      (IAudioEndpointVolumeCallback*)&callback_);
  }
}


bool Mixer::Init(HWND hwnd)
{
  hr_ = callback_.CreateContext(hwnd);

  if (FAILED(hr_)) return false;

  hr_ = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
    CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
    (void**)&enumerator_);

  if (FAILED(hr_)) return false;

  hr_ = enumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &device_);

  if (FAILED(hr_)) return false;

  hr_ = device_->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL,
    (void**)&volume_);

  if (FAILED(hr_)) return false;

  hr_ = volume_->RegisterControlChangeNotify(
    (IAudioEndpointVolumeCallback*)&callback_);

  if (FAILED(hr_)) return false;

  return true;
}


bool Mixer::GetVolume(float& volume)
{
  hr_ = volume_->GetMasterVolumeLevelScalar(&volume);
  return SUCCEEDED(hr_);
}


bool Mixer::SetVolume(float volume)
{
  hr_ = volume_->SetMasterVolumeLevelScalar(volume, &callback_.Context());
  return SUCCEEDED(hr_);
}


bool Mixer::GetMute(bool& mute)
{
  BOOL temp = mute;
  hr_ = volume_->GetMute(&temp);
  mute = (temp != FALSE);
  return SUCCEEDED(hr_);
}


bool Mixer::SetMute(bool mute)
{
  hr_ = volume_->SetMute(mute, &callback_.Context());
  return SUCCEEDED(hr_);
}
