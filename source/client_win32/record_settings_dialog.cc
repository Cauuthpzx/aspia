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
 * =====================================================================
 *  Win32 DIALOGEX template for IDD_RECORD_SETTINGS (to be placed in
 *  client_win32/client.rc). Sized 280 x 160 dialog units.
 * =====================================================================
 *
 * IDD_RECORD_SETTINGS DIALOGEX 0, 0, 280, 160
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Recording Settings"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     GROUPBOX        "",                     -1,                        7,   4, 266, 120
 *     AUTOCHECKBOX    "Automatically start recording sessions on connection",
 *                                             IDC_RECORD_AUTOSTART,     14,  16, 252,  10
 *     LTEXT           "Write path:",          IDC_RECORD_PATH_LABEL,    14,  32, 100,   9
 *     EDITTEXT                                IDC_RECORD_PATH,          14,  44, 234,  12, ES_AUTOHSCROLL
 *     PUSHBUTTON      "...",                  IDC_RECORD_BROWSE,       251,  43,  15,  14
 *     LTEXT           "Container format:",    IDC_RECORD_FORMAT_LABEL,  14,  64, 100,   9
 *     COMBOBOX                                IDC_RECORD_FORMAT,        14,  76, 252,  60,
 *                                             CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Codec:",               IDC_RECORD_CODEC_LABEL,   14,  94, 100,   9
 *     COMBOBOX                                IDC_RECORD_CODEC,         14, 106, 252,  60,
 *                                             CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
 *     DEFPUSHBUTTON   "OK",                   IDOK,                    166, 136,  50,  14
 *     PUSHBUTTON      "Cancel",               IDCANCEL,                223, 136,  50,  14
 * END
 *
 * =====================================================================
 */

#include "client_win32/record_settings_dialog.h"

#include "client_win32/resource.h"

#include <objbase.h>
#include <shobjidl.h>

#include <string>

namespace aspia::client_win32 {

namespace {

struct ComboItem
{
    const wchar_t* label;
};

// Available container formats. Order must match the persisted Result.container_format index.
constexpr ComboItem kContainers[] =
{
    { L"WebM" },
    { L"MKV"  },
};

// Available video codecs. Order must match the persisted Result.codec index.
constexpr ComboItem kCodecs[] =
{
    { L"VP8" },
    { L"VP9" },
};

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

void populateCombo(HWND combo, const ComboItem* items, size_t count, int selected)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < count; ++i)
    {
        SendMessageW(combo, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(items[i].label));
    }

    if (selected < 0 || static_cast<size_t>(selected) >= count)
        selected = 0;
    SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(selected), 0);
}

// RAII wrapper so we can bail out of onBrowse() at any point without leaking.
template <typename T>
class ComPtr
{
public:
    ComPtr() = default;
    ~ComPtr() { reset(); }

    ComPtr(const ComPtr&) = delete;
    ComPtr& operator=(const ComPtr&) = delete;

    T** addressOf() { reset(); return &ptr_; }
    T* get() const { return ptr_; }
    T* operator->() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }

    void reset()
    {
        if (ptr_)
        {
            ptr_->Release();
            ptr_ = nullptr;
        }
    }

private:
    T* ptr_ = nullptr;
};

}  // namespace

RecordSettingsDialog::RecordSettingsDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool RecordSettingsDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_RECORD_SETTINGS), parent_,
        &RecordSettingsDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK RecordSettingsDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    RecordSettingsDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<RecordSettingsDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<RecordSettingsDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR RecordSettingsDialog::handleMessage(
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

                case IDC_RECORD_BROWSE:
                    onBrowse(hwnd);
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

void RecordSettingsDialog::onInitDialog(HWND hwnd)
{
    CheckDlgButton(hwnd, IDC_RECORD_AUTOSTART,
                   result_.enable_recording ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(hwnd, IDC_RECORD_PATH, EM_LIMITTEXT, MAX_PATH, 0);
    SetDlgItemTextW(hwnd, IDC_RECORD_PATH, result_.output_path.c_str());

    populateCombo(GetDlgItem(hwnd, IDC_RECORD_FORMAT),
                  kContainers,
                  sizeof(kContainers) / sizeof(kContainers[0]),
                  result_.container_format);

    populateCombo(GetDlgItem(hwnd, IDC_RECORD_CODEC),
                  kCodecs,
                  sizeof(kCodecs) / sizeof(kCodecs[0]),
                  result_.codec);

    SetFocus(GetDlgItem(hwnd, IDC_RECORD_PATH));
}

void RecordSettingsDialog::onOk(HWND hwnd)
{
    result_.enable_recording =
        (IsDlgButtonChecked(hwnd, IDC_RECORD_AUTOSTART) == BST_CHECKED);
    result_.output_path = readEditText(GetDlgItem(hwnd, IDC_RECORD_PATH));

    const LRESULT fmt = SendDlgItemMessageW(hwnd, IDC_RECORD_FORMAT, CB_GETCURSEL, 0, 0);
    result_.container_format = (fmt == CB_ERR) ? 0 : static_cast<int>(fmt);

    const LRESULT codec = SendDlgItemMessageW(hwnd, IDC_RECORD_CODEC, CB_GETCURSEL, 0, 0);
    result_.codec = (codec == CB_ERR) ? 0 : static_cast<int>(codec);
}

void RecordSettingsDialog::onBrowse(HWND hwnd)
{
    // IFileDialog in folder-picker mode. Uses COM, so we must initialize
    // apartment-threaded COM for this thread if it isn't already.
    const HRESULT co_init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool owns_com = SUCCEEDED(co_init) && co_init != S_FALSE;

    ComPtr<IFileOpenDialog> dialog;
    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(dialog.addressOf()));

    if (SUCCEEDED(hr) && dialog)
    {
        DWORD options = 0;
        if (SUCCEEDED(dialog->GetOptions(&options)))
        {
            dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM |
                               FOS_PATHMUSTEXIST);
        }

        dialog->SetTitle(L"Select recording folder");

        if (dialog->Show(hwnd) == S_OK)
        {
            ComPtr<IShellItem> item;
            if (SUCCEEDED(dialog->GetResult(item.addressOf())) && item)
            {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path)
                {
                    SetDlgItemTextW(hwnd, IDC_RECORD_PATH, path);
                    CoTaskMemFree(path);
                }
            }
        }
    }

    dialog.reset();

    if (owns_com)
        CoUninitialize();
}

}  // namespace aspia::client_win32
