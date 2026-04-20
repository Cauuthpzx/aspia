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
#define IDD_ABOUT              210
#define IDD_STATUS             220
#define IDD_DOWNLOAD           230
#define IDD_ROUTER             240
#define IDD_DESKTOP_CONFIG     250
#define IDD_RECORD_SETTINGS    260
#define IDD_SETTINGS           270
#define IDD_STATISTICS         280
#define IDD_FILE_TRANSFER      300
#define IDD_FILE_REMOVE        310
#define IDD_LOCAL_COMPUTER     330
#define IDD_ROUTER_USER        340
#define IDD_LOCAL_GROUP        350
#define IDD_HOST_CHANGE_PASSWORD   360
#define IDD_HOST_CHECK_PASSWORD    370
#define IDD_HOST_CONNECT_CONFIRM   380
#define IDD_HOST_USER              390
#define IDD_HOST_CONFIG            430
#define IDD_CONSOLE_ADDRESS_BOOK      450
#define IDD_CONSOLE_OPEN_ADDRESS_BOOK 460
#define IDD_CONSOLE_FAST_CONNECT      470
#define IDD_CONSOLE_COMPUTER          480
#define IDD_CONSOLE_COMPUTER_GROUP    490
#define IDD_CONSOLE_UPDATE_SETTINGS   500

// Accelerator / message IDs for top-level windows (no dialog template)
#define IDR_TASKMGR_ACCEL      290
#define IDM_CHAT_SEND          320
#define IDR_ROUTER_MGR_ACCEL   400
#define IDR_HOST_NOTIFIER_ACCEL 410
#define IDR_HOST_MAIN_MENU     420
#define IDR_HOST_MAIN_ACCEL    421

// Main window child-control IDs
#define IDC_MAIN_TABS         1001
#define IDC_MAIN_STATUSBAR    1002

// Authorization dialog control IDs
#define IDC_AUTH_ADDRESS       2000
#define IDC_AUTH_PORT          2001
#define IDC_AUTH_USERNAME      2002
#define IDC_AUTH_PASSWORD      2003
#define IDC_AUTH_SHOW_PASSWORD 2004
#define IDC_AUTH_ONE_TIME      2005

// About dialog control IDs
#define IDC_ABOUT_NAME        2100
#define IDC_ABOUT_VERSION     2101
#define IDC_ABOUT_COPYRIGHT   2102
#define IDC_ABOUT_INFO        2103
#define IDC_ABOUT_SITE_LINK   2104
#define IDC_ABOUT_MAIL_LINK   2105
#define IDC_ABOUT_DONATE      2106

// Status dialog control IDs
#define IDC_STATUS_MESSAGE    2200

// Download dialog control IDs
#define IDC_DOWNLOAD_STATUS   2300
#define IDC_DOWNLOAD_PROGRESS 2301

// Router dialog control IDs
#define IDC_ROUTER_ADDRESS         2401
#define IDC_ROUTER_NAME            2402
#define IDC_ROUTER_SESSION_TYPE    2403
#define IDC_ROUTER_USERNAME        2404
#define IDC_ROUTER_PASSWORD        2405
#define IDC_ROUTER_SHOW_PASSWORD   2406

// Desktop config dialog control IDs
#define IDC_DCFG_VIDEO_CODEC             2500
#define IDC_DCFG_PIXEL_FORMAT            2501
#define IDC_DCFG_COMPRESS_RATIO          2502
#define IDC_DCFG_COMPRESS_RATIO_LABEL    2503
#define IDC_DCFG_FPS                     2504
#define IDC_DCFG_FPS_LABEL               2505
#define IDC_DCFG_AUDIO                   2510
#define IDC_DCFG_CLIPBOARD               2511
#define IDC_DCFG_CURSOR_SHAPE            2512
#define IDC_DCFG_CURSOR_POSITION         2513
#define IDC_DCFG_DISABLE_EFFECTS         2514
#define IDC_DCFG_DISABLE_WALLPAPER       2515
#define IDC_DCFG_LOCK_AT_DISCONNECT      2516
#define IDC_DCFG_BLOCK_REMOTE_INPUT      2517

// Record settings dialog control IDs
#define IDC_RECORD_AUTOSTART       2600
#define IDC_RECORD_PATH_LABEL      2601
#define IDC_RECORD_PATH            2602
#define IDC_RECORD_BROWSE          2603
#define IDC_RECORD_FORMAT_LABEL    2604
#define IDC_RECORD_FORMAT          2605
#define IDC_RECORD_CODEC_LABEL     2606
#define IDC_RECORD_CODEC           2607

// Settings dialog control IDs
#define IDC_SETTINGS_LANGUAGE              2700
#define IDC_SETTINGS_THEME                 2701
#define IDC_SETTINGS_DISPLAY_NAME          2702
#define IDC_SETTINGS_AUDIO                 2710
#define IDC_SETTINGS_CLIPBOARD             2711
#define IDC_SETTINGS_CURSOR_SHAPE          2712
#define IDC_SETTINGS_CURSOR_POSITION       2713
#define IDC_SETTINGS_DISABLE_EFFECTS       2720
#define IDC_SETTINGS_DISABLE_WALLPAPER     2721
#define IDC_SETTINGS_LOCK_AT_DISCONNECT    2730
#define IDC_SETTINGS_BLOCK_REMOTE_INPUT    2731
#define IDC_SETTINGS_CHECK_UPDATES         2740
#define IDC_SETTINGS_CUSTOM_UPDATE_SERVER  2741
#define IDC_SETTINGS_UPDATE_SERVER         2742
#define IDC_SETTINGS_CHECK_FOR_UPDATES_NOW 2743
#define IDC_SETTINGS_LOG_ENABLED           2750
#define IDC_SETTINGS_LOG_LEVEL             2751
#define IDC_SETTINGS_LOG_PATH              2752
#define IDC_SETTINGS_LOG_BROWSE            2753

// Statistics dialog control IDs
#define IDC_STATS_DURATION_VALUE            2801
#define IDC_STATS_TCP_TOTAL_VALUE           2802
#define IDC_STATS_TCP_SPEED_VALUE           2803
#define IDC_STATS_UDP_TOTAL_VALUE           2804
#define IDC_STATS_UDP_SPEED_VALUE           2805
#define IDC_STATS_VIDEO_PACKETS_VALUE       2806
#define IDC_STATS_VIDEO_PAUSE_RESUME_VALUE  2807
#define IDC_STATS_VIDEO_PACKET_MIN_MAX_AVG  2808
#define IDC_STATS_AUDIO_PACKETS_VALUE       2809
#define IDC_STATS_AUDIO_PAUSE_RESUME_VALUE  2810
#define IDC_STATS_AUDIO_PACKET_MIN_MAX_AVG  2811
#define IDC_STATS_VIDEO_CAPTURER_ENCODER    2812
#define IDC_STATS_FPS_VALUE                 2813
#define IDC_STATS_MOUSE_SEND_DROP_VALUE     2814
#define IDC_STATS_KEY_SEND_VALUE            2815
#define IDC_STATS_TEXT_SEND_VALUE           2816
#define IDC_STATS_CLIPBOARD_READ_SEND_VALUE 2817
#define IDC_STATS_CURSOR_SHAPE_COUNT_CACHE  2818
#define IDC_STATS_CURSOR_SHAPE_CACHE_SIZE   2819
#define IDC_STATS_CURSOR_POS_COUNT_VALUE    2820

// Task manager window control IDs
#define IDC_TASKMGR_TABS          2900
#define IDC_TASKMGR_LIST_PROCESS  2901
#define IDC_TASKMGR_LIST_SERVICE  2902
#define IDC_TASKMGR_LIST_USERS    2903
#define IDC_TASKMGR_LIST_PERF     2904
#define IDC_TASKMGR_STATUSBAR     2905
#define IDC_TASKMGR_END_TASK      2910
#define IDC_TASKMGR_DISCONNECT    2911
#define IDC_TASKMGR_LOGOFF        2912

// File transfer dialog control IDs
#define IDC_FILE_TRANSFER_CURRENT_ITEM      3000
#define IDC_FILE_TRANSFER_SPEED             3001
#define IDC_FILE_TRANSFER_OVERALL_LABEL     3002
#define IDC_FILE_TRANSFER_ITEM_LABEL        3003
#define IDC_FILE_TRANSFER_PROGRESS_OVERALL  3004
#define IDC_FILE_TRANSFER_PROGRESS_ITEM     3005
#define IDC_FILE_TRANSFER_STATUS            3006

// File remove dialog control IDs
#define IDC_FILE_REMOVE_CURRENT_ITEM  3100
#define IDC_FILE_REMOVE_PROGRESS      3101

// Chat session window control IDs
#define IDC_CHAT_HISTORY   3201
#define IDC_CHAT_INPUT     3202
#define IDC_CHAT_SEND      3203

// Local computer dialog control IDs
#define IDC_LOCAL_COMPUTER_GROUP          3301
#define IDC_LOCAL_COMPUTER_NAME           3302
#define IDC_LOCAL_COMPUTER_ADDRESS        3303
#define IDC_LOCAL_COMPUTER_USERNAME       3304
#define IDC_LOCAL_COMPUTER_PASSWORD       3305
#define IDC_LOCAL_COMPUTER_SHOW_PASSWORD  3306
#define IDC_LOCAL_COMPUTER_COMMENT        3307

// Local group dialog control IDs
#define IDC_LOCAL_GROUP_PARENT    3501
#define IDC_LOCAL_GROUP_NAME      3502
#define IDC_LOCAL_GROUP_COMMENT   3503

// Router user dialog control IDs
#define IDC_ROUTER_USER_USERNAME                 3400
#define IDC_ROUTER_USER_PASSWORD                 3401
#define IDC_ROUTER_USER_PASSWORD_RETRY           3402
#define IDC_ROUTER_USER_SHOW_PASSWORD            3403
#define IDC_ROUTER_USER_DISABLE                  3404
#define IDC_ROUTER_USER_SESSIONS_GROUP           3405
#define IDC_ROUTER_USER_SESSION_DESKTOP_MANAGE   3410
#define IDC_ROUTER_USER_SESSION_DESKTOP_VIEW     3411
#define IDC_ROUTER_USER_SESSION_FILE_TRANSFER    3412
#define IDC_ROUTER_USER_SESSION_SYSTEM_INFO      3413
#define IDC_ROUTER_USER_SESSION_TEXT_CHAT        3414
#define IDC_ROUTER_USER_SESSION_PORT_FORWARDING  3415

// Router manager window control IDs (top-level window)
#define IDC_ROUTER_MGR_TABS          4000
#define IDC_ROUTER_MGR_LIST_USERS    4001
#define IDC_ROUTER_MGR_LIST_HOSTS    4002
#define IDC_ROUTER_MGR_LIST_ACTIVE   4003
#define IDC_ROUTER_MGR_STATUSBAR     4004
#define IDC_ROUTER_MGR_TOOLBAR       4005
#define IDC_ROUTER_MGR_ADD           4010
#define IDC_ROUTER_MGR_EDIT          4011
#define IDC_ROUTER_MGR_REMOVE        4012
#define IDC_ROUTER_MGR_REFRESH       4013
#define IDC_ROUTER_MGR_DISCONNECT    4014
#define IDC_ROUTER_MGR_REFRESH_ACT   4015
#define IDC_ROUTER_MGR_CLOSE         4016

// File panel (dual-pane file manager) control IDs
#define IDC_FILE_PANEL_HEADER       4100
#define IDC_FILE_PANEL_DRIVE        4101
#define IDC_FILE_PANEL_ADDRESS      4102
#define IDC_FILE_PANEL_TOOLBAR      4103
#define IDC_FILE_PANEL_LIST         4104
#define IDC_FILE_PANEL_STATUS       4105
#define IDC_FILE_PANEL_TB_UP        4110
#define IDC_FILE_PANEL_TB_REFRESH   4111
#define IDC_FILE_PANEL_TB_HOME      4112
#define IDC_FILE_PANEL_TB_NEWFOLDER 4113
#define IDC_FILE_PANEL_TB_DELETE    4114
#define IDC_FILE_PANEL_TB_SEND      4115
#define IDC_FILE_PANEL_TB_UPLOAD    4116
#define IDC_FILE_PANEL_TB_DOWNLOAD  4117

// File transfer session window control IDs
#define IDC_FT_SESSION_LOCAL_PANEL   4200
#define IDC_FT_SESSION_REMOTE_PANEL  4201
#define IDC_FT_SESSION_STATUSBAR     4202

// Hosts tab control IDs
#define IDC_HOSTS_SEARCH    4300
#define IDC_HOSTS_TREE      4301
#define IDC_HOSTS_LIST      4302

// Sys-info child widgets list control IDs
#define IDC_SYSINFO_PROCESSES_LIST          4400
#define IDC_SYSINFO_SUMMARY_TREE            4500
#define IDC_SYSINFO_APPS_LIST               4600
#define IDC_SYSINFO_SERVICES_LIST           4700
#define IDC_SYSINFO_CONNECTIONS_LIST        4800
#define IDC_SYSINFO_DEVICES_LIST            4900
#define IDC_SYSINFO_DRIVERS_LIST            5000
#define IDC_SYSINFO_EVENT_LOGS_LIST         5100
#define IDC_SYSINFO_EVENT_LOGS_COMBO        5101
#define IDC_SYSINFO_LICENSES_LIST           5200
#define IDC_SYSINFO_LOCAL_USERS_LIST        5300
#define IDC_SYSINFO_LOCAL_USER_GROUPS_LIST  5400
#define IDC_SYSINFO_MONITORS_LIST           5500
#define IDC_SYSINFO_NET_ADAPTERS_LIST       5600
#define IDC_SYSINFO_NET_SHARES_LIST         5700
#define IDC_SYSINFO_OPEN_FILES_LIST         5800
#define IDC_SYSINFO_PRINTERS_LIST           5900
#define IDC_SYSINFO_ROUTES_LIST             6000
#define IDC_SYSINFO_VIDEO_ADAPTERS_LIST     6100
#define IDC_SYSINFO_POWER_OPTIONS_TREE      6200
#define IDC_SYSINFO_ENV_VARS_LIST           6300

// Host notifier window control IDs
#define IDC_HOST_NOTIFIER_LIST        6400
#define IDC_HOST_NOTIFIER_DISCONNECT  6401
#define IDC_HOST_NOTIFIER_LOCK_MOUSE  6402
#define IDC_HOST_NOTIFIER_LOCK_KEYBD  6403
#define IDC_HOST_NOTIFIER_PAUSE       6404
#define IDC_HOST_NOTIFIER_COLLAPSE    6405

// Host config dialog control IDs (General tab)
#define IDC_HOST_CONFIG_PORT                         6600
#define IDC_HOST_CONFIG_ALLOW_UDP                    6601
#define IDC_HOST_CONFIG_VIDEO_CAPTURER               6602
#define IDC_HOST_CONFIG_AUTO_UPDATE                  6603
#define IDC_HOST_CONFIG_UPDATE_CHECK_FREQ            6604
#define IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER     6605
#define IDC_HOST_CONFIG_UPDATE_SERVER                6606
#define IDC_HOST_CONFIG_CHECK_UPDATES_NOW            6607
#define IDC_HOST_CONFIG_IMPORT                       6608
#define IDC_HOST_CONFIG_EXPORT                       6609
// Host config dialog control IDs (Security tab)
#define IDC_HOST_CONFIG_CHANGE_PASSWORD              6620
#define IDC_HOST_CONFIG_PASS_PROTECTION              6621
#define IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE      6622
#define IDC_HOST_CONFIG_ONETIME_PASS_CHANGE          6623
#define IDC_HOST_CONFIG_ONETIME_PASS_CHARS           6624
#define IDC_HOST_CONFIG_ONETIME_PASS_CHAR_COUNT      6625
#define IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE         6626
#define IDC_HOST_CONFIG_CONN_CONFIRM_AUTO            6627
#define IDC_HOST_CONFIG_NO_USER_ACTION               6628
#define IDC_HOST_CONFIG_DISABLE_SHUTDOWN             6629
// Host config dialog control IDs (Router tab)
#define IDC_HOST_CONFIG_ENABLE_ROUTER                6640
#define IDC_HOST_CONFIG_ROUTER_ADDRESS               6641
#define IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY            6642
// Host config dialog control IDs (Users tab)
#define IDC_HOST_CONFIG_USERS_LIST                   6660
#define IDC_HOST_CONFIG_USER_ADD                     6661
#define IDC_HOST_CONFIG_USER_MODIFY                  6662
#define IDC_HOST_CONFIG_USER_DELETE                  6663
// Host config dialog Apply button
#define IDC_HOST_CONFIG_APPLY                        6680

// Host main window control IDs
#define IDC_HOST_MAIN_LABEL_ID         6500
#define IDC_HOST_MAIN_VALUE_ID         6501
#define IDC_HOST_MAIN_VALUE_IP         6502
#define IDC_HOST_MAIN_VALUE_STATUS     6503
#define IDC_HOST_MAIN_ACCEPT_INCOMING  6530
#define IDC_HOST_MAIN_USE_PASSWORD     6531
#define IDC_HOST_MAIN_CHANGE_PASSWORD  6540
#define IDC_HOST_MAIN_SETTINGS         6541
#define IDC_HOST_MAIN_HELP             6542
#define IDC_HOST_MAIN_EXIT             6543
#define IDC_HOST_MAIN_STATUSBAR        6590

// Console address-book dialog control IDs
#define IDC_CONSOLE_AB_NAME                        6800
#define IDC_CONSOLE_AB_ENCRYPTION                  6801
#define IDC_CONSOLE_AB_PASSWORD                    6802
#define IDC_CONSOLE_AB_PASSWORD_RETRY              6803
#define IDC_CONSOLE_AB_SHOW_PASSWORD               6804
#define IDC_CONSOLE_AB_COMMENT                     6805
#define IDC_CONSOLE_AB_LABEL_NAME                  6806
#define IDC_CONSOLE_AB_LABEL_ENCRYPTION            6807
#define IDC_CONSOLE_AB_LABEL_PASSWORD              6808
#define IDC_CONSOLE_AB_LABEL_PASSWORD_RETRY        6809
#define IDC_CONSOLE_AB_LABEL_COMMENT               6810

// Console open-address-book dialog control IDs
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_PROMPT       6900
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_FILE_LABEL   6901
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_FILE         6902
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_ENC_LABEL    6903
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_ENC          6904
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD_LABEL    6905
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD          6906
#define IDC_CONSOLE_OPEN_ADDRESS_BOOK_SHOW         6907

// Console fast-connect dialog control IDs
#define IDC_CONSOLE_FAST_CONNECT_ADDRESS       7000
#define IDC_CONSOLE_FAST_CONNECT_SESSION_TYPE  7001

// Console computer dialog control IDs
#define IDC_CONSOLE_COMPUTER_NAME                 7100
#define IDC_CONSOLE_COMPUTER_PARENT_NAME          7101
#define IDC_CONSOLE_COMPUTER_ADDRESS              7102
#define IDC_CONSOLE_COMPUTER_INHERIT_CREDS        7103
#define IDC_CONSOLE_COMPUTER_USERNAME             7104
#define IDC_CONSOLE_COMPUTER_PASSWORD             7105
#define IDC_CONSOLE_COMPUTER_SHOW_PASSWORD        7106
#define IDC_CONSOLE_COMPUTER_COMMENT              7107
#define IDC_CONSOLE_COMPUTER_INHERIT_CONFIG       7120
#define IDC_CONSOLE_COMPUTER_AUDIO                7121
#define IDC_CONSOLE_COMPUTER_CLIPBOARD            7122
#define IDC_CONSOLE_COMPUTER_CURSOR_SHAPE         7123
#define IDC_CONSOLE_COMPUTER_CURSOR_POSITION      7124
#define IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS      7125
#define IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER    7126
#define IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT   7127
#define IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT   7128
#define IDC_CONSOLE_COMPUTER_PARENT_PATH          7140

// Console computer-group dialog control IDs
#define IDC_CONSOLE_CG_NAME                 7200
#define IDC_CONSOLE_CG_PARENT               7201
#define IDC_CONSOLE_CG_COMMENT              7202
#define IDC_CONSOLE_CG_LABEL_NAME           7203
#define IDC_CONSOLE_CG_LABEL_PARENT         7204
#define IDC_CONSOLE_CG_LABEL_COMMENT        7205
#define IDC_CONSOLE_CG_INHERIT_CONFIG       7210
#define IDC_CONSOLE_CG_GB_FEATURES          7211
#define IDC_CONSOLE_CG_AUDIO                7212
#define IDC_CONSOLE_CG_CLIPBOARD            7213
#define IDC_CONSOLE_CG_CURSOR_SHAPE         7214
#define IDC_CONSOLE_CG_CURSOR_POSITION      7215
#define IDC_CONSOLE_CG_GB_APPEARANCE        7216
#define IDC_CONSOLE_CG_DISABLE_EFFECTS      7217
#define IDC_CONSOLE_CG_DISABLE_WALLPAPER    7218
#define IDC_CONSOLE_CG_GB_OTHER             7219
#define IDC_CONSOLE_CG_LOCK_AT_DISCONNECT   7220
#define IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT   7221

// Console update-settings dialog control IDs
#define IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP  7300
#define IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER 7301
#define IDC_CONSOLE_UPDATE_SERVER_LABEL      7302
#define IDC_CONSOLE_UPDATE_SERVER            7303
#define IDC_CONSOLE_UPDATE_CHECK_NOW         7304

// Console main window control IDs
#define IDC_CONSOLE_MAIN_TABS       7400
#define IDC_CONSOLE_MAIN_STATUSBAR  7401
#define IDC_CONSOLE_MAIN_TOOLBAR    7402
#define IDC_CONSOLE_NEW             7410
#define IDC_CONSOLE_OPEN            7411
#define IDC_CONSOLE_SAVE            7412
#define IDC_CONSOLE_SAVE_AS         7413
#define IDC_CONSOLE_SAVE_ALL        7414
#define IDC_CONSOLE_CLOSE           7415
#define IDC_CONSOLE_CLOSE_ALL       7416
#define IDC_CONSOLE_EXIT            7417
#define IDC_CONSOLE_AB_PROPERTIES   7420
#define IDC_CONSOLE_ADD_GROUP       7421
#define IDC_CONSOLE_MODIFY_GROUP    7422
#define IDC_CONSOLE_DELETE_GROUP    7423
#define IDC_CONSOLE_ADD_COMPUTER    7424
#define IDC_CONSOLE_COPY_COMPUTER   7425
#define IDC_CONSOLE_MODIFY_COMPUTER 7426
#define IDC_CONSOLE_DELETE_COMPUTER 7427
#define IDC_CONSOLE_IMPORT          7428
#define IDC_CONSOLE_EXPORT          7429
#define IDC_CONSOLE_FAST_CONNECT_CMD 7440
#define IDC_CONSOLE_ROUTER_MANAGE   7441
#define IDC_CONSOLE_ONLINE_HELP     7450
#define IDC_CONSOLE_CHECK_UPDATES   7451
#define IDC_CONSOLE_UPDATE_SETTINGS_CMD 7452
#define IDC_CONSOLE_ABOUT           7453

// Host change-password dialog control IDs
#define IDC_HOST_CHANGE_PASSWORD_OLD         3600
#define IDC_HOST_CHANGE_PASSWORD_NEW         3601
#define IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT  3602
#define IDC_HOST_CHANGE_PASSWORD_SHOW        3603

// Host check-password dialog control IDs
#define IDC_HOST_CHECK_PASSWORD_PROMPT  3700
#define IDC_HOST_CHECK_PASSWORD_LABEL   3701
#define IDC_HOST_CHECK_PASSWORD_EDIT    3702
#define IDC_HOST_CHECK_PASSWORD_SHOW    3703

// Host connect-confirm dialog control IDs
#define IDC_HOST_CONFIRM_CLIENT_ADDRESS 3800
#define IDC_HOST_CONFIRM_USER_AGENT     3801
#define IDC_HOST_CONFIRM_SESSION_TYPE   3802
#define IDC_HOST_CONFIRM_REMEMBER       3803

// Host user dialog control IDs
#define IDC_HOST_USER_USERNAME                   3900
#define IDC_HOST_USER_PASSWORD                   3901
#define IDC_HOST_USER_PASSWORD_RETRY             3902
#define IDC_HOST_USER_SHOW_PASSWORD              3903
#define IDC_HOST_USER_DISABLE                    3904
#define IDC_HOST_USER_SESSIONS_GROUP             3905
#define IDC_HOST_USER_SESSION_DESKTOP_MANAGE     3910
#define IDC_HOST_USER_SESSION_DESKTOP_VIEW       3911
#define IDC_HOST_USER_SESSION_FILE_TRANSFER      3912
#define IDC_HOST_USER_SESSION_SYSTEM_INFO        3913
#define IDC_HOST_USER_SESSION_TEXT_CHAT          3914
#define IDC_HOST_USER_SESSION_PORT_FORWARDING    3915

// Desktop toolbar button / menu command IDs (8000..8099)
#define IDC_DT_PIN                8000
#define IDC_DT_SWITCH_SESSION     8001
#define IDC_DT_POWER_CONTROL      8002
#define IDC_DT_CAD                8003
#define IDC_DT_SETTINGS           8004
#define IDC_DT_AUTOSIZE           8005
#define IDC_DT_FULLSCREEN         8006
#define IDC_DT_FILE_TRANSFER      8007
#define IDC_DT_TEXT_CHAT          8008
#define IDC_DT_TASK_MANAGER       8009
#define IDC_DT_SYSTEM_INFO        8010
#define IDC_DT_MENU               8011
#define IDC_DT_MINIMIZE           8012
#define IDC_DT_CLOSE              8013
// Power control popup menu
#define IDM_DT_SHUTDOWN           8020
#define IDM_DT_REBOOT             8021
#define IDM_DT_REBOOT_SAFE_MODE   8022
#define IDM_DT_LOGOFF             8023
#define IDM_DT_LOCK               8024
// Advanced popup menu
#define IDM_DT_SCREENSHOT         8030
#define IDM_DT_STATISTICS         8031
#define IDM_DT_RECORDING_SETTINGS 8032
#define IDM_DT_START_RECORDING    8033
#define IDM_DT_PAUSE_VIDEO        8034
#define IDM_DT_PAUSE_AUDIO        8035
#define IDM_DT_PASTE_CLIPBOARD    8036
#define IDM_DT_SEND_KEY_COMBOS    8037
#define IDM_DT_AUTOSCROLL         8038
// Scale sub-menu items
#define IDM_DT_SCALE_FIT          8040
#define IDM_DT_SCALE_100          8041
#define IDM_DT_SCALE_90           8042
#define IDM_DT_SCALE_80           8043
#define IDM_DT_SCALE_70           8044
#define IDM_DT_SCALE_60           8045
#define IDM_DT_SCALE_50           8046

// Local group widget control IDs (8100..8109)
#define IDC_LOCAL_GROUP_WIDGET_LIST  8100

// Router widget control IDs (8110..8119)
#define IDC_ROUTER_WIDGET_LABEL      8110

// Router group widget control IDs (8120..8129)
#define IDC_ROUTER_GROUP_WIDGET_LIST 8120

// System info session window control IDs (8200..8299)
#define IDC_SYSINFO_WIN_TOOLBAR      8200
#define IDC_SYSINFO_WIN_TREE_CAT     8201
#define IDC_SYSINFO_WIN_CONTENT      8202
#define IDM_SYSINFO_SAVE             8210
#define IDM_SYSINFO_PRINT            8211
#define IDM_SYSINFO_REFRESH          8212
#define IDM_SYSINFO_COPY_ROW         8213
#define IDM_SYSINFO_COPY_NAME        8214
#define IDM_SYSINFO_COPY_VALUE       8215

// Desktop session window control IDs (8300..8399)
#define IDC_DESKTOP_WIN_CANVAS       8300

// Menu command IDs
#define ID_FILE_EXIT         40001
#define ID_FILE_CONNECT      40002
#define ID_HELP_ABOUT        40003

#endif  // CLIENT_WIN32_RESOURCE_H
