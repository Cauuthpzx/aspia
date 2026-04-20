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
// Dialog template owned by this translation unit (expected to live in
// client.rc; reproduced here for reference). Grid: 16 pt rows, column 1 at
// x=14 (labels, 120 wide), column 2 at x=138 (values, ~104 wide).
//
//   IDD_STATISTICS DIALOGEX 0, 0, 250, 280
//   STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
//   CAPTION "Statistics"
//   FONT 9, "Segoe UI", 400, 0, 0x1
//   BEGIN
//       GROUPBOX        "",                          -1,                                 7,   4, 236, 250
//       LTEXT           "Duration:",                 -1,                                14,  16, 120,   9
//       LTEXT           "",                          IDC_STATS_DURATION_VALUE,         138,  16, 104,   9
//       LTEXT           "Total TCP RX/TX:",          -1,                                14,  28, 120,   9
//       LTEXT           "",                          IDC_STATS_TCP_TOTAL_VALUE,        138,  28, 104,   9
//       LTEXT           "Speed TCP RX/TX:",          -1,                                14,  40, 120,   9
//       LTEXT           "",                          IDC_STATS_TCP_SPEED_VALUE,        138,  40, 104,   9
//       LTEXT           "Total UDP RX/TX:",          -1,                                14,  52, 120,   9
//       LTEXT           "",                          IDC_STATS_UDP_TOTAL_VALUE,        138,  52, 104,   9
//       LTEXT           "Speed UDP RX/TX:",          -1,                                14,  64, 120,   9
//       LTEXT           "",                          IDC_STATS_UDP_SPEED_VALUE,        138,  64, 104,   9
//       LTEXT           "Video Packet Count:",       -1,                                14,  76, 120,   9
//       LTEXT           "",                          IDC_STATS_VIDEO_PACKETS_VALUE,    138,  76, 104,   9
//       LTEXT           "Video Pause/Resume:",       -1,                                14,  88, 120,   9
//       LTEXT           "",                          IDC_STATS_VIDEO_PAUSE_RESUME_VALUE,138, 88, 104,   9
//       LTEXT           "Video Packet MIN/MAX/AVG:", -1,                                14, 100, 120,   9
//       LTEXT           "",                          IDC_STATS_VIDEO_PACKET_MIN_MAX_AVG,138,100, 104,   9
//       LTEXT           "Audio Packet Count:",       -1,                                14, 112, 120,   9
//       LTEXT           "",                          IDC_STATS_AUDIO_PACKETS_VALUE,    138, 112, 104,   9
//       LTEXT           "Audio Pause/Resume:",       -1,                                14, 124, 120,   9
//       LTEXT           "",                          IDC_STATS_AUDIO_PAUSE_RESUME_VALUE,138,124, 104,   9
//       LTEXT           "Audio Packet MIN/MAX/AVG:", -1,                                14, 136, 120,   9
//       LTEXT           "",                          IDC_STATS_AUDIO_PACKET_MIN_MAX_AVG,138,136, 104,   9
//       LTEXT           "Video Capturer/Encoder:",   -1,                                14, 148, 120,   9
//       LTEXT           "",                          IDC_STATS_VIDEO_CAPTURER_ENCODER, 138, 148, 104,   9
//       LTEXT           "FPS:",                      -1,                                14, 160, 120,   9
//       LTEXT           "",                          IDC_STATS_FPS_VALUE,              138, 160, 104,   9
//       LTEXT           "Mouse Event Send/Drop:",    -1,                                14, 172, 120,   9
//       LTEXT           "",                          IDC_STATS_MOUSE_SEND_DROP_VALUE,  138, 172, 104,   9
//       LTEXT           "Key Event Send:",           -1,                                14, 184, 120,   9
//       LTEXT           "",                          IDC_STATS_KEY_SEND_VALUE,         138, 184, 104,   9
//       LTEXT           "Text Event Send:",          -1,                                14, 196, 120,   9
//       LTEXT           "",                          IDC_STATS_TEXT_SEND_VALUE,        138, 196, 104,   9
//       LTEXT           "Clipboard Event R/S:",      -1,                                14, 208, 120,   9
//       LTEXT           "",                          IDC_STATS_CLIPBOARD_READ_SEND_VALUE,138,208,104,   9
//       LTEXT           "Cursor Shape Count/Cache:", -1,                                14, 220, 120,   9
//       LTEXT           "",                          IDC_STATS_CURSOR_SHAPE_COUNT_CACHE,138,220, 104,   9
//       LTEXT           "Cursor Shape Cache Size:",  -1,                                14, 232, 120,   9
//       LTEXT           "",                          IDC_STATS_CURSOR_SHAPE_CACHE_SIZE,138, 232, 104,   9
//       LTEXT           "Cursor Pos Count:",         -1,                                14, 244, 120,   9
//       LTEXT           "",                          IDC_STATS_CURSOR_POS_COUNT_VALUE, 138, 244, 104,   9
//       DEFPUSHBUTTON   "OK",                        IDOK,                             193, 260,  50,  14
//   END
//

#include "client_win32/statistics_dialog.h"

#include "client_win32/resource.h"

#include <cstdio>
#include <cwchar>

namespace aspia::client_win32 {

namespace {

// Formats a byte count into a human-readable string using binary (KiB/MiB/GiB)
// units. Values below 1024 are rendered as bytes; larger values use one decimal
// place.
std::wstring formatBytes(int64_t bytes)
{
    constexpr int64_t kKiB = 1024;
    constexpr int64_t kMiB = kKiB * 1024;
    constexpr int64_t kGiB = kMiB * 1024;
    constexpr int64_t kTiB = kGiB * 1024;

    wchar_t buf[64] = {};

    if (bytes < kKiB)
    {
        std::swprintf(buf, std::size(buf), L"%lld B",
                      static_cast<long long>(bytes));
    }
    else if (bytes < kMiB)
    {
        std::swprintf(buf, std::size(buf), L"%.1f KiB",
                      static_cast<double>(bytes) / static_cast<double>(kKiB));
    }
    else if (bytes < kGiB)
    {
        std::swprintf(buf, std::size(buf), L"%.1f MiB",
                      static_cast<double>(bytes) / static_cast<double>(kMiB));
    }
    else if (bytes < kTiB)
    {
        std::swprintf(buf, std::size(buf), L"%.2f GiB",
                      static_cast<double>(bytes) / static_cast<double>(kGiB));
    }
    else
    {
        std::swprintf(buf, std::size(buf), L"%.2f TiB",
                      static_cast<double>(bytes) / static_cast<double>(kTiB));
    }

    return buf;
}

// Formats a throughput value in bytes per second. Values under 1 KiB/s are
// rendered as B/s; otherwise the same binary units are used.
std::wstring formatBps(int64_t bps)
{
    constexpr int64_t kKiB = 1024;
    constexpr int64_t kMiB = kKiB * 1024;
    constexpr int64_t kGiB = kMiB * 1024;

    wchar_t buf[64] = {};

    if (bps < kKiB)
    {
        std::swprintf(buf, std::size(buf), L"%lld B/s",
                      static_cast<long long>(bps));
    }
    else if (bps < kMiB)
    {
        std::swprintf(buf, std::size(buf), L"%.1f KiB/s",
                      static_cast<double>(bps) / static_cast<double>(kKiB));
    }
    else if (bps < kGiB)
    {
        std::swprintf(buf, std::size(buf), L"%.1f MiB/s",
                      static_cast<double>(bps) / static_cast<double>(kMiB));
    }
    else
    {
        std::swprintf(buf, std::size(buf), L"%.2f GiB/s",
                      static_cast<double>(bps) / static_cast<double>(kGiB));
    }

    return buf;
}

std::wstring formatDuration(int64_t seconds)
{
    if (seconds < 0)
        seconds = 0;

    const int64_t hours   = seconds / 3600;
    const int64_t minutes = (seconds % 3600) / 60;
    const int64_t secs    = seconds % 60;

    wchar_t buf[32] = {};
    std::swprintf(buf, std::size(buf), L"%02lld:%02lld:%02lld",
                  static_cast<long long>(hours),
                  static_cast<long long>(minutes),
                  static_cast<long long>(secs));
    return buf;
}

std::wstring formatInt64(int64_t value)
{
    wchar_t buf[32] = {};
    std::swprintf(buf, std::size(buf), L"%lld", static_cast<long long>(value));
    return buf;
}

std::wstring formatPair(int64_t a, int64_t b)
{
    return formatInt64(a) + L" / " + formatInt64(b);
}

std::wstring formatBytesPair(int64_t a, int64_t b)
{
    return formatBytes(a) + L" / " + formatBytes(b);
}

std::wstring formatBpsPair(int64_t a, int64_t b)
{
    return formatBps(a) + L" / " + formatBps(b);
}

std::wstring formatBytesTriple(int64_t mn, int64_t mx, int64_t avg)
{
    return formatBytes(mn) + L" / " + formatBytes(mx) + L" / " + formatBytes(avg);
}

std::wstring formatMsPair(int a, int b)
{
    wchar_t buf[32] = {};
    std::swprintf(buf, std::size(buf), L"%d ms / %d ms", a, b);
    return buf;
}

}  // namespace

StatisticsDialog::StatisticsDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool StatisticsDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_STATISTICS), parent_,
        &StatisticsDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

void StatisticsDialog::updateStats(const Stats& stats)
{
    stats_ = stats;
    if (hwnd_ && IsWindow(hwnd_))
        refreshControls(hwnd_);
}

// static
INT_PTR CALLBACK StatisticsDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    StatisticsDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<StatisticsDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<StatisticsDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR StatisticsDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_DESTROY:
            hwnd_ = nullptr;
            break;

        default:
            break;
    }

    return FALSE;
}

void StatisticsDialog::onInitDialog(HWND hwnd)
{
    hwnd_ = hwnd;
    refreshControls(hwnd);
    SetFocus(GetDlgItem(hwnd, IDOK));
}

void StatisticsDialog::refreshControls(HWND hwnd)
{
    const Stats& s = stats_;

    SetDlgItemTextW(hwnd, IDC_STATS_DURATION_VALUE,
                    formatDuration(s.session_duration_sec).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_TCP_TOTAL_VALUE,
                    formatBytesPair(s.tcp_bytes_received, s.tcp_bytes_sent).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_TCP_SPEED_VALUE,
                    formatBpsPair(s.tcp_rx_bps, s.tcp_tx_bps).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_UDP_TOTAL_VALUE,
                    formatBytesPair(s.udp_bytes_received, s.udp_bytes_sent).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_UDP_SPEED_VALUE,
                    formatBpsPair(s.udp_rx_bps, s.udp_tx_bps).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_VIDEO_PACKETS_VALUE,
                    formatInt64(s.video_packet_count).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_VIDEO_PAUSE_RESUME_VALUE,
                    formatPair(s.video_pause_count, s.video_resume_count).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_VIDEO_PACKET_MIN_MAX_AVG,
                    formatBytesTriple(s.video_packet_min_bytes,
                                      s.video_packet_max_bytes,
                                      s.video_packet_avg_bytes).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_AUDIO_PACKETS_VALUE,
                    formatInt64(s.audio_packet_count).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_AUDIO_PAUSE_RESUME_VALUE,
                    formatPair(s.audio_pause_count, s.audio_resume_count).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_AUDIO_PACKET_MIN_MAX_AVG,
                    formatBytesTriple(s.audio_packet_min_bytes,
                                      s.audio_packet_max_bytes,
                                      s.audio_packet_avg_bytes).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_VIDEO_CAPTURER_ENCODER,
                    formatMsPair(s.video_capturer_ms, s.video_encoder_ms).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_FPS_VALUE,
                    formatInt64(s.fps).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_MOUSE_SEND_DROP_VALUE,
                    formatPair(s.mouse_events_sent, s.mouse_events_dropped).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_KEY_SEND_VALUE,
                    formatInt64(s.key_events_sent).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_TEXT_SEND_VALUE,
                    formatInt64(s.text_events_sent).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_CLIPBOARD_READ_SEND_VALUE,
                    formatPair(s.clipboard_events_read,
                               s.clipboard_events_sent).c_str());

    SetDlgItemTextW(hwnd, IDC_STATS_CURSOR_SHAPE_COUNT_CACHE,
                    formatPair(s.cursor_shape_count,
                               s.cursor_shape_cache_hits).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_CURSOR_SHAPE_CACHE_SIZE,
                    formatBytes(s.cursor_shape_cache_size).c_str());
    SetDlgItemTextW(hwnd, IDC_STATS_CURSOR_POS_COUNT_VALUE,
                    formatInt64(s.cursor_pos_count).c_str());
}

}  // namespace aspia::client_win32
