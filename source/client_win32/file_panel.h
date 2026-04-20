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
// Win32 replacement for client/ui/file_transfer/file_panel.ui.
//
// Control IDs (range 4100..4199):
//   IDC_FILE_PANEL_NAME_LABEL    = 4100  (static header label)
//   IDC_FILE_PANEL_DRIVE_COMBO   = 4101  (drive/bookmark combo-box)
//   IDC_FILE_PANEL_ADDRESS_EDIT  = 4102  (current path edit)
//   IDC_FILE_PANEL_BTN_UP        = 4110  (Up)
//   IDC_FILE_PANEL_BTN_REFRESH   = 4111  (Refresh)
//   IDC_FILE_PANEL_BTN_HOME      = 4112  (Home)
//   IDC_FILE_PANEL_BTN_ADD_DIR   = 4113  (Add folder)
//   IDC_FILE_PANEL_BTN_DELETE    = 4114  (Delete)
//   IDC_FILE_PANEL_BTN_SEND      = 4115  (Send / Receive)
//   IDC_FILE_PANEL_BTN_UPLOAD    = 4116  (Upload)
//   IDC_FILE_PANEL_BTN_DOWNLOAD  = 4117  (Download)
//   IDC_FILE_PANEL_LIST          = 4120  (SysListView32 of entries)
//   IDC_FILE_PANEL_STATUS_LABEL  = 4121  (status line at bottom)
//

#ifndef CLIENT_WIN32_FILE_PANEL_H
#define CLIENT_WIN32_FILE_PANEL_H

#include <windows.h>
#include <commctrl.h>

#include <cstdint>
#include <span>
#include <string>

namespace aspia::client_win32 {

class FilePanel
{
public:
    struct Entry
    {
        std::wstring name;
        int64_t size = 0;
        bool is_dir = false;
        FILETIME modified = {};
    };

    // Control identifiers (relative to this panel's HWND).
    static constexpr int kIdNameLabel    = 4100;
    static constexpr int kIdDriveCombo   = 4101;
    static constexpr int kIdAddressEdit  = 4102;
    static constexpr int kIdBtnUp        = 4110;
    static constexpr int kIdBtnRefresh   = 4111;
    static constexpr int kIdBtnHome      = 4112;
    static constexpr int kIdBtnAddDir    = 4113;
    static constexpr int kIdBtnDelete    = 4114;
    static constexpr int kIdBtnSend      = 4115;
    static constexpr int kIdBtnUpload    = 4116;
    static constexpr int kIdBtnDownload  = 4117;
    static constexpr int kIdList         = 4120;
    static constexpr int kIdStatusLabel  = 4121;

    FilePanel(HWND parent, HINSTANCE instance);
    ~FilePanel();

    FilePanel(const FilePanel&) = delete;
    FilePanel& operator=(const FilePanel&) = delete;

    // Creates the child HWND. Returns true on success.
    bool create();

    // Show or hide this panel.
    void show(bool visible = true);

    // Underlying HWND of the panel (child window hosting all controls).
    HWND handle() const { return hwnd_; }

    // Update the address-bar text with a new path.
    void setPath(const std::wstring& path);

    // Replace the contents of the file list view.
    void reloadList(std::span<const Entry> entries);

    // Optional title shown in the top-most label (e.g. "Local" / "Remote").
    void setPanelName(const std::wstring& name);

    // Update the status line at the bottom.
    void setStatusText(const std::wstring& text);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onDestroy();

    void createChildren();
    void layoutChildren(int width, int height);
    void populateListColumns();

    HWND parent_ = nullptr;
    HINSTANCE instance_ = nullptr;

    HWND hwnd_ = nullptr;
    HWND nameLabel_ = nullptr;
    HWND driveCombo_ = nullptr;
    HWND addressEdit_ = nullptr;
    HWND toolbar_ = nullptr;
    HWND list_ = nullptr;
    HWND statusLabel_ = nullptr;

    HIMAGELIST toolbarImages_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32FilePanel";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_FILE_PANEL_H
