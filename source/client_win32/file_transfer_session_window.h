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
// Win32 replacement for client/ui/file_transfer/file_transfer_session_window.ui.
//
// Control IDs (range 4200..4299):
//   4200  IDC_FILETR_LOCAL_PANEL    - left-hand FilePanel child window
//   4201  IDC_FILETR_REMOTE_PANEL   - right-hand FilePanel child window
//   4202  IDC_FILETR_STATUSBAR      - status bar at the bottom
//
// WM_COMMAND IDs bubbled up from the FilePanel toolbars (forwarded here so
// the parent window can react if needed):
//   4210  IDC_FILETR_CMD_REFRESH
//   4211  IDC_FILETR_CMD_MKDIR
//   4212  IDC_FILETR_CMD_DELETE
//   4213  IDC_FILETR_CMD_SEND
//   4214  IDC_FILETR_CMD_HOME
//   4215  IDC_FILETR_CMD_UP
//   4216  IDC_FILETR_CMD_ADDRESS
//   4217  IDC_FILETR_CMD_DRIVES
//

#ifndef CLIENT_WIN32_FILE_TRANSFER_SESSION_WINDOW_H
#define CLIENT_WIN32_FILE_TRANSFER_SESSION_WINDOW_H

#include <windows.h>

#include <memory>

namespace aspia::client_win32 {

class FilePanel;

class FileTransferSessionWindow
{
public:
    explicit FileTransferSessionWindow(HINSTANCE instance);
    ~FileTransferSessionWindow();

    FileTransferSessionWindow(const FileTransferSessionWindow&) = delete;
    FileTransferSessionWindow& operator=(const FileTransferSessionWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id);
    void onDestroy();

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND status_ = nullptr;

    std::unique_ptr<FilePanel> localPanel_;
    std::unique_ptr<FilePanel> remotePanel_;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32FileTransferSession";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_FILE_TRANSFER_SESSION_WINDOW_H
