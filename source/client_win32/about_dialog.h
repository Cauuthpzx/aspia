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
// Win32 replacement for common/ui/about_dialog.ui.
//
// resource.h additions:
//   #define IDD_ABOUT            210
//   #define IDC_ABOUT_NAME       2100
//   #define IDC_ABOUT_VERSION    2101
//   #define IDC_ABOUT_COPYRIGHT  2102
//   #define IDC_ABOUT_INFO       2103
//   #define IDC_ABOUT_SITE_LINK  2104
//   #define IDC_ABOUT_MAIL_LINK  2105
//   #define IDC_ABOUT_DONATE     2106
//

#ifndef CLIENT_WIN32_ABOUT_DIALOG_H
#define CLIENT_WIN32_ABOUT_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class AboutDialog
{
public:
    AboutDialog(HINSTANCE instance, HWND parent);
    ~AboutDialog() = default;

    AboutDialog(const AboutDialog&) = delete;
    AboutDialog& operator=(const AboutDialog&) = delete;

    bool exec();

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onLinkClicked(const std::wstring& url);

    HINSTANCE instance_;
    HWND parent_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_ABOUT_DIALOG_H
