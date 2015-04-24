// Copyright (c) 2002 Rob Caldecott
// 
// No explicit license provided.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#pragma once


#include <atlmisc.h>
#include <strsafe.h>


class CNotifyIconData : public NOTIFYICONDATA
{
public:
  CNotifyIconData()
  {
    memset(this, 0, sizeof(NOTIFYICONDATA));
    cbSize = sizeof(NOTIFYICONDATA);
  }
};


template < class T >
class CTrayIconImpl
{
private:

  UINT WM_TRAYICON;
  CNotifyIconData m_nid;
  bool m_bInstalled;
  UINT m_nDefault;

public:

  CTrayIconImpl() : m_bInstalled(false), m_nDefault(0)
  {
    WM_TRAYICON = ::RegisterWindowMessage(_T("WM_TRAYICON__734F3C44_73AB_4571_91B0_1B6D83E4EB33"));
  }


  ~CTrayIconImpl()
  {
    RemoveIcon();
  }


  bool InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID)
  {
    T* pT = static_cast<T*>(this);

    m_nid.hWnd = pT->m_hWnd;
    m_nid.uID = nID;
    m_nid.hIcon = hIcon;
    m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    StringCchCopy(m_nid.szTip, 128, lpszToolTip);

    m_bInstalled = Shell_NotifyIcon(NIM_ADD, &m_nid) ? true : false;

    return m_bInstalled;
  }


  bool RemoveIcon()
  {
    if (!m_bInstalled)
    {
      return false;
    }

    m_nid.uFlags = 0;

    return Shell_NotifyIcon(NIM_DELETE, &m_nid) ? true : false;
  }


  bool SetTooltipText(LPCTSTR pszTooltipText)
  {
    if (pszTooltipText == NULL)
    {
      return FALSE;
    }

    m_nid.uFlags = NIF_TIP;
    StringCchCopy(m_nid.szTip, 128, pszTooltipText);

    return Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
  }


  inline void SetDefaultItem(UINT nID) { m_nDefault = nID; }


  BEGIN_MSG_MAP(CTrayIcon)
    MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
  END_MSG_MAP()


  LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
  {
    if (wParam != m_nid.uID)
    {
      return 0;
    }

    T* pT = static_cast<T*>(this);

    if (LOWORD(lParam) == WM_RBUTTONUP)
    {
      CMenu oMenu;

      if (!oMenu.LoadMenu(m_nid.uID))
      {
        return 0;
      }

      CMenuHandle oPopup(oMenu.GetSubMenu(0));

      pT->PrepareMenu(oPopup);

      CPoint pos;
      GetCursorPos(&pos);

      SetForegroundWindow(pT->m_hWnd);

      if (m_nDefault == 0)
      {
        oPopup.SetMenuDefaultItem(0, TRUE);
      }
      else
      {
        oPopup.SetMenuDefaultItem(m_nDefault);
      }

      oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);

      // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
      pT->PostMessage(WM_NULL);

      oMenu.DestroyMenu();
    }
    else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
    {
      SetForegroundWindow(pT->m_hWnd);

      CMenu oMenu;

      if (!oMenu.LoadMenu(m_nid.uID))
      {
        return 0;
      }

      CMenuHandle oPopup(oMenu.GetSubMenu(0));

      if (m_nDefault)
      {
        pT->SendMessage(WM_COMMAND, m_nDefault, 0);
      }
      else
      {
        UINT nItem = oPopup.GetMenuItemID(0);
        pT->SendMessage(WM_COMMAND, nItem, 0);
      }

      oMenu.DestroyMenu();
    }

    return 0;
  }


  virtual void PrepareMenu(HMENU hMenu)
  {
    // user-defined override
  }

};
