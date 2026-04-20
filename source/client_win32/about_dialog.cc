//
// Aspia Project
// Copyright (C) 2016-2026 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

/*
 * Append the following DIALOGEX template to client_win32/client.rc:
 *
 * IDD_ABOUT DIALOGEX 0, 0, 260, 200
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "About"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     ICON            IDI_ASPIA,           -1,                   10,   8,  20,  20
 *     LTEXT           "Aspia",             IDC_ABOUT_NAME,       40,   8, 210,  12, SS_CENTER
 *     LTEXT           "Version 3.0.0",     IDC_ABOUT_VERSION,    40,  22, 210,  10, SS_CENTER
 *     LTEXT           "\xA9 2016-2026 Dmitry Chapyshev", IDC_ABOUT_COPYRIGHT, 40, 34, 210, 10, SS_CENTER
 *     LTEXT           "Aspia remote desktop and system information suite.",
 *                                          IDC_ABOUT_INFO,        7,  56, 246,  76, SS_LEFT | WS_BORDER
 *     CONTROL         "<a href=\"https://aspia.org\">https://aspia.org</a>",
 *                                          IDC_ABOUT_SITE_LINK, "SysLink",
 *                                          WS_TABSTOP | WS_CHILD | WS_VISIBLE, 7, 138, 246, 10
 *     CONTROL         "<a href=\"mailto:dmitry@aspia.ru\">dmitry@aspia.ru</a>",
 *                                          IDC_ABOUT_MAIL_LINK, "SysLink",
 *                                          WS_TABSTOP | WS_CHILD | WS_VISIBLE, 7, 150, 246, 10
 *     PUSHBUTTON      "Donate!",           IDC_ABOUT_DONATE,      7, 178,  60,  14
 *     DEFPUSHBUTTON   "Close",             IDOK,                203, 178,  50,  14
 * END
 */

#include "client_win32/about_dialog.h"

#include "client_win32/resource.h"

#include <commctrl.h>
#include <shellapi.h>

namespace aspia::client_win32 {

namespace {

constexpr wchar_t kDonateUrl[] = L"https://aspia.org/donate";

}  // namespace

AboutDialog::AboutDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool AboutDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_ABOUT), parent_,
        &AboutDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK AboutDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    AboutDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<AboutDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<AboutDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR AboutDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            onInitDialog(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wp))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDC_ABOUT_DONATE:
                    onLinkClicked(kDonateUrl);
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:
        {
            const NMHDR* hdr = reinterpret_cast<const NMHDR*>(lp);
            if (hdr && (hdr->code == NM_CLICK || hdr->code == NM_RETURN) &&
                (hdr->idFrom == IDC_ABOUT_SITE_LINK || hdr->idFrom == IDC_ABOUT_MAIL_LINK))
            {
                const NMLINK* link = reinterpret_cast<const NMLINK*>(lp);
                onLinkClicked(link->item.szUrl);
                return TRUE;
            }
            break;
        }

        default:
            break;
    }

    return FALSE;
}

void AboutDialog::onInitDialog(HWND hwnd)
{
    SetDlgItemTextW(hwnd, IDC_ABOUT_NAME, L"Aspia");
    SetDlgItemTextW(hwnd, IDC_ABOUT_VERSION, L"Version 3.0.0");
    SetDlgItemTextW(hwnd, IDC_ABOUT_COPYRIGHT, L"\u00A9 2016-2026 Dmitry Chapyshev");
    SetDlgItemTextW(hwnd, IDC_ABOUT_INFO,
                    L"Aspia remote desktop and system information suite.\r\n"
                    L"Built with Qt, protobuf, libvpx, libyuv, Opus and OpenSSL.");
    SetFocus(GetDlgItem(hwnd, IDOK));
}

void AboutDialog::onLinkClicked(const std::wstring& url)
{
    if (url.empty())
        return;
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

}  // namespace aspia::client_win32
