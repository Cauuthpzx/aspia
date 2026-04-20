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
// Win32 replacement for client/ui/desktop/desktop_config_dialog.ui.
//
// When wiring this dialog up, add the following to
// source/client_win32/resource.h (do NOT edit that file from this port;
// these are the identifiers the implementation below expects):
//
// #define IDD_DESKTOP_CONFIG                250
//
// #define IDC_DCFG_VIDEO_CODEC              2500
// #define IDC_DCFG_PIXEL_FORMAT             2501
// #define IDC_DCFG_COMPRESS_RATIO           2502
// #define IDC_DCFG_COMPRESS_RATIO_LABEL     2503
// #define IDC_DCFG_FPS                      2504
// #define IDC_DCFG_FPS_LABEL                2505
// #define IDC_DCFG_AUDIO                    2510
// #define IDC_DCFG_CLIPBOARD                2511
// #define IDC_DCFG_CURSOR_SHAPE             2512
// #define IDC_DCFG_CURSOR_POSITION          2513
// #define IDC_DCFG_DISABLE_EFFECTS          2514
// #define IDC_DCFG_DISABLE_WALLPAPER        2515
// #define IDC_DCFG_LOCK_AT_DISCONNECT       2516
// #define IDC_DCFG_BLOCK_REMOTE_INPUT       2517
//

#ifndef CLIENT_WIN32_DESKTOP_CONFIG_DIALOG_H
#define CLIENT_WIN32_DESKTOP_CONFIG_DIALOG_H

#include <windows.h>

namespace aspia::client_win32 {

class DesktopConfigDialog
{
public:
    // Video codec selector. Values are stable enum-like ints so this header
    // does not need to depend on the proto definitions.
    enum VideoCodec
    {
        kVideoCodecVp8    = 0,
        kVideoCodecVp9    = 1,
        kVideoCodecZstd   = 2,
    };

    // Pixel format selector (only meaningful for the ZSTD codec in practice,
    // ignored by the VPx codecs, but always present on the dialog).
    enum PixelFormat
    {
        kPixelFormatArgb   = 0,
        kPixelFormatRgb565 = 1,
    };

    struct Result
    {
        // Video stream settings.
        int videoCodec     = kVideoCodecVp9;
        int pixelFormat    = kPixelFormatArgb;
        int compressRatio  = 6;    // 1..9, used when codec == ZSTD
        int fps            = 30;   // 1..60

        // Feature toggles (match the .ui groupbox "Features").
        bool enableAudio        = true;
        bool enableClipboard    = true;
        bool enableCursorShape  = true;
        bool enableCursorPos    = false;

        // Appearance toggles (match the .ui groupbox "Appearance").
        bool disableDesktopEffects   = false;
        bool disableDesktopWallpaper = false;

        // Other toggles (match the .ui groupbox "Other").
        bool lockAtDisconnect = false;
        bool blockRemoteInput = false;
    };

    DesktopConfigDialog(HINSTANCE instance, HWND parent);
    ~DesktopConfigDialog() = default;

    DesktopConfigDialog(const DesktopConfigDialog&) = delete;
    DesktopConfigDialog& operator=(const DesktopConfigDialog&) = delete;

    // Pre-populate the dialog with an existing configuration.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onVideoCodecChanged(HWND hwnd);
    void onCompressRatioChanged(HWND hwnd);
    void onFpsChanged(HWND hwnd);
    void onOk(HWND hwnd);

    void updateCompressRatioLabel(HWND hwnd, int value);
    void updateFpsLabel(HWND hwnd, int value);
    void updateCodecDependentEnables(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_DESKTOP_CONFIG_DIALOG_H
