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
 * paste this into client.rc (do NOT edit client.rc from here):
 *
 * IDD_LOCAL_GROUP DIALOGEX 0, 0, 260, 160
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Group"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     LTEXT           "Parent Group:", -1,                          7,   9,  60,   9
 *     COMBOBOX        IDC_LOCAL_GROUP_PARENT,                      70,   7, 183,  60,
 *                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Name:",         -1,                          7,  25,  60,   9
 *     EDITTEXT        IDC_LOCAL_GROUP_NAME,                        70,  23, 183,  12, ES_AUTOHSCROLL
 *     LTEXT           "Comment:",      -1,                          7,  42, 246,   9
 *     EDITTEXT        IDC_LOCAL_GROUP_COMMENT,                      7,  54, 246,  84,
 *                     ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL
 *     DEFPUSHBUTTON   "OK",            IDOK,                       148, 140,  50,  14
 *     PUSHBUTTON      "Cancel",        IDCANCEL,                   203, 140,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/local_group_dialog.h"

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

}  // namespace

LocalGroupDialog::LocalGroupDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool LocalGroupDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_LOCAL_GROUP), parent_,
        &LocalGroupDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK LocalGroupDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    LocalGroupDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<LocalGroupDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<LocalGroupDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR LocalGroupDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void LocalGroupDialog::onInitDialog(HWND hwnd)
{
    // Mirror reasonable max-length limits for the corresponding fields.
    SendDlgItemMessageW(hwnd, IDC_LOCAL_GROUP_NAME,    EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_LOCAL_GROUP_COMMENT, EM_LIMITTEXT, 2048, 0);

    // If setInitial() was called before exec(), push those values into the
    // controls so this dialog can be used for "Edit group" as well.
    if (!result_.parent_group.empty())
    {
        const HWND combo = GetDlgItem(hwnd, IDC_LOCAL_GROUP_PARENT);
        const int index = static_cast<int>(SendMessageW(
            combo, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1),
            reinterpret_cast<LPARAM>(result_.parent_group.c_str())));
        if (index != CB_ERR)
        {
            SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(index), 0);
        }
        else
        {
            // Fallback: drop the group name as edit text for display.
            SetWindowTextW(combo, result_.parent_group.c_str());
        }
    }

    SetDlgItemTextW(hwnd, IDC_LOCAL_GROUP_NAME,    result_.name.c_str());
    SetDlgItemTextW(hwnd, IDC_LOCAL_GROUP_COMMENT, result_.comment.c_str());

    SetFocus(GetDlgItem(hwnd, IDC_LOCAL_GROUP_NAME));
}

void LocalGroupDialog::onOk(HWND hwnd)
{
    // Read the currently selected parent group from the combobox (if any).
    const HWND combo = GetDlgItem(hwnd, IDC_LOCAL_GROUP_PARENT);
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel != CB_ERR)
    {
        const LRESULT len = SendMessageW(combo, CB_GETLBTEXTLEN,
                                         static_cast<WPARAM>(sel), 0);
        if (len > 0)
        {
            std::wstring out(static_cast<size_t>(len), L'\0');
            SendMessageW(combo, CB_GETLBTEXT, static_cast<WPARAM>(sel),
                         reinterpret_cast<LPARAM>(out.data()));
            result_.parent_group = std::move(out);
        }
        else
        {
            result_.parent_group.clear();
        }
    }
    else
    {
        result_.parent_group = readEditText(combo);
    }

    result_.name    = readEditText(GetDlgItem(hwnd, IDC_LOCAL_GROUP_NAME));
    result_.comment = readEditText(GetDlgItem(hwnd, IDC_LOCAL_GROUP_COMMENT));
}

}  // namespace aspia::client_win32
