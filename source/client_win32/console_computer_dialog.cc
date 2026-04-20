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
 * ---------------------------------------------------------------------------
 * Win32 DIALOGEX template for this dialog. The resource-file owner should
 * paste this into client.rc (do NOT edit client.rc from here). The control
 * IDs match the #defines listed at the top of console_computer_dialog.h.
 *
 * The Qt original (console/computer_dialog.ui) is a splitter with a tree
 * switching between three QWidget pages. A plain Win32 DIALOG has no
 * equivalent of that splitter/tree navigation in this project's style, so
 * the three pages are flattened into titled GROUPBOXes arranged
 * vertically -- same pattern as settings_dialog.cc.
 *
 * IDD_CONSOLE_COMPUTER DIALOGEX 0, 0, 360, 400
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Computer Properties"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     // -- General group ---------------------------------------------------
 *     GROUPBOX        "General",                            -1,
 *                     7,   4, 346,  94
 *     LTEXT           "Name:",                              -1,
 *                     14,  16,  70,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_NAME,
 *                     90,  14, 256,  12, ES_AUTOHSCROLL
 *     LTEXT           "Parent Group:",                      -1,
 *                     14,  32,  70,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_PARENT_NAME,
 *                     90,  30, 256,  12, ES_AUTOHSCROLL | ES_READONLY
 *     LTEXT           "Address / ID:",                      -1,
 *                     14,  48,  70,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_ADDRESS,
 *                     90,  46, 256,  12, ES_AUTOHSCROLL
 *     AUTOCHECKBOX    "Inherit credentials from parent",
 *                                                           IDC_CONSOLE_COMPUTER_INHERIT_CREDS,
 *                     14,  64, 256,  10
 *     LTEXT           "User Name:",                         -1,
 *                     22,  78,  62,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_USERNAME,
 *                     90,  76, 256,  12, ES_AUTOHSCROLL
 *     LTEXT           "Password:",                          -1,
 *                     22,  92,  62,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_PASSWORD,
 *                     90,  90, 240,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",
 *                                                           IDC_CONSOLE_COMPUTER_SHOW_PASSWORD,
 *                     333, 89,  13,  14
 *
 *     LTEXT           "Comment:",                           -1,
 *                     14, 108,  70,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_COMMENT,
 *                     14, 120, 332,  48,
 *                     ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL
 *
 *     // -- Desktop session group -------------------------------------------
 *     GROUPBOX        "Desktop session",                    -1,
 *                     7, 174, 346, 158
 *     AUTOCHECKBOX    "Inherit configuration from parent",
 *                                                           IDC_CONSOLE_COMPUTER_INHERIT_CONFIG,
 *                     14, 186, 332,  10
 *
 *     GROUPBOX        "Features",                           -1,
 *                     14, 200, 332,  56
 *     AUTOCHECKBOX    "Enable audio",
 *                                                           IDC_CONSOLE_COMPUTER_AUDIO,
 *                     22, 212, 318,  10
 *     AUTOCHECKBOX    "Enable clipboard",
 *                                                           IDC_CONSOLE_COMPUTER_CLIPBOARD,
 *                     22, 224, 318,  10
 *     AUTOCHECKBOX    "Show shape of remote cursor",
 *                                                           IDC_CONSOLE_COMPUTER_CURSOR_SHAPE,
 *                     22, 236, 318,  10
 *     AUTOCHECKBOX    "Show position of remote cursor",
 *                                                           IDC_CONSOLE_COMPUTER_CURSOR_POSITION,
 *                     22, 248, 318,  10
 *
 *     GROUPBOX        "Appearance",                         -1,
 *                     14, 260, 160,  32
 *     AUTOCHECKBOX    "Disable desktop effects",
 *                                                           IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS,
 *                     22, 270, 148,  10
 *     AUTOCHECKBOX    "Disable desktop wallpaper",
 *                                                           IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER,
 *                     22, 282, 148,  10
 *
 *     GROUPBOX        "Other",                              -1,
 *                     180, 260, 166,  32
 *     AUTOCHECKBOX    "Lock computer at disconnect",
 *                                                           IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT,
 *                     188, 270, 152,  10
 *     AUTOCHECKBOX    "Block remote input",
 *                                                           IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT,
 *                     188, 282, 152,  10
 *
 *     // -- Parent group ----------------------------------------------------
 *     GROUPBOX        "Parent",                             -1,
 *                     7, 338, 346,  30
 *     LTEXT           "Path:",                              -1,
 *                     14, 350,  24,   9
 *     EDITTEXT        IDC_CONSOLE_COMPUTER_PARENT_PATH,
 *                     44, 348, 302,  12, ES_AUTOHSCROLL | ES_READONLY
 *
 *     // -- Dialog buttons --------------------------------------------------
 *     DEFPUSHBUTTON   "OK",                                 IDOK,
 *                     246, 378,  50,  14
 *     PUSHBUTTON      "Cancel",                             IDCANCEL,
 *                     303, 378,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/console_computer_dialog.h"

#include "client_win32/resource.h"

#include <cwchar>

namespace aspia::client_win32 {

namespace {

constexpr int kNameMaxLength     = 64;
constexpr int kAddressMaxLength  = 64;
constexpr int kUsernameMaxLength = 64;
constexpr int kPasswordMaxLength = 64;
constexpr int kCommentMaxLength  = 2048;

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

// Splits "host:port" into a host string and a port. If no ':' is present the
// whole string is treated as the host and 0 is returned for the port.
void splitAddress(const std::wstring& full, std::wstring* host, uint16_t* port)
{
    const size_t colon = full.find_last_of(L':');
    if (colon == std::wstring::npos)
    {
        *host = full;
        *port = 0;
        return;
    }

    *host = full.substr(0, colon);

    const wchar_t* begin = full.c_str() + colon + 1;
    wchar_t* end = nullptr;
    const unsigned long value = std::wcstoul(begin, &end, 10);
    if (end == begin || value > 0xFFFFu)
        *port = 0;
    else
        *port = static_cast<uint16_t>(value);
}

// Joins a host and a port back into "host:port" form for display. If the port
// is zero, only the host is used.
std::wstring joinAddress(const std::wstring& host, uint16_t port)
{
    if (port == 0)
        return host;

    wchar_t buf[16];
    std::swprintf(buf, sizeof(buf) / sizeof(buf[0]), L":%u",
                  static_cast<unsigned int>(port));
    return host + buf;
}

}  // namespace

ConsoleComputerDialog::ConsoleComputerDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool ConsoleComputerDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_COMPUTER), parent_,
        &ConsoleComputerDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleComputerDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleComputerDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleComputerDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleComputerDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleComputerDialog::handleMessage(
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

                case IDC_CONSOLE_COMPUTER_INHERIT_CREDS:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onInheritCredsToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_CONSOLE_COMPUTER_INHERIT_CONFIG:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onInheritConfigToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_CONSOLE_COMPUTER_SHOW_PASSWORD:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        togglePasswordVisibility(hwnd);
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

void ConsoleComputerDialog::onInitDialog(HWND hwnd)
{
    // -- General ------------------------------------------------------
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_NAME,
                        EM_LIMITTEXT, kNameMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_PARENT_NAME,
                        EM_LIMITTEXT, kNameMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_ADDRESS,
                        EM_LIMITTEXT, kAddressMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_USERNAME,
                        EM_LIMITTEXT, kUsernameMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_PASSWORD,
                        EM_LIMITTEXT, kPasswordMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_COMPUTER_COMMENT,
                        EM_LIMITTEXT, kCommentMaxLength, 0);

    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_NAME,
                    result_.general.name.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_PARENT_NAME,
                    result_.general.parentName.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_ADDRESS,
                    joinAddress(result_.general.address,
                                result_.general.port).c_str());

    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CREDS,
                   result_.general.inheritCreds ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_USERNAME,
                    result_.general.username.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_PASSWORD,
                    result_.general.password.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_COMMENT,
                    result_.general.comment.c_str());

    // -- Desktop ------------------------------------------------------
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CONFIG,
                   result_.desktop.inheritConfig ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_AUDIO,
                   result_.desktop.enableAudio ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_CLIPBOARD,
                   result_.desktop.enableClipboard ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_SHAPE,
                   result_.desktop.enableCursorShape ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_POSITION,
                   result_.desktop.enableCursorPos ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS,
                   result_.desktop.disableDesktopEffects ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER,
                   result_.desktop.disableDesktopWallpaper ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT,
                   result_.desktop.lockAtDisconnect ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT,
                   result_.desktop.blockRemoteInput ? BST_CHECKED : BST_UNCHECKED);

    // -- Parent -------------------------------------------------------
    SetDlgItemTextW(hwnd, IDC_CONSOLE_COMPUTER_PARENT_PATH,
                    result_.parent.path.c_str());

    updateInheritCredsEnables(hwnd);
    updateInheritConfigEnables(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_NAME));
}

void ConsoleComputerDialog::onOk(HWND hwnd)
{
    // -- General ------------------------------------------------------
    result_.general.name =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_NAME));
    result_.general.parentName =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_PARENT_NAME));

    const std::wstring address_raw =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_ADDRESS));
    splitAddress(address_raw, &result_.general.address, &result_.general.port);

    result_.general.inheritCreds =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CREDS) == BST_CHECKED;
    result_.general.username =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_USERNAME));
    result_.general.password =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_PASSWORD));
    result_.general.comment =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_COMMENT));

    // -- Desktop ------------------------------------------------------
    result_.desktop.inheritConfig =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CONFIG) == BST_CHECKED;
    result_.desktop.enableAudio =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_AUDIO) == BST_CHECKED;
    result_.desktop.enableClipboard =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_CLIPBOARD) == BST_CHECKED;
    result_.desktop.enableCursorShape =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_SHAPE) == BST_CHECKED;
    result_.desktop.enableCursorPos =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_POSITION) == BST_CHECKED;
    result_.desktop.disableDesktopEffects =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS) == BST_CHECKED;
    result_.desktop.disableDesktopWallpaper =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER) == BST_CHECKED;
    result_.desktop.lockAtDisconnect =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT) == BST_CHECKED;
    result_.desktop.blockRemoteInput =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT) == BST_CHECKED;

    // -- Parent -------------------------------------------------------
    // parent.path is a read-only display field; preserved as-is from
    // setInitial(). No need to re-read it here.
}

void ConsoleComputerDialog::onInheritCredsToggled(HWND hwnd)
{
    updateInheritCredsEnables(hwnd);
}

void ConsoleComputerDialog::onInheritConfigToggled(HWND hwnd)
{
    updateInheritConfigEnables(hwnd);
}

void ConsoleComputerDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_PASSWORD);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

void ConsoleComputerDialog::updateInheritCredsEnables(HWND hwnd)
{
    // In the Qt .ui the groupbox is "checkable" and checked means
    // "inherit" -- i.e. the inner fields are the parent's. We treat that
    // the same way: if checked, the per-entry username/password fields
    // are disabled because they are sourced from the parent.
    const BOOL inherit =
        (IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CREDS)
             == BST_CHECKED) ? TRUE : FALSE;
    const BOOL editable = inherit ? FALSE : TRUE;

    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_USERNAME),      editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_PASSWORD),      editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_SHOW_PASSWORD), editable);
}

void ConsoleComputerDialog::updateInheritConfigEnables(HWND hwnd)
{
    const BOOL inherit =
        (IsDlgButtonChecked(hwnd, IDC_CONSOLE_COMPUTER_INHERIT_CONFIG)
             == BST_CHECKED) ? TRUE : FALSE;
    const BOOL editable = inherit ? FALSE : TRUE;

    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_AUDIO),              editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_CLIPBOARD),          editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_SHAPE),       editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_CURSOR_POSITION),    editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS),    editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER),  editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT), editable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT), editable);
}

}  // namespace aspia::client_win32
