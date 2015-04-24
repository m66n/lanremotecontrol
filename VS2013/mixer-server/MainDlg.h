// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Mixer.h"
#include "..\common\MulticastReceiver.h"
#include "..\common\MulticastSender.h"
#include "..\common\TrayIconImpl.h"
#include <memory>

#define FEEDBACK_TIMER 10
#define FEEDBACK_TIMEOUT 250

const UINT RWM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated"));

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
  public CMessageFilter, public CIdleHandler, public CTrayIconImpl < CMainDlg >
{
public:
  enum { IDD = IDD_MAINDLG };

  CMainDlg() : receiver_(SERVER_LISTEN_IP, SERVER_LISTEN_PORT),
    sender_(CLIENT_LISTEN_IP, CLIENT_LISTEN_PORT), suppressFeedback_(false) {}

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

  BEGIN_UPDATE_UI_MAP(CMainDlg)
  END_UPDATE_UI_MAP()

  BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
    MESSAGE_HANDLER(MulticastReceiver::RWM_RECEIVED, OnMessageReceived)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(RWM_TASKBARCREATED, OnTaskbarCreated)
    MESSAGE_HANDLER(RWM_MIXERCHANGED, OnMixerChanged)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
  END_MSG_MAP()

private:
  MulticastReceiver receiver_;
  MulticastSender sender_;
  bool suppressFeedback_;
  Mixer mixer_;
  CIcon smallIcon_;

  // Handler prototypes (uncomment arguments if needed):
  //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnMessageReceived(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnMixerChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  void CloseDialog(int nVal);

  void SendDiscoveryMessage();
  void SendVolumeToClient();
  void SendMuteToClient();
  void SendServerShutdown();
};
