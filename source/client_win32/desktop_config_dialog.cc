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
 * ------------------------------------------------------------------------
 * Win32 DIALOGEX template for this dialog. Copy this verbatim into
 * client.rc (it is NOT in the build yet; we deliberately avoid editing
 * client.rc from inside this port). The control IDs match the #defines
 * listed at the top of desktop_config_dialog.h.
 *
 * IDD_DESKTOP_CONFIG DIALOGEX 0, 0, 320, 280
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Session Configuration"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     // -- Video stream group ---------------------------------------------
 *     GROUPBOX        "Video",                  -1,
 *                     7,   4, 306,  86
 *     LTEXT           "Codec:",                 -1,
 *                     14,  18,  60,   9
 *     COMBOBOX        IDC_DCFG_VIDEO_CODEC,
 *                     80,  16, 226,  60,
 *                     CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Color depth:",           -1,
 *                     14,  34,  60,   9
 *     COMBOBOX        IDC_DCFG_PIXEL_FORMAT,
 *                     80,  32, 226,  60,
 *                     CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Compression ratio:",     -1,
 *                     14,  50,  80,   9
 *     CONTROL         "",                       IDC_DCFG_COMPRESS_RATIO,
 *                     "msctls_trackbar32",
 *                     TBS_AUTOTICKS | TBS_HORZ | WS_TABSTOP,
 *                     96,  48, 184,  16
 *     LTEXT           "6",                      IDC_DCFG_COMPRESS_RATIO_LABEL,
 *                     284, 50,  22,   9
 *     LTEXT           "Frame rate (fps):",      -1,
 *                     14,  70,  80,   9
 *     CONTROL         "",                       IDC_DCFG_FPS,
 *                     "msctls_trackbar32",
 *                     TBS_AUTOTICKS | TBS_HORZ | WS_TABSTOP,
 *                     96,  68, 184,  16
 *     LTEXT           "30",                     IDC_DCFG_FPS_LABEL,
 *                     284, 70,  22,   9
 *
 *     // -- Features group -------------------------------------------------
 *     GROUPBOX        "Features",               -1,
 *                     7,  96, 306,  56
 *     AUTOCHECKBOX    "Enable audio",           IDC_DCFG_AUDIO,
 *                     14, 108, 292,  10
 *     AUTOCHECKBOX    "Enable clipboard",       IDC_DCFG_CLIPBOARD,
 *                     14, 120, 292,  10
 *     AUTOCHECKBOX    "Show shape of remote cursor",
 *                                               IDC_DCFG_CURSOR_SHAPE,
 *                     14, 132, 292,  10
 *     AUTOCHECKBOX    "Show position of remote cursor",
 *                                               IDC_DCFG_CURSOR_POSITION,
 *                     14, 144, 292,  10
 *
 *     // -- Appearance group -----------------------------------------------
 *     GROUPBOX        "Appearance",             -1,
 *                     7, 158, 306,  40
 *     AUTOCHECKBOX    "Disable desktop effects",
 *                                               IDC_DCFG_DISABLE_EFFECTS,
 *                     14, 170, 292,  10
 *     AUTOCHECKBOX    "Disable desktop wallpaper",
 *                                               IDC_DCFG_DISABLE_WALLPAPER,
 *                     14, 182, 292,  10
 *
 *     // -- Other group ----------------------------------------------------
 *     GROUPBOX        "Other",                  -1,
 *                     7, 204, 306,  40
 *     AUTOCHECKBOX    "Lock computer at disconnect",
 *                                               IDC_DCFG_LOCK_AT_DISCONNECT,
 *                     14, 216, 292,  10
 *     AUTOCHECKBOX    "Block remote input",     IDC_DCFG_BLOCK_REMOTE_INPUT,
 *                     14, 228, 292,  10
 *
 *     // -- Dialog buttons -------------------------------------------------
 *     DEFPUSHBUTTON   "OK",                     IDOK,
 *                     206, 258,  50,  14
 *     PUSHBUTTON      "Cancel",                 IDCANCEL,
 *                     263, 258,  50,  14
 * END
 * ------------------------------------------------------------------------
 */

#include "client_win32/desktop_config_dialog.h"

#include "client_win32/resource.h"

#include <commctrl.h>

#include <array>
#include <cwchar>

namespace aspia::client_win32 {

namespace {

constexpr int kCompressRatioMin = 1;
constexpr int kCompressRatioMax = 9;

constexpr int kFpsMin = 1;
constexpr int kFpsMax = 60;

struct ComboEntry
{
    const wchar_t* text;
    int value;
};

constexpr std::array<ComboEntry, 3> kCodecEntries = {{
    { L"VP9 (default)", DesktopConfigDialog::kVideoCodecVp9 },
    { L"VP8",           DesktopConfigDialog::kVideoCodecVp8 },
    { L"ZSTD",          DesktopConfigDialog::kVideoCodecZstd },
}};

constexpr std::array<ComboEntry, 2> kPixelFormatEntries = {{
    { L"True color (32 bit)",  DesktopConfigDialog::kPixelFormatArgb },
    { L"High color (16 bit)",  DesktopConfigDialog::kPixelFormatRgb565 },
}};

void populateCombo(HWND combo,
                   const ComboEntry* entries,
                   size_t count,
                   int selected_value)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    int selected_index = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const LRESULT idx = SendMessageW(
            combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(entries[i].text));
        SendMessageW(combo, CB_SETITEMDATA, static_cast<WPARAM>(idx),
                     static_cast<LPARAM>(entries[i].value));
        if (entries[i].value == selected_value)
            selected_index = static_cast<int>(idx);
    }
    SendMessageW(combo, CB_SETCURSEL,
                 static_cast<WPARAM>(selected_index), 0);
}

int readComboValue(HWND combo, int fallback)
{
    const LRESULT cur = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (cur == CB_ERR)
        return fallback;
    const LRESULT data = SendMessageW(combo, CB_GETITEMDATA,
                                      static_cast<WPARAM>(cur), 0);
    if (data == CB_ERR)
        return fallback;
    return static_cast<int>(data);
}

void setupTrackbar(HWND track, int min, int max, int value)
{
    SendMessageW(track, TBM_SETRANGE, TRUE,
                 MAKELPARAM(min, max));
    SendMessageW(track, TBM_SETPAGESIZE, 0, 1);
    SendMessageW(track, TBM_SETTICFREQ, 1, 0);
    SendMessageW(track, TBM_SETPOS, TRUE, value);
}

int clamp(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

}  // namespace

DesktopConfigDialog::DesktopConfigDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool DesktopConfigDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_DESKTOP_CONFIG), parent_,
        &DesktopConfigDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK DesktopConfigDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    DesktopConfigDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<DesktopConfigDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<DesktopConfigDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR DesktopConfigDialog::handleMessage(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            onInitDialog(hwnd);
            return TRUE;

        case WM_HSCROLL:
        {
            const HWND source = reinterpret_cast<HWND>(lp);
            if (source == GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO))
            {
                onCompressRatioChanged(hwnd);
                return TRUE;
            }
            if (source == GetDlgItem(hwnd, IDC_DCFG_FPS))
            {
                onFpsChanged(hwnd);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wp))
            {
                case IDOK:
                    onOk(hwnd);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDC_DCFG_VIDEO_CODEC:
                    if (HIWORD(wp) == CBN_SELCHANGE)
                    {
                        onVideoCodecChanged(hwnd);
                        return TRUE;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void DesktopConfigDialog::onInitDialog(HWND hwnd)
{
    // Make sure the trackbar window class is registered for this process.
    // Safe to call more than once.
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    populateCombo(GetDlgItem(hwnd, IDC_DCFG_VIDEO_CODEC),
                  kCodecEntries.data(), kCodecEntries.size(),
                  result_.videoCodec);

    populateCombo(GetDlgItem(hwnd, IDC_DCFG_PIXEL_FORMAT),
                  kPixelFormatEntries.data(), kPixelFormatEntries.size(),
                  result_.pixelFormat);

    const int compress_ratio =
        clamp(result_.compressRatio, kCompressRatioMin, kCompressRatioMax);
    const int fps = clamp(result_.fps, kFpsMin, kFpsMax);

    setupTrackbar(GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO),
                  kCompressRatioMin, kCompressRatioMax, compress_ratio);
    setupTrackbar(GetDlgItem(hwnd, IDC_DCFG_FPS),
                  kFpsMin, kFpsMax, fps);

    updateCompressRatioLabel(hwnd, compress_ratio);
    updateFpsLabel(hwnd, fps);

    CheckDlgButton(hwnd, IDC_DCFG_AUDIO,
                   result_.enableAudio ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_CLIPBOARD,
                   result_.enableClipboard ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_CURSOR_SHAPE,
                   result_.enableCursorShape ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_CURSOR_POSITION,
                   result_.enableCursorPos ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_DISABLE_EFFECTS,
                   result_.disableDesktopEffects ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_DISABLE_WALLPAPER,
                   result_.disableDesktopWallpaper ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_LOCK_AT_DISCONNECT,
                   result_.lockAtDisconnect ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DCFG_BLOCK_REMOTE_INPUT,
                   result_.blockRemoteInput ? BST_CHECKED : BST_UNCHECKED);

    updateCodecDependentEnables(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_DCFG_VIDEO_CODEC));
}

void DesktopConfigDialog::onVideoCodecChanged(HWND hwnd)
{
    updateCodecDependentEnables(hwnd);
}

void DesktopConfigDialog::onCompressRatioChanged(HWND hwnd)
{
    const int value = static_cast<int>(SendMessageW(
        GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO), TBM_GETPOS, 0, 0));
    updateCompressRatioLabel(hwnd, value);
}

void DesktopConfigDialog::onFpsChanged(HWND hwnd)
{
    const int value = static_cast<int>(SendMessageW(
        GetDlgItem(hwnd, IDC_DCFG_FPS), TBM_GETPOS, 0, 0));
    updateFpsLabel(hwnd, value);
}

void DesktopConfigDialog::onOk(HWND hwnd)
{
    result_.videoCodec = readComboValue(
        GetDlgItem(hwnd, IDC_DCFG_VIDEO_CODEC), result_.videoCodec);
    result_.pixelFormat = readComboValue(
        GetDlgItem(hwnd, IDC_DCFG_PIXEL_FORMAT), result_.pixelFormat);

    result_.compressRatio = static_cast<int>(SendMessageW(
        GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO), TBM_GETPOS, 0, 0));
    result_.fps = static_cast<int>(SendMessageW(
        GetDlgItem(hwnd, IDC_DCFG_FPS), TBM_GETPOS, 0, 0));

    result_.enableAudio =
        IsDlgButtonChecked(hwnd, IDC_DCFG_AUDIO) == BST_CHECKED;
    result_.enableClipboard =
        IsDlgButtonChecked(hwnd, IDC_DCFG_CLIPBOARD) == BST_CHECKED;
    result_.enableCursorShape =
        IsDlgButtonChecked(hwnd, IDC_DCFG_CURSOR_SHAPE) == BST_CHECKED;
    result_.enableCursorPos =
        IsDlgButtonChecked(hwnd, IDC_DCFG_CURSOR_POSITION) == BST_CHECKED;
    result_.disableDesktopEffects =
        IsDlgButtonChecked(hwnd, IDC_DCFG_DISABLE_EFFECTS) == BST_CHECKED;
    result_.disableDesktopWallpaper =
        IsDlgButtonChecked(hwnd, IDC_DCFG_DISABLE_WALLPAPER) == BST_CHECKED;
    result_.lockAtDisconnect =
        IsDlgButtonChecked(hwnd, IDC_DCFG_LOCK_AT_DISCONNECT) == BST_CHECKED;
    result_.blockRemoteInput =
        IsDlgButtonChecked(hwnd, IDC_DCFG_BLOCK_REMOTE_INPUT) == BST_CHECKED;
}

void DesktopConfigDialog::updateCompressRatioLabel(HWND hwnd, int value)
{
    wchar_t buffer[16] = {};
    std::swprintf(buffer, _countof(buffer), L"%d", value);
    SetDlgItemTextW(hwnd, IDC_DCFG_COMPRESS_RATIO_LABEL, buffer);
}

void DesktopConfigDialog::updateFpsLabel(HWND hwnd, int value)
{
    wchar_t buffer[16] = {};
    std::swprintf(buffer, _countof(buffer), L"%d", value);
    SetDlgItemTextW(hwnd, IDC_DCFG_FPS_LABEL, buffer);
}

void DesktopConfigDialog::updateCodecDependentEnables(HWND hwnd)
{
    const int codec = readComboValue(
        GetDlgItem(hwnd, IDC_DCFG_VIDEO_CODEC), result_.videoCodec);

    // Pixel format and compression ratio only apply to the ZSTD codec.
    const BOOL zstd_enabled = (codec == kVideoCodecZstd) ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_DCFG_PIXEL_FORMAT), zstd_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO), zstd_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DCFG_COMPRESS_RATIO_LABEL), zstd_enabled);
}

}  // namespace aspia::client_win32
