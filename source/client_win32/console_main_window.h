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
// Win32 replacement for source/console/main_window.ui. Top-level admin
// console shell: menu bar + toolbar + tab control (one tab per open
// address book) + status bar.
//
// Control IDs (range 7400..7499) and resource IDs used by this window
// (kept local to the .cc until they are formally added to resource.h):
//   #define IDC_CONSOLE_MAIN_TABS       7400
//   #define IDC_CONSOLE_MAIN_STATUSBAR  7401
//   #define IDC_CONSOLE_MAIN_TOOLBAR    7402
//
//   // File menu
//   #define IDC_CONSOLE_NEW             7410
//   #define IDC_CONSOLE_OPEN            7411
//   #define IDC_CONSOLE_SAVE            7412
//   #define IDC_CONSOLE_SAVE_AS         7413
//   #define IDC_CONSOLE_SAVE_ALL        7414
//   #define IDC_CONSOLE_CLOSE           7415
//   #define IDC_CONSOLE_CLOSE_ALL       7416
//   #define IDC_CONSOLE_EXIT            7417
//
//   // Edit menu
//   #define IDC_CONSOLE_AB_PROPERTIES   7420
//   #define IDC_CONSOLE_ADD_GROUP       7421
//   #define IDC_CONSOLE_MODIFY_GROUP    7422
//   #define IDC_CONSOLE_DELETE_GROUP    7423
//   #define IDC_CONSOLE_ADD_COMPUTER    7424
//   #define IDC_CONSOLE_COPY_COMPUTER   7425
//   #define IDC_CONSOLE_MODIFY_COMPUTER 7426
//   #define IDC_CONSOLE_DELETE_COMPUTER 7427
//   #define IDC_CONSOLE_IMPORT          7428
//   #define IDC_CONSOLE_EXPORT          7429
//
//   // Tools menu
//   #define IDC_CONSOLE_FAST_CONNECT    7440
//   #define IDC_CONSOLE_ROUTER_MANAGE   7441
//
//   // Help menu
//   #define IDC_CONSOLE_ONLINE_HELP     7450
//   #define IDC_CONSOLE_CHECK_UPDATES   7451
//   #define IDC_CONSOLE_UPDATE_SETTINGS 7452
//   #define IDC_CONSOLE_ABOUT           7453
//
//   // Menu/accelerator resources:
//   #define IDR_CONSOLE_MAIN_MENU       510
//   #define IDR_CONSOLE_MAIN_ACCEL      511
//

#ifndef CLIENT_WIN32_CONSOLE_MAIN_WINDOW_H
#define CLIENT_WIN32_CONSOLE_MAIN_WINDOW_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class ConsoleMainWindow
{
public:
    explicit ConsoleMainWindow(HINSTANCE instance);
    ~ConsoleMainWindow();

    ConsoleMainWindow(const ConsoleMainWindow&) = delete;
    ConsoleMainWindow& operator=(const ConsoleMainWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

    // Appends a new address-book tab. Child content is a placeholder for
    // now; an AddressBookTab child widget will be wired up in a later
    // commit. Returns the new tab's zero-based index, or -1 on failure.
    int addAddressBookTab(const std::wstring& title);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id);
    void onDestroy();

    // Menu command handlers (stubs for now).
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileSaveAll();
    void onFileClose();
    void onFileCloseAll();
    void onFileExit();
    void onImportComputers();
    void onExportComputers();

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND tabs_ = nullptr;
    HWND status_ = nullptr;
    HWND toolbar_ = nullptr;
    HACCEL accel_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32ConsoleMain";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CONSOLE_MAIN_WINDOW_H
