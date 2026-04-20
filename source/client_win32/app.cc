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

#include "client_win32/app.h"

#include "client_win32/main_window.h"

#include <QCoreApplication>
#include <commctrl.h>

namespace aspia::client_win32 {

App::App(HINSTANCE instance)
    : instance_(instance)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_TREEVIEW_CLASSES |
                ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);
}

App::~App() = default;

int App::run(int argc, char** argv, int showCmd)
{
    // QCoreApplication's event loop on Windows is a Win32 message pump.
    // Win32 HWNDs created normally receive messages through it, and Qt
    // objects (signals/slots, timers, network) also work correctly.
    QCoreApplication qApp(argc, argv);

    MainWindow mainWindow(instance_);
    if (!mainWindow.create())
        return -1;

    mainWindow.show(showCmd);

    // MainWindow::onDestroy() calls QCoreApplication::quit() via
    // PostQuitMessage(0), which causes qApp.exec() to return.
    return qApp.exec();
}

}  // namespace aspia::client_win32
