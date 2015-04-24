// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

#include <mmdeviceapi.h>
#include <endpointvolume.h>


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
  return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
  UIUpdateChildWindows();
  return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  // center the dialog on the screen
  CenterWindow();

  // set icons
  HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
  SetIcon(hIcon, TRUE);
  HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
  SetIcon(hIconSmall, FALSE);

  // register object for message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->AddMessageFilter(this);
  pLoop->AddIdleHandler(this);

  UIAddChildWindowContainer(m_hWnd);

  if (!mixer_.Init(m_hWnd)) CloseDialog(mixer_.GetLastError());

  sender_.Initialize();

  if (receiver_.Initialize())
  {
    receiver_.Listen(m_hWnd);
  }

  smallIcon_.Attach(hIconSmall);
  InstallIcon(_T("mixer-server"), smallIcon_, IDR_SYSTRAYMENU);

  return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  // unregister message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->RemoveMessageFilter(this);
  pLoop->RemoveIdleHandler(this);

  return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CloseDialog(wID);
  return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
  SendServerShutdown();
  DestroyWindow();
  ::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  bHandled = FALSE;
  WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);
  pWinPos->flags &= ~SWP_SHOWWINDOW;
  return 0;
}

LRESULT CMainDlg::OnMessageReceived(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  std::unique_ptr<Message> pMsg(reinterpret_cast<Message*>(wParam));

  switch (pMsg->GetType())
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
      for (int i = 0; i < payloadSize; ++i)
      {
        mapped |= ((int)(pMsg->GetPayload()[i]) & 0xff) << (((payloadSize - i) - 1) * 8);
      }
      float volume = (float)mapped / 65536;
      // This fixes the slider bouncing around on the client side
      //
      suppressFeedback_ = true;
      SetTimer(FEEDBACK_TIMER, FEEDBACK_TIMEOUT);
      mixer_.SetVolume(volume);
    }
    break;

  case Message::Mute:
    mixer_.SetMute(pMsg->GetPayload()[0] == 1);
    break;
  }

  return 0;
}

void CMainDlg::SendDiscoveryMessage()
{
  char name[Message::MAX_PAYLOAD];

  if (gethostname(name, Message::MAX_PAYLOAD) != SOCKET_ERROR)
  {
    Message msg(Message::Discovery, (unsigned char)strlen(name), name);
    sender_.Send(msg);
  }
}

void CMainDlg::SendVolumeToClient()
{
  if (!suppressFeedback_)
  {
    float volume = 0.0;
    if (mixer_.GetVolume(volume))
    {
      DWORD converted = (DWORD)(volume * 65536.0);
      char buffer[4];
      buffer[0] = static_cast<char>(converted >> 24 & 0xff);
      buffer[1] = static_cast<char>(converted >> 16 & 0xff);
      buffer[2] = static_cast<char>(converted >> 8 & 0xff);
      buffer[3] = static_cast<char>(converted & 0xff);
      Message msg(Message::Volume, 4, buffer);
      sender_.Send(msg);
    }
  }
}

void CMainDlg::SendMuteToClient()
{
  bool mute = false;
  if (mixer_.GetMute(mute))
  {
    char buffer[1];
    buffer[0] = (mute) ? 1 : 0;
    Message msg(Message::Mute, 1, buffer);
    sender_.Send(msg);
  }
}

void CMainDlg::SendServerShutdown()
{
  Message msg(Message::ServerShutdown);
  sender_.Send(msg);
}

LRESULT CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if (wParam == FEEDBACK_TIMER)
  {
    KillTimer(FEEDBACK_TIMER);
    suppressFeedback_ = false;
    SendVolumeToClient();
  }
  return 0;
}

LRESULT CMainDlg::OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  InstallIcon(_T("mixer-server"), smallIcon_, IDR_SYSTRAYMENU);
  return 0;
}

LRESULT CMainDlg::OnMixerChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  SendVolumeToClient();
  SendMuteToClient();
  return 0;
}
