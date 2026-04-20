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

#include "client_win32/host_config_dialog.h"

#include "client_win32/resource.h"

#include <commctrl.h>
#include <windowsx.h>

#include <array>
#include <cwchar>

namespace aspia::client_win32 {

namespace {

// -----------------------------------------------------------------------
// Maximum lengths for text controls.
// -----------------------------------------------------------------------
constexpr int kPortMinValue          = 1;
constexpr int kPortMaxValue          = 65535;
constexpr int kOnetimeCharCountMin   = 6;
constexpr int kOnetimeCharCountMax   = 16;
constexpr int kUpdateServerMaxLen    = 255;
constexpr int kRouterAddressMaxLen   = 255;
constexpr int kRouterPublicKeyMaxLen = 4096;

// -----------------------------------------------------------------------
// Integer-keyed combo box helpers (text → integer payload).
// -----------------------------------------------------------------------
struct IntComboEntry
{
    const wchar_t* text;
    int            value;
};

void populateIntCombo(HWND combo,
                      const IntComboEntry* entries,
                      size_t              count,
                      int                 selected_value)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    int selected_index = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const LRESULT idx = SendMessageW(
            combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(entries[i].text));
        SendMessageW(combo, CB_SETITEMDATA,
                     static_cast<WPARAM>(idx),
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

// -----------------------------------------------------------------------
// Read the text from an EDITTEXT or COMBOBOX edit portion.
// -----------------------------------------------------------------------
std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

// -----------------------------------------------------------------------
// Read / write a numeric EDITTEXT that holds a port or count.
// -----------------------------------------------------------------------
int readIntEdit(HWND edit, int fallback)
{
    const std::wstring text = readEditText(edit);
    if (text.empty())
        return fallback;
    wchar_t* end   = nullptr;
    const long val = std::wcstol(text.c_str(), &end, 10);
    if (end == text.c_str())
        return fallback;
    return static_cast<int>(val);
}

void writeIntEdit(HWND hwnd, int ctrl_id, int value)
{
    wchar_t buf[16];
    std::swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d", value);
    SetDlgItemTextW(hwnd, ctrl_id, buf);
}

// -----------------------------------------------------------------------
// Combo-box entry tables (match the Qt config_dialog.cc addItem calls).
// -----------------------------------------------------------------------

// combo_video_capturer  -- Windows targets only.
constexpr std::array<IntComboEntry, 3> kVideoCapturerEntries = {{
    { L"Default", 0 },
    { L"DXGI",    1 },
    { L"GDI",     2 },
}};

// combobox_update_check_freq
constexpr std::array<IntComboEntry, 3> kUpdateFreqEntries = {{
    { L"Once a day",   1  },
    { L"Once a week",  7  },
    { L"Once a month", 30 },
}};

// combobox_onetime_pass_change
constexpr std::array<IntComboEntry, 7> kOnetimePassChangeEntries = {{
    { L"On reboot",       0    },
    { L"Every 5 minutes", 5    },
    { L"Every 30 minutes",30   },
    { L"Every 1 hour",    60   },
    { L"Every 6 hours",   360  },
    { L"Every 12 hours",  720  },
    { L"Every 24 hours",  1440 },
}};

// combobox_onetime_pass_chars  (bit-flag values matching base::PasswordGenerator)
// LOWER_CASE=1, UPPER_CASE=2, DIGITS=4  → combinations used by the Qt code.
constexpr std::array<IntComboEntry, 3> kOnetimePassCharsEntries = {{
    { L"Letters and digits", 7 },  // LOWER_CASE|UPPER_CASE|DIGITS
    { L"Letters",            3 },  // UPPER_CASE|LOWER_CASE
    { L"Digits",             4 },  // DIGITS
}};

// combobox_conn_confirm_auto  (seconds, 0 = Never)
constexpr std::array<IntComboEntry, 5> kConnConfirmAutoEntries = {{
    { L"Never",      0  },
    { L"15 seconds", 15 },
    { L"30 seconds", 30 },
    { L"45 seconds", 45 },
    { L"60 seconds", 60 },
}};

// combobox_no_user_action  (values matching SystemSettings::NoUserAction enum)
constexpr std::array<IntComboEntry, 2> kNoUserActionEntries = {{
    { L"Accept connection", 0 },
    { L"Reject connection", 1 },
}};

}  // namespace

// ---------------------------------------------------------------------------
// HostConfigDialog
// ---------------------------------------------------------------------------

HostConfigDialog::HostConfigDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool HostConfigDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_HOST_CONFIG), parent_,
        &HostConfigDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK HostConfigDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostConfigDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<HostConfigDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<HostConfigDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR HostConfigDialog::handleMessage(
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
                    collectValues(hwnd);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDC_HOST_CONFIG_APPLY:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onApply(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_AUTO_UPDATE:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onAutoUpdateToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCustomUpdateServerToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_CHECK_UPDATES_NOW:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCheckForUpdatesNow(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_IMPORT:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onImport(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_EXPORT:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onExport(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_CHANGE_PASSWORD:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onChangePassword(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_PASS_PROTECTION:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onTogglePassProtection(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onOneTimePasswordToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onRequireConfirmToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_ENABLE_ROUTER:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onEnableRouterToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_USER_ADD:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onAddUser(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_USER_MODIFY:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onModifyUser(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_HOST_CONFIG_USER_DELETE:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onDeleteUser(hwnd);
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

void HostConfigDialog::onInitDialog(HWND hwnd)
{
    // ----------------------------------------------------------------
    // General tab: Common group
    // ----------------------------------------------------------------
    // Port is stored as a plain numeric EDITTEXT with ES_NUMBER.
    writeIntEdit(hwnd, IDC_HOST_CONFIG_PORT, result_.general.tcp_port);

    CheckDlgButton(hwnd, IDC_HOST_CONFIG_ALLOW_UDP,
                   result_.general.allow_udp ? BST_CHECKED : BST_UNCHECKED);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_VIDEO_CAPTURER),
        kVideoCapturerEntries.data(), kVideoCapturerEntries.size(),
        0 /* Default */);

    // ----------------------------------------------------------------
    // General tab: Update Server group
    // ----------------------------------------------------------------
    CheckDlgButton(hwnd, IDC_HOST_CONFIG_AUTO_UPDATE,
                   result_.general.auto_update_enabled ? BST_CHECKED : BST_UNCHECKED);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_UPDATE_CHECK_FREQ),
        kUpdateFreqEntries.data(), kUpdateFreqEntries.size(),
        result_.general.update_check_freq);

    CheckDlgButton(hwnd, IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER,
                   result_.general.use_custom_update_server ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(hwnd, IDC_HOST_CONFIG_UPDATE_SERVER, EM_LIMITTEXT,
                        kUpdateServerMaxLen, 0);
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIG_UPDATE_SERVER,
                    result_.general.update_server.c_str());

    // ----------------------------------------------------------------
    // Security tab: One-time Password group
    // ----------------------------------------------------------------
    CheckDlgButton(hwnd, IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE,
                   result_.advanced.one_time_password_enabled ? BST_CHECKED : BST_UNCHECKED);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHANGE),
        kOnetimePassChangeEntries.data(), kOnetimePassChangeEntries.size(),
        result_.advanced.one_time_pass_change);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHARS),
        kOnetimePassCharsEntries.data(), kOnetimePassCharsEntries.size(),
        result_.advanced.one_time_pass_chars);

    writeIntEdit(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHAR_COUNT,
                 result_.advanced.one_time_pass_char_count);

    // ----------------------------------------------------------------
    // Security tab: Connection Confirmation group
    // ----------------------------------------------------------------
    CheckDlgButton(hwnd, IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE,
                   result_.advanced.require_connection_confirm ? BST_CHECKED : BST_UNCHECKED);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_CONN_CONFIRM_AUTO),
        kConnConfirmAutoEntries.data(), kConnConfirmAutoEntries.size(),
        result_.advanced.auto_confirm_via);

    populateIntCombo(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_NO_USER_ACTION),
        kNoUserActionEntries.data(), kNoUserActionEntries.size(),
        result_.advanced.no_user_action);

    // ----------------------------------------------------------------
    // Security tab: Other group
    // ----------------------------------------------------------------
    CheckDlgButton(hwnd, IDC_HOST_CONFIG_DISABLE_SHUTDOWN,
                   result_.advanced.disable_shutdown ? BST_CHECKED : BST_UNCHECKED);

    // ----------------------------------------------------------------
    // Router tab
    // ----------------------------------------------------------------
    CheckDlgButton(hwnd, IDC_HOST_CONFIG_ENABLE_ROUTER,
                   result_.router.enabled ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(hwnd, IDC_HOST_CONFIG_ROUTER_ADDRESS, EM_LIMITTEXT,
                        kRouterAddressMaxLen, 0);
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIG_ROUTER_ADDRESS,
                    result_.router.address.c_str());

    SendDlgItemMessageW(hwnd, IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY, EM_LIMITTEXT,
                        kRouterPublicKeyMaxLen, 0);
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY,
                    result_.router.host_key.c_str());

    // ----------------------------------------------------------------
    // Users tab
    // ----------------------------------------------------------------
    populateUsersList(hwnd);

    // ----------------------------------------------------------------
    // Apply initial enable-state for dependent controls.
    // ----------------------------------------------------------------
    updateUpdateServerEnable(hwnd);
    updateRouterEnables(hwnd);
    updateOneTimePasswordEnables(hwnd);
    updateConnectionConfirmEnables(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_HOST_CONFIG_PORT));
}

void HostConfigDialog::onOk(HWND hwnd)
{
    collectValues(hwnd);
}

void HostConfigDialog::onApply(HWND hwnd)
{
    collectValues(hwnd);
    applied_ = true;
}

void HostConfigDialog::onAutoUpdateToggled(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd, IDC_HOST_CONFIG_AUTO_UPDATE) == BST_CHECKED)
            ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_UPDATE_CHECK_FREQ), enabled);
}

void HostConfigDialog::onCustomUpdateServerToggled(HWND hwnd)
{
    updateUpdateServerEnable(hwnd);
}

void HostConfigDialog::onEnableRouterToggled(HWND hwnd)
{
    updateRouterEnables(hwnd);
}

void HostConfigDialog::onOneTimePasswordToggled(HWND hwnd)
{
    updateOneTimePasswordEnables(hwnd);
}

void HostConfigDialog::onRequireConfirmToggled(HWND hwnd)
{
    updateConnectionConfirmEnables(hwnd);
}

void HostConfigDialog::onAddUser(HWND /*hwnd*/)
{
    // TODO: launch HostUserDialog to create a new user entry, then append
    // the resulting UserEntry to result_.users.users and refresh the list.
}

void HostConfigDialog::onModifyUser(HWND hwnd)
{
    const HWND list = GetDlgItem(hwnd, IDC_HOST_CONFIG_USERS_LIST);
    const LRESULT sel = SendMessageW(list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR)
        return;
    // TODO: launch HostUserDialog pre-populated with result_.users.users[sel],
    // update the entry on acceptance, and refresh the list.
}

void HostConfigDialog::onDeleteUser(HWND hwnd)
{
    const HWND list = GetDlgItem(hwnd, IDC_HOST_CONFIG_USERS_LIST);
    const LRESULT sel = SendMessageW(list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR)
        return;

    const size_t idx = static_cast<size_t>(sel);
    if (idx >= result_.users.users.size())
        return;

    result_.users.users.erase(result_.users.users.begin() + idx);
    populateUsersList(hwnd);
}

void HostConfigDialog::onImport(HWND /*hwnd*/)
{
    // TODO: open a file-open dialog (.reg / .xml per host settings format),
    // parse the file, and pre-populate the dialog fields.
}

void HostConfigDialog::onExport(HWND /*hwnd*/)
{
    // TODO: open a file-save dialog, serialise the current collectValues()
    // snapshot, and write it to the chosen path.
}

void HostConfigDialog::onCheckForUpdatesNow(HWND /*hwnd*/)
{
    // The actual update check is triggered by the host main window once
    // the dialog returns IDOK. This stub prevents the button from being
    // treated as an accidental IDOK via accelerator processing.
}

void HostConfigDialog::onChangePassword(HWND /*hwnd*/)
{
    // TODO: launch HostChangePasswordDialog to let the user change the
    // settings protection password.
}

void HostConfigDialog::onTogglePassProtection(HWND hwnd)
{
    // The button text is "Install" when protection is off and "Remove"
    // when it is on. Toggle the internal flag and update the button label.
    result_.advanced.password_protect_settings =
        !result_.advanced.password_protect_settings;
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIG_PASS_PROTECTION,
                    result_.advanced.password_protect_settings
                        ? L"Remove" : L"Install");
}

// ---------------------------------------------------------------------------
// collectValues -- snapshot all control states into result_.
// ---------------------------------------------------------------------------
void HostConfigDialog::collectValues(HWND hwnd)
{
    // ---- General tab: Common group ------------------------------------
    {
        const int port = readIntEdit(
            GetDlgItem(hwnd, IDC_HOST_CONFIG_PORT), result_.general.tcp_port);
        result_.general.tcp_port =
            (port >= kPortMinValue && port <= kPortMaxValue)
                ? port : result_.general.tcp_port;
        result_.service.tcp_port = result_.general.tcp_port;
    }

    result_.general.allow_udp =
        IsDlgButtonChecked(hwnd, IDC_HOST_CONFIG_ALLOW_UDP) == BST_CHECKED;

    // video_capturer combo stores int values (index 0=Default, 1=DXGI, 2=GDI).
    // Store the selected item text as the objectName used by the Qt dialog.
    {
        const int capturer_val = readIntComboValue(
            GetDlgItem(hwnd, IDC_HOST_CONFIG_VIDEO_CAPTURER), 0);
        switch (capturer_val)
        {
            case 1:  result_.general.video_capturer = L"DXGI";    break;
            case 2:  result_.general.video_capturer = L"GDI";     break;
            default: result_.general.video_capturer = L"Default"; break;
        }
    }

    // ---- General tab: Update Server group ----------------------------
    result_.general.auto_update_enabled =
        IsDlgButtonChecked(hwnd, IDC_HOST_CONFIG_AUTO_UPDATE) == BST_CHECKED;

    result_.general.update_check_freq = readIntComboValue(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_UPDATE_CHECK_FREQ),
        result_.general.update_check_freq);

    result_.general.use_custom_update_server =
        IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER) == BST_CHECKED;

    result_.general.update_server =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CONFIG_UPDATE_SERVER));

    // ---- Security tab: One-time Password group -----------------------
    result_.advanced.one_time_password_enabled =
        IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE) == BST_CHECKED;

    result_.advanced.one_time_pass_change = readIntComboValue(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHANGE),
        result_.advanced.one_time_pass_change);

    result_.advanced.one_time_pass_chars = readIntComboValue(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHARS),
        result_.advanced.one_time_pass_chars);

    {
        const int count = readIntEdit(
            GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHAR_COUNT),
            result_.advanced.one_time_pass_char_count);
        result_.advanced.one_time_pass_char_count =
            (count >= kOnetimeCharCountMin && count <= kOnetimeCharCountMax)
                ? count : result_.advanced.one_time_pass_char_count;
    }

    // ---- Security tab: Connection Confirmation group -----------------
    result_.advanced.require_connection_confirm =
        IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE) == BST_CHECKED;

    result_.advanced.auto_confirm_via = readIntComboValue(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_CONN_CONFIRM_AUTO),
        result_.advanced.auto_confirm_via);

    result_.advanced.no_user_action = readIntComboValue(
        GetDlgItem(hwnd, IDC_HOST_CONFIG_NO_USER_ACTION),
        result_.advanced.no_user_action);

    // ---- Security tab: Other group -----------------------------------
    result_.advanced.disable_shutdown =
        IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_DISABLE_SHUTDOWN) == BST_CHECKED;

    // ---- Router tab --------------------------------------------------
    result_.router.enabled =
        IsDlgButtonChecked(hwnd, IDC_HOST_CONFIG_ENABLE_ROUTER) == BST_CHECKED;

    result_.router.address =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CONFIG_ROUTER_ADDRESS));

    result_.router.host_key =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY));
}

// ---------------------------------------------------------------------------
// populateUsersList -- rebuild the listbox from result_.users.users.
// ---------------------------------------------------------------------------
void HostConfigDialog::populateUsersList(HWND hwnd)
{
    const HWND list = GetDlgItem(hwnd, IDC_HOST_CONFIG_USERS_LIST);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);

    for (const UserEntry& entry : result_.users.users)
    {
        // Prefix disabled users with "(disabled)" so the state is visible
        // without a separate column.
        std::wstring display;
        if (!entry.enabled)
            display = L"(disabled) ";
        display += entry.username;

        SendMessageW(list, LB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(display.c_str()));
    }
}

// ---------------------------------------------------------------------------
// Enable-state helpers.
// ---------------------------------------------------------------------------
void HostConfigDialog::updateUpdateServerEnable(HWND hwnd)
{
    const BOOL custom =
        (IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER) == BST_CHECKED)
            ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_UPDATE_SERVER), custom);
}

void HostConfigDialog::updateRouterEnables(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd, IDC_HOST_CONFIG_ENABLE_ROUTER) == BST_CHECKED)
            ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_ROUTER_ADDRESS),    enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY), enabled);
}

void HostConfigDialog::updateOneTimePasswordEnables(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE) == BST_CHECKED)
            ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHANGE),    enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHARS),     enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_ONETIME_PASS_CHAR_COUNT),enabled);
}

void HostConfigDialog::updateConnectionConfirmEnables(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd,
            IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE) == BST_CHECKED)
            ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_CONN_CONFIRM_AUTO), enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_HOST_CONFIG_NO_USER_ACTION),    enabled);
}

}  // namespace aspia::client_win32
