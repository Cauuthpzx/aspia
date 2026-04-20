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
// Win32 replacement for client/ui/settings_dialog.ui.
//
// When wiring this dialog up, add the following to
// source/client_win32/resource.h (do NOT edit that file from this port;
// these are the identifiers the implementation below expects):
//
// #define IDD_SETTINGS                       270
//
// // --- General section ---
// #define IDC_SETTINGS_LANGUAGE              2700
// #define IDC_SETTINGS_THEME                 2701
// #define IDC_SETTINGS_DISPLAY_NAME          2702
//
// // --- Desktop / Features section ---
// #define IDC_SETTINGS_AUDIO                 2710
// #define IDC_SETTINGS_CLIPBOARD             2711
// #define IDC_SETTINGS_CURSOR_SHAPE          2712
// #define IDC_SETTINGS_CURSOR_POSITION      2713
//
// // --- Desktop / Appearance section ---
// #define IDC_SETTINGS_DISABLE_EFFECTS       2720
// #define IDC_SETTINGS_DISABLE_WALLPAPER     2721
//
// // --- Desktop / Other section ---
// #define IDC_SETTINGS_LOCK_AT_DISCONNECT    2730
// #define IDC_SETTINGS_BLOCK_REMOTE_INPUT    2731
//
// // --- Update section ---
// #define IDC_SETTINGS_CHECK_UPDATES         2740
// #define IDC_SETTINGS_CUSTOM_UPDATE_SERVER  2741
// #define IDC_SETTINGS_UPDATE_SERVER         2742
// #define IDC_SETTINGS_CHECK_FOR_UPDATES_NOW 2743
//
// // --- Logging section ---
// #define IDC_SETTINGS_LOG_ENABLED           2750
// #define IDC_SETTINGS_LOG_LEVEL             2751
// #define IDC_SETTINGS_LOG_PATH              2752
// #define IDC_SETTINGS_LOG_BROWSE            2753
//

#ifndef CLIENT_WIN32_SETTINGS_DIALOG_H
#define CLIENT_WIN32_SETTINGS_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class SettingsDialog
{
public:
    // Log verbosity selector. Values are stable enum-like ints so this header
    // does not need to depend on base/logging.
    enum LogLevel
    {
        kLogLevelInfo    = 0,
        kLogLevelWarning = 1,
        kLogLevelError   = 2,
        kLogLevelFatal   = 3,
    };

    // Visual theme selector (matches the .ui "Theme" combobox).
    enum Theme
    {
        kThemeSystem = 0,
        kThemeLight  = 1,
        kThemeDark   = 2,
    };

    struct Result
    {
        // --- General tab -------------------------------------------------
        // BCP-47 / Qt locale tag, e.g. L"en", L"ru", L"de". Empty selects
        // the system default.
        std::wstring language;
        int          theme        = kThemeSystem;
        // Free-form display name shown to the remote host (max 16 chars
        // in the .ui, enforced here too).
        std::wstring displayName;

        // --- Desktop tab / Features -------------------------------------
        bool enableAudio       = true;
        bool enableClipboard   = true;
        bool enableCursorShape = true;
        bool enableCursorPos   = false;

        // --- Desktop tab / Appearance -----------------------------------
        bool disableDesktopEffects   = false;
        bool disableDesktopWallpaper = false;

        // --- Desktop tab / Other ----------------------------------------
        bool lockAtDisconnect = false;
        bool blockRemoteInput = false;

        // --- Update tab --------------------------------------------------
        bool         checkUpdatesOnStartup = true;
        bool         useCustomUpdateServer = false;
        std::wstring updateServer;

        // --- Logging section (not tabbed in the .ui but a common client
        // setting; kept together in this dialog so the port stays in one
        // place). ---------------------------------------------------------
        bool         loggingEnabled = true;
        int          logLevel       = kLogLevelInfo;
        std::wstring logPath;
    };

    SettingsDialog(HINSTANCE instance, HWND parent);
    ~SettingsDialog() = default;

    SettingsDialog(const SettingsDialog&) = delete;
    SettingsDialog& operator=(const SettingsDialog&) = delete;

    // Pre-populate the dialog with an existing configuration.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void onCustomUpdateServerToggled(HWND hwnd);
    void onLoggingToggled(HWND hwnd);
    void onBrowseLogPath(HWND hwnd);
    void onCheckForUpdatesNow(HWND hwnd);

    void updateUpdateServerEnable(HWND hwnd);
    void updateLoggingEnables(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SETTINGS_DIALOG_H
