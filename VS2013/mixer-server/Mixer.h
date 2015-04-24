#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>


const UINT RWM_MIXERCHANGED = RegisterWindowMessageW(L"RWM_MIXERCHANGED__72F28368_F7A6_43E2_BC6C_DF1C56CB3160");


class Mixer
{
public:
  Mixer();
  ~Mixer();

  bool Init(HWND hwnd);
  inline HRESULT GetLastError() { return hr_; }

  bool GetVolume(float& volume);
  bool SetVolume(float volume);

  bool GetMute(bool& mute);
  bool SetMute(bool mute);

private:
  HRESULT hr_;
  CComPtr<IMMDeviceEnumerator> enumerator_;
  CComPtr<IMMDevice> device_;
  CComPtr<IAudioEndpointVolume> volume_;
  GUID context_;

  class Callback : public IAudioEndpointVolumeCallback
  {
  public:
    Callback() : refCount_(1) {}
    ~Callback() {}

    HRESULT CreateContext(HWND hwnd)
    {
      hwnd_ = hwnd;
      return CoCreateGuid(&context_);
    }

    const GUID& Context() const { return context_; }

    ULONG STDMETHODCALLTYPE AddRef()
    {
      return InterlockedIncrement(&refCount_);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
      ULONG ulRef = InterlockedDecrement(&refCount_);
      if (0 == refCount_)
      {
        delete this;
      }
      return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
    {
      if (IID_IUnknown == riid)
      {
        AddRef();
        *ppvInterface = (IUnknown*)this;
      }
      else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
      {
        AddRef();
        *ppvInterface = (IAudioEndpointVolumeCallback*)this;
      }
      else
      {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
      }
      return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
    {
      if (NULL == pNotify)
      {
        return E_INVALIDARG;
      }
      else if (pNotify->guidEventContext != context_)
      {
        PostMessage(hwnd_, RWM_MIXERCHANGED, 0, 0);
      }
      return S_OK;
    }

  private:
    UINT refCount_;
    GUID context_;
    HWND hwnd_;
  };

  Callback callback_;
};

