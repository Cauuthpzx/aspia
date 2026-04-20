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

#ifndef CLIENT_WIN32_RESOURCE_H
#define CLIENT_WIN32_RESOURCE_H

// Icons and menu resources
#define IDI_ASPIA              101
#define IDR_MAIN_MENU          102
#define IDR_MAIN_ACCEL         103

// Dialog templates
#define IDD_AUTH               200

// Main window child-control IDs
#define IDC_MAIN_TABS         1001
#define IDC_MAIN_STATUSBAR    1002

// Authorization dialog control IDs
#define IDC_AUTH_USERNAME     2001
#define IDC_AUTH_PASSWORD     2002
#define IDC_AUTH_SHOW_PASSWORD 2003
#define IDC_AUTH_ONE_TIME     2004

// Menu command IDs
#define ID_FILE_EXIT         40001
#define ID_FILE_CONNECT      40002
#define ID_HELP_ABOUT        40003

#endif  // CLIENT_WIN32_RESOURCE_H
