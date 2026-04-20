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

#include <windows.h>

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE /*prev*/, LPWSTR /*cmdLine*/, int showCmd)
{
    // QCoreApplication requires argc/argv; use the CRT globals available in
    // all Windows executables (__argc / __argv from <stdlib.h>).
    aspia::client_win32::App app(instance);
    return app.run(__argc, __argv, showCmd);
}
