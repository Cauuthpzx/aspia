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
// Win32 replacement for client/ui/desktop/record_settings_dialog.ui.
//

#ifndef CLIENT_WIN32_RECORD_SETTINGS_DIALOG_H
#define CLIENT_WIN32_RECORD_SETTINGS_DIALOG_H

//
// Resource identifiers owned by this dialog. These #define lines must be
// mirrored in client_win32/resource.h (do not edit resource.h from this
// change -- they are listed here for reference only):
//
//   #define IDD_RECORD_SETTINGS            260
//   #define IDC_RECORD_AUTOSTART           2600
//   #define IDC_RECORD_PATH_LABEL          2601
//   #define IDC_RECORD_PATH                2602
//   #define IDC_RECORD_BROWSE              2603
//   #define IDC_RECORD_FORMAT_LABEL        2604
//   #define IDC_RECORD_FORMAT              2605
//   #define IDC_RECORD_CODEC_LABEL         2606
//   #define IDC_RECORD_CODEC               2607
//

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class RecordSettingsDialog
{
public:
    struct Result
    {
        bool enable_recording = false;
        std::wstring output_path;
        int container_format = 0;  // index into the container format combo.
        int codec = 0;              // index into the codec combo.
    };

    RecordSettingsDialog(HINSTANCE instance, HWND parent);
    ~RecordSettingsDialog() = default;

    RecordSettingsDialog(const RecordSettingsDialog&) = delete;
    RecordSettingsDialog& operator=(const RecordSettingsDialog&) = delete;

    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void onBrowse(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_RECORD_SETTINGS_DIALOG_H
