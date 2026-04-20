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
// Win32 replacement for client/ui/sys_info/system_info_session_window.ui.
//
// The Qt original is a QWidget with:
//   - A QToolBar (Save | Print | Refresh)
//   - A QSplitter (horizontal)
//       left:  QTreeWidget (category tree, no header, icon size 20)
//       right: QWidget placeholder (receives the active sys_info sub-widget)
//
// Resource IDs:
//   IDC_SYSINFO_WIN_TOOLBAR   8200  (Win32 toolbar control)
//   IDC_SYSINFO_WIN_TREE_CAT  8201  (SysTreeView32, category tree)
//   IDC_SYSINFO_WIN_CONTENT   8202  (placeholder child window for sub-panels)
//   IDM_SYSINFO_SAVE          8210
//   IDM_SYSINFO_PRINT         8211
//   IDM_SYSINFO_REFRESH       8212
//   IDM_SYSINFO_COPY_ROW      8213
//   IDM_SYSINFO_COPY_NAME     8214
//   IDM_SYSINFO_COPY_VALUE    8215
//

#ifndef CLIENT_WIN32_SYSTEM_INFO_SESSION_WINDOW_H
#define CLIENT_WIN32_SYSTEM_INFO_SESSION_WINDOW_H

#include <windows.h>
#include <commctrl.h>

#include <functional>
#include <string>

namespace aspia::client_win32 {

class SystemInfoSessionWindow
{
public:
    // Callbacks fired by toolbar buttons or keyboard shortcuts.
    struct Delegate
    {
        std::function<void()> onSave;
        std::function<void()> onPrint;
        std::function<void()> onRefresh;
        std::function<void()> onCopyRow;
        std::function<void()> onCopyName;
        std::function<void()> onCopyValue;
        // Called when the user selects a category in the tree.
        std::function<void(int categoryId)> onCategorySelected;
    };

    explicit SystemInfoSessionWindow(HINSTANCE instance);
    ~SystemInfoSessionWindow();

    SystemInfoSessionWindow(const SystemInfoSessionWindow&) = delete;
    SystemInfoSessionWindow& operator=(const SystemInfoSessionWindow&) = delete;

    bool create();
    void show(int showCmd = SW_SHOW);

    // Add a named category to the tree. Returns the newly inserted HTREEITEM.
    HTREEITEM addCategory(HTREEITEM parent, const wchar_t* name, int categoryId);

    // Replace the child window shown in the content pane.
    void setContentWindow(HWND child);

    // Enable / disable toolbar buttons by command ID.
    void enableButton(int cmdId, bool enable);

    void setDelegate(Delegate delegate) { delegate_ = std::move(delegate); }

    HWND handle()       const { return hwnd_; }
    HWND contentPane()  const { return content_; }

    // Pre-translate WM_KEYDOWN Ctrl+S / Ctrl+P / F5.
    bool preTranslateMessage(MSG* msg);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onNotify(LPARAM lp);
    void onCommand(int cmdId);
    void onContextMenu(HWND target, int screenX, int screenY);
    void onDestroy();

    void createToolbar();
    void createTree();
    void createContentPane();
    void layoutChildren(int width, int height);

    HINSTANCE instance_;
    HWND      hwnd_    = nullptr;
    HWND      toolbar_ = nullptr;
    HWND      tree_    = nullptr;
    HWND      content_ = nullptr;
    Delegate  delegate_;

    // Splitter position: left pane width in pixels (default 200).
    int splitterX_ = 200;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoSessionWindow";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYSTEM_INFO_SESSION_WINDOW_H
