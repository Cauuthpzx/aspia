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
 * listed at the top of settings_dialog.h.
 *
 * The .ui uses a QTabWidget with four tabs. A plain Win32 DIALOG does not
 * have a tab control by default, so we flatten the layout into titled
 * GROUPBOXes arranged vertically. Every control that existed in the .ui
 * is represented here.
 *
 * IDD_SETTINGS DIALOGEX 0, 0, 320, 300
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Settings"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     // -- General group --------------------------------------------------
 *     GROUPBOX        "General",                -1,
 *                     7,   4, 306,  62
 *     LTEXT           "Language:",              -1,
 *                     14,  16,  80,   9
 *     COMBOBOX        IDC_SETTINGS_LANGUAGE,
 *                     100, 14, 206,  80,
 *                     CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Theme:",                 -1,
 *                     14,  32,  80,   9
 *     COMBOBOX        IDC_SETTINGS_THEME,
 *                     100, 30, 206,  60,
 *                     CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Display name when connected:", -1,
 *                     14,  48, 104,   9
 *     EDITTEXT        IDC_SETTINGS_DISPLAY_NAME,
 *                     120, 46, 186,  12, ES_AUTOHSCROLL
 *
 *     // -- Desktop features group -----------------------------------------
 *     GROUPBOX        "Desktop features",       -1,
 *                     7,  70, 306,  58
 *     AUTOCHECKBOX    "Enable audio",           IDC_SETTINGS_AUDIO,
 *                     14,  82, 292,  10
 *     AUTOCHECKBOX    "Enable clipboard",       IDC_SETTINGS_CLIPBOARD,
 *                     14,  94, 292,  10
 *     AUTOCHECKBOX    "Show shape of remote cursor",
 *                                               IDC_SETTINGS_CURSOR_SHAPE,
 *                     14, 106, 292,  10
 *     AUTOCHECKBOX    "Show position of remote cursor",
 *                                               IDC_SETTINGS_CURSOR_POSITION,
 *                     14, 118, 292,  10
 *
 *     // -- Desktop appearance / other group -------------------------------
 *     GROUPBOX        "Desktop (appearance and other)", -1,
 *                     7, 132, 306,  58
 *     AUTOCHECKBOX    "Disable desktop effects",
 *                                               IDC_SETTINGS_DISABLE_EFFECTS,
 *                     14, 144, 292,  10
 *     AUTOCHECKBOX    "Disable desktop wallpaper",
 *                                               IDC_SETTINGS_DISABLE_WALLPAPER,
 *                     14, 156, 292,  10
 *     AUTOCHECKBOX    "Lock computer at disconnect",
 *                                               IDC_SETTINGS_LOCK_AT_DISCONNECT,
 *                     14, 168, 292,  10
 *     AUTOCHECKBOX    "Block remote input",     IDC_SETTINGS_BLOCK_REMOTE_INPUT,
 *                     14, 180, 292,  10
 *
 *     // -- Update group ---------------------------------------------------
 *     GROUPBOX        "Update",                 -1,
 *                     7, 194, 306,  52
 *     AUTOCHECKBOX    "Check for updates on startup",
 *                                               IDC_SETTINGS_CHECK_UPDATES,
 *                     14, 206, 180,  10
 *     PUSHBUTTON      "Check for updates",      IDC_SETTINGS_CHECK_FOR_UPDATES_NOW,
 *                     200, 204, 106,  14
 *     AUTOCHECKBOX    "Use custom update server",
 *                                               IDC_SETTINGS_CUSTOM_UPDATE_SERVER,
 *                     14, 220, 180,  10
 *     LTEXT           "Server:",                -1,
 *                     14, 234,  30,   9
 *     EDITTEXT        IDC_SETTINGS_UPDATE_SERVER,
 *                     50, 232, 256,  12, ES_AUTOHSCROLL
 *
 *     // -- Logging group --------------------------------------------------
 *     GROUPBOX        "Logging",                -1,
 *                     7, 250, 306,  30
 *     AUTOCHECKBOX    "Enable",                 IDC_SETTINGS_LOG_ENABLED,
 *                     14, 262,  52,  10
 *     COMBOBOX        IDC_SETTINGS_LOG_LEVEL,
 *                     70, 260,  70,  60,
 *                     CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     EDITTEXT        IDC_SETTINGS_LOG_PATH,
 *                     144, 260, 142,  12, ES_AUTOHSCROLL
 *     PUSHBUTTON      "...",                    IDC_SETTINGS_LOG_BROWSE,
 *                     290, 260,  16,  12
 *
 *     // -- Dialog buttons -------------------------------------------------
 *     DEFPUSHBUTTON   "OK",                     IDOK,
 *                     206, 282,  50,  14
 *     PUSHBUTTON      "Cancel",                 IDCANCEL,
 *                     263, 282,  50,  14
 * END
 * ------------------------------------------------------------------------
 */

#include "client_win32/settings_dialog.h"

#include "client_win32/resource.h"

#include <shlobj.h>

#include <array>

namespace aspia::client_win32 {

namespace {

constexpr int kDisplayNameMaxLength = 16;
constexpr int kUpdateServerMaxLength = 255;
constexpr int kLogPathMaxLength = 512;

struct LanguageEntry
{
    const wchar_t* text;
    const wchar_t* tag;
};

// Matches the set of languages shipped in source/translations/.
constexpr std::array<LanguageEntry, 11> kLanguageEntries = {{
    { L"System default",      L"" },
    { L"English",              L"en" },
    { L"\u0420\u0443\u0441\u0441\u043A\u0438\u0439 (Russian)", L"ru" },
    { L"Deutsch (German)",     L"de" },
    { L"Espa\u00F1ol (Spanish)",L"es" },
    { L"Fran\u00E7ais (French)", L"fr" },
    { L"Italiano (Italian)",   L"it" },
    { L"Nederlands (Dutch)",   L"nl" },
    { L"Polski (Polish)",      L"pl" },
    { L"\u4E2D\u6587 (Chinese)", L"zh" },
    { L"\u65E5\u672C\u8A9E (Japanese)", L"ja" },
}};

struct IntComboEntry
{
    const wchar_t* text;
    int value;
};

constexpr std::array<IntComboEntry, 3> kThemeEntries = {{
    { L"System",  SettingsDialog::kThemeSystem },
    { L"Light",   SettingsDialog::kThemeLight },
    { L"Dark",    SettingsDialog::kThemeDark },
}};

constexpr std::array<IntComboEntry, 4> kLogLevelEntries = {{
    { L"Info",    SettingsDialog::kLogLevelInfo },
    { L"Warning", SettingsDialog::kLogLevelWarning },
    { L"Error",   SettingsDialog::kLogLevelError },
    { L"Fatal",   SettingsDialog::kLogLevelFatal },
}};

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

void populateIntCombo(HWND combo,
                      const IntComboEntry* entries,
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

int readIntComboValue(HWND combo, int fallback)
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

void populateLanguageCombo(HWND combo, const std::wstring& selected_tag)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    int selected_index = 0;
    for (size_t i = 0; i < kLanguageEntries.size(); ++i)
    {
        const LRESULT idx = SendMessageW(
            combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(kLanguageEntries[i].text));
        // Store the tag pointer as item data so we can read it back on OK.
        SendMessageW(combo, CB_SETITEMDATA, static_cast<WPARAM>(idx),
                     reinterpret_cast<LPARAM>(kLanguageEntries[i].tag));
        if (selected_tag == kLanguageEntries[i].tag)
            selected_index = static_cast<int>(idx);
    }
    SendMessageW(combo, CB_SETCURSEL,
                 static_cast<WPARAM>(selected_index), 0);
}

std::wstring readLanguageCombo(HWND combo, const std::wstring& fallback)
{
    const LRESULT cur = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (cur == CB_ERR)
        return fallback;
    const LRESULT data = SendMessageW(combo, CB_GETITEMDATA,
                                      static_cast<WPARAM>(cur), 0);
    if (data == CB_ERR || data == 0)
        return fallback;
    return std::wstring(reinterpret_cast<const wchar_t*>(data));
}

// Prompts the user for a folder and returns its path. Empty string on
// cancel/failure.
std::wstring browseForFolder(HWND owner, const std::wstring& initial)
{
    BROWSEINFOW bi = {};
    bi.hwndOwner = owner;
    bi.lpszTitle = L"Select a folder for log files";
    bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // Pass the currently selected folder as the starting point.
    bi.lParam = reinterpret_cast<LPARAM>(initial.c_str());
    bi.lpfn = [](HWND hwnd, UINT msg, LPARAM /*lp*/, LPARAM data) -> int
    {
        if (msg == BFFM_INITIALIZED && data)
        {
            SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, data);
        }
        return 0;
    };

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (!pidl)
        return {};

    wchar_t buffer[MAX_PATH] = {};
    std::wstring result;
    if (SHGetPathFromIDListW(pidl, buffer))
        result = buffer;

    CoTaskMemFree(pidl);
    return result;
}

}  // namespace

SettingsDialog::SettingsDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool SettingsDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_SETTINGS), parent_,
        &SettingsDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK SettingsDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SettingsDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<SettingsDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<SettingsDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR SettingsDialog::handleMessage(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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
                    onOk(hwnd);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDC_SETTINGS_CUSTOM_UPDATE_SERVER:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCustomUpdateServerToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_SETTINGS_LOG_ENABLED:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onLoggingToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_SETTINGS_LOG_BROWSE:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onBrowseLogPath(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_SETTINGS_CHECK_FOR_UPDATES_NOW:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCheckForUpdatesNow(hwnd);
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

void SettingsDialog::onInitDialog(HWND hwnd)
{
    // -- General -------------------------------------------------------
    populateLanguageCombo(GetDlgItem(hwnd, IDC_SETTINGS_LANGUAGE),
                          result_.language);

    populateIntCombo(GetDlgItem(hwnd, IDC_SETTINGS_THEME),
                     kThemeEntries.data(), kThemeEntries.size(),
                     result_.theme);

    SendDlgItemMessageW(hwnd, IDC_SETTINGS_DISPLAY_NAME, EM_LIMITTEXT,
                        kDisplayNameMaxLength, 0);
    SetDlgItemTextW(hwnd, IDC_SETTINGS_DISPLAY_NAME,
                    result_.displayName.c_str());

    // -- Desktop / Features -------------------------------------------
    CheckDlgButton(hwnd, IDC_SETTINGS_AUDIO,
                   result_.enableAudio ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_CLIPBOARD,
                   result_.enableClipboard ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_CURSOR_SHAPE,
                   result_.enableCursorShape ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_CURSOR_POSITION,
                   result_.enableCursorPos ? BST_CHECKED : BST_UNCHECKED);

    // -- Desktop / Appearance + Other ---------------------------------
    CheckDlgButton(hwnd, IDC_SETTINGS_DISABLE_EFFECTS,
                   result_.disableDesktopEffects ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_DISABLE_WALLPAPER,
                   result_.disableDesktopWallpaper ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_LOCK_AT_DISCONNECT,
                   result_.lockAtDisconnect ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_BLOCK_REMOTE_INPUT,
                   result_.blockRemoteInput ? BST_CHECKED : BST_UNCHECKED);

    // -- Update --------------------------------------------------------
    CheckDlgButton(hwnd, IDC_SETTINGS_CHECK_UPDATES,
                   result_.checkUpdatesOnStartup ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_SETTINGS_CUSTOM_UPDATE_SERVER,
                   result_.useCustomUpdateServer ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(hwnd, IDC_SETTINGS_UPDATE_SERVER, EM_LIMITTEXT,
                        kUpdateServerMaxLength, 0);
    SetDlgItemTextW(hwnd, IDC_SETTINGS_UPDATE_SERVER,
                    result_.updateServer.c_str());

    // -- Logging -------------------------------------------------------
    CheckDlgButton(hwnd, IDC_SETTINGS_LOG_ENABLED,
                   result_.loggingEnabled ? BST_CHECKED : BST_UNCHECKED);

    populateIntCombo(GetDlgItem(hwnd, IDC_SETTINGS_LOG_LEVEL),
                     kLogLevelEntries.data(), kLogLevelEntries.size(),
                     result_.logLevel);

    SendDlgItemMessageW(hwnd, IDC_SETTINGS_LOG_PATH, EM_LIMITTEXT,
                        kLogPathMaxLength, 0);
    SetDlgItemTextW(hwnd, IDC_SETTINGS_LOG_PATH, result_.logPath.c_str());

    updateUpdateServerEnable(hwnd);
    updateLoggingEnables(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_SETTINGS_LANGUAGE));
}

void SettingsDialog::onOk(HWND hwnd)
{
    // -- General -------------------------------------------------------
    result_.language = readLanguageCombo(
        GetDlgItem(hwnd, IDC_SETTINGS_LANGUAGE), result_.language);
    result_.theme = readIntComboValue(
        GetDlgItem(hwnd, IDC_SETTINGS_THEME), result_.theme);
    result_.displayName =
        readEditText(GetDlgItem(hwnd, IDC_SETTINGS_DISPLAY_NAME));

    // -- Desktop / Features -------------------------------------------
    result_.enableAudio =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_AUDIO) == BST_CHECKED;
    result_.enableClipboard =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_CLIPBOARD) == BST_CHECKED;
    result_.enableCursorShape =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_CURSOR_SHAPE) == BST_CHECKED;
    result_.enableCursorPos =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_CURSOR_POSITION) == BST_CHECKED;

    // -- Desktop / Appearance + Other ---------------------------------
    result_.disableDesktopEffects =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_DISABLE_EFFECTS) == BST_CHECKED;
    result_.disableDesktopWallpaper =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_DISABLE_WALLPAPER) == BST_CHECKED;
    result_.lockAtDisconnect =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_LOCK_AT_DISCONNECT) == BST_CHECKED;
    result_.blockRemoteInput =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_BLOCK_REMOTE_INPUT) == BST_CHECKED;

    // -- Update --------------------------------------------------------
    result_.checkUpdatesOnStartup =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_CHECK_UPDATES) == BST_CHECKED;
    result_.useCustomUpdateServer =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_CUSTOM_UPDATE_SERVER) == BST_CHECKED;
    result_.updateServer =
        readEditText(GetDlgItem(hwnd, IDC_SETTINGS_UPDATE_SERVER));

    // -- Logging -------------------------------------------------------
    result_.loggingEnabled =
        IsDlgButtonChecked(hwnd, IDC_SETTINGS_LOG_ENABLED) == BST_CHECKED;
    result_.logLevel = readIntComboValue(
        GetDlgItem(hwnd, IDC_SETTINGS_LOG_LEVEL), result_.logLevel);
    result_.logPath =
        readEditText(GetDlgItem(hwnd, IDC_SETTINGS_LOG_PATH));
}

void SettingsDialog::onCustomUpdateServerToggled(HWND hwnd)
{
    updateUpdateServerEnable(hwnd);
}

void SettingsDialog::onLoggingToggled(HWND hwnd)
{
    updateLoggingEnables(hwnd);
}

void SettingsDialog::onBrowseLogPath(HWND hwnd)
{
    const std::wstring current =
        readEditText(GetDlgItem(hwnd, IDC_SETTINGS_LOG_PATH));
    const std::wstring selected = browseForFolder(hwnd, current);
    if (!selected.empty())
    {
        SetDlgItemTextW(hwnd, IDC_SETTINGS_LOG_PATH, selected.c_str());
    }
}

void SettingsDialog::onCheckForUpdatesNow(HWND /*hwnd*/)
{
    // The actual update check is fired from the enclosing application
    // once the dialog returns IDOK; this stub exists so the button has a
    // handler and does not bubble as an accidental IDOK via accelerator
    // processing. No-op here.
}

void SettingsDialog::updateUpdateServerEnable(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd, IDC_SETTINGS_CUSTOM_UPDATE_SERVER)
             == BST_CHECKED) ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_SETTINGS_UPDATE_SERVER), enabled);
}

void SettingsDialog::updateLoggingEnables(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd, IDC_SETTINGS_LOG_ENABLED)
             == BST_CHECKED) ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_SETTINGS_LOG_LEVEL),  enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SETTINGS_LOG_PATH),   enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SETTINGS_LOG_BROWSE), enabled);
}

}  // namespace aspia::client_win32
