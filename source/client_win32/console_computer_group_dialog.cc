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
 * paste this into client.rc (do NOT edit client.rc from here).
 *
 * The original .ui file uses a QTabWidget with two tabs ("General" and
 * "Default Configuration"). In this Win32 port the two tabs are flattened
 * onto a single dialog: the General fields occupy the top strip and the
 * "Default Configuration" controls appear below, grouped the same way they
 * are on the original tab.
 *
 * IDD_CONSOLE_COMPUTER_GROUP DIALOGEX 0, 0, 320, 300
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Computer Group Properties"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     // -- General ---------------------------------------------------------
 *     LTEXT           "Name:",
 *                     IDC_CONSOLE_CG_LABEL_NAME,                  7,   9,  60,   9
 *     EDITTEXT        IDC_CONSOLE_CG_NAME,                       70,   7, 243,  12,
 *                     ES_AUTOHSCROLL
 *     LTEXT           "Parent Group:",
 *                     IDC_CONSOLE_CG_LABEL_PARENT,                7,  25,  60,   9
 *     EDITTEXT        IDC_CONSOLE_CG_PARENT,                     70,  23, 243,  12,
 *                     ES_AUTOHSCROLL | ES_READONLY
 *     LTEXT           "Comment:",
 *                     IDC_CONSOLE_CG_LABEL_COMMENT,               7,  41, 246,   9
 *     EDITTEXT        IDC_CONSOLE_CG_COMMENT,                     7,  53, 306,  40,
 *                     ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE |
 *                     ES_WANTRETURN | WS_VSCROLL
 *
 *     // -- Default Configuration ------------------------------------------
 *     AUTOCHECKBOX    "Inherit configuration from parent",
 *                     IDC_CONSOLE_CG_INHERIT_CONFIG,              7,  99, 306,  10
 *
 *     GROUPBOX        "Features",
 *                     IDC_CONSOLE_CG_GB_FEATURES,                 7, 114, 150,  76
 *     AUTOCHECKBOX    "Enable audio",
 *                     IDC_CONSOLE_CG_AUDIO,                      14, 126, 140,  10
 *     AUTOCHECKBOX    "Enable clipboard",
 *                     IDC_CONSOLE_CG_CLIPBOARD,                  14, 140, 140,  10
 *     AUTOCHECKBOX    "Show shape of remote cursor",
 *                     IDC_CONSOLE_CG_CURSOR_SHAPE,               14, 154, 140,  10
 *     AUTOCHECKBOX    "Show position of remote cursor",
 *                     IDC_CONSOLE_CG_CURSOR_POSITION,            14, 168, 140,  10
 *
 *     GROUPBOX        "Appearance",
 *                     IDC_CONSOLE_CG_GB_APPEARANCE,             163, 114, 150,  48
 *     AUTOCHECKBOX    "Disable desktop effects",
 *                     IDC_CONSOLE_CG_DISABLE_EFFECTS,           170, 126, 140,  10
 *     AUTOCHECKBOX    "Disable desktop wallpaper",
 *                     IDC_CONSOLE_CG_DISABLE_WALLPAPER,         170, 140, 140,  10
 *
 *     GROUPBOX        "Other",
 *                     IDC_CONSOLE_CG_GB_OTHER,                  163, 164, 150,  40
 *     AUTOCHECKBOX    "Lock computer at disconnect",
 *                     IDC_CONSOLE_CG_LOCK_AT_DISCONNECT,        170, 176, 140,  10
 *     AUTOCHECKBOX    "Block remote input",
 *                     IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT,        170, 190, 140,  10
 *
 *     DEFPUSHBUTTON   "OK",            IDOK,                    208, 280,  50,  14
 *     PUSHBUTTON      "Cancel",        IDCANCEL,                263, 280,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/console_computer_group_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

void setCheck(HWND hwnd, int id, bool checked)
{
    CheckDlgButton(hwnd, id, checked ? BST_CHECKED : BST_UNCHECKED);
}

bool isChecked(HWND hwnd, int id)
{
    return IsDlgButtonChecked(hwnd, id) == BST_CHECKED;
}

}  // namespace

ConsoleComputerGroupDialog::ConsoleComputerGroupDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool ConsoleComputerGroupDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_COMPUTER_GROUP), parent_,
        &ConsoleComputerGroupDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleComputerGroupDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleComputerGroupDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleComputerGroupDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleComputerGroupDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleComputerGroupDialog::handleMessage(
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

                case IDC_CONSOLE_CG_INHERIT_CONFIG:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onInheritConfigChanged(hwnd);
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

void ConsoleComputerGroupDialog::onInitDialog(HWND hwnd)
{
    // Mirror the maxLength limits set on the .ui line edits.
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_CG_NAME,    EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_CG_PARENT,  EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_CG_COMMENT, EM_LIMITTEXT, 2048, 0);

    // -- General tab --
    SetDlgItemTextW(hwnd, IDC_CONSOLE_CG_NAME,    result_.name.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_CG_PARENT,  result_.parent_group.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_CG_COMMENT, result_.comment.c_str());

    // -- Default Configuration tab --
    setCheck(hwnd, IDC_CONSOLE_CG_INHERIT_CONFIG,     result_.inherit_config);

    setCheck(hwnd, IDC_CONSOLE_CG_AUDIO,              result_.enable_audio);
    setCheck(hwnd, IDC_CONSOLE_CG_CLIPBOARD,          result_.enable_clipboard);
    setCheck(hwnd, IDC_CONSOLE_CG_CURSOR_SHAPE,       result_.enable_cursor_shape);
    setCheck(hwnd, IDC_CONSOLE_CG_CURSOR_POSITION,    result_.enable_cursor_position);

    setCheck(hwnd, IDC_CONSOLE_CG_DISABLE_EFFECTS,    result_.disable_desktop_effects);
    setCheck(hwnd, IDC_CONSOLE_CG_DISABLE_WALLPAPER,  result_.disable_desktop_wallpaper);

    setCheck(hwnd, IDC_CONSOLE_CG_LOCK_AT_DISCONNECT, result_.lock_at_disconnect);
    setCheck(hwnd, IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT, result_.block_remote_input);

    updateConfigEnableState(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_CG_NAME));
}

void ConsoleComputerGroupDialog::onInheritConfigChanged(HWND hwnd)
{
    updateConfigEnableState(hwnd);
}

void ConsoleComputerGroupDialog::updateConfigEnableState(HWND hwnd)
{
    // When "Inherit configuration from parent" is checked, grey out all the
    // per-feature/appearance/other overrides — they would have no effect.
    const BOOL enable = isChecked(hwnd, IDC_CONSOLE_CG_INHERIT_CONFIG) ? FALSE : TRUE;

    static const int kDependentIds[] = {
        IDC_CONSOLE_CG_GB_FEATURES,
        IDC_CONSOLE_CG_AUDIO,
        IDC_CONSOLE_CG_CLIPBOARD,
        IDC_CONSOLE_CG_CURSOR_SHAPE,
        IDC_CONSOLE_CG_CURSOR_POSITION,
        IDC_CONSOLE_CG_GB_APPEARANCE,
        IDC_CONSOLE_CG_DISABLE_EFFECTS,
        IDC_CONSOLE_CG_DISABLE_WALLPAPER,
        IDC_CONSOLE_CG_GB_OTHER,
        IDC_CONSOLE_CG_LOCK_AT_DISCONNECT,
        IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT,
    };

    for (int id : kDependentIds)
    {
        HWND ctrl = GetDlgItem(hwnd, id);
        if (ctrl)
            EnableWindow(ctrl, enable);
    }
}

void ConsoleComputerGroupDialog::onOk(HWND hwnd)
{
    // -- General tab --
    result_.name         = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_CG_NAME));
    result_.parent_group = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_CG_PARENT));
    result_.comment      = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_CG_COMMENT));

    // -- Default Configuration tab --
    result_.inherit_config            = isChecked(hwnd, IDC_CONSOLE_CG_INHERIT_CONFIG);

    result_.enable_audio              = isChecked(hwnd, IDC_CONSOLE_CG_AUDIO);
    result_.enable_clipboard          = isChecked(hwnd, IDC_CONSOLE_CG_CLIPBOARD);
    result_.enable_cursor_shape       = isChecked(hwnd, IDC_CONSOLE_CG_CURSOR_SHAPE);
    result_.enable_cursor_position    = isChecked(hwnd, IDC_CONSOLE_CG_CURSOR_POSITION);

    result_.disable_desktop_effects   = isChecked(hwnd, IDC_CONSOLE_CG_DISABLE_EFFECTS);
    result_.disable_desktop_wallpaper = isChecked(hwnd, IDC_CONSOLE_CG_DISABLE_WALLPAPER);

    result_.lock_at_disconnect        = isChecked(hwnd, IDC_CONSOLE_CG_LOCK_AT_DISCONNECT);
    result_.block_remote_input        = isChecked(hwnd, IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT);
}

}  // namespace aspia::client_win32
