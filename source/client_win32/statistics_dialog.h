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
// Win32 replacement for client/ui/desktop/statistics_dialog.ui.
//
// Resource identifiers assigned to this dialog (declared in resource.h):
//
//   #define IDD_STATISTICS                          280
//
//   #define IDC_STATS_DURATION_VALUE                2801
//   #define IDC_STATS_TCP_TOTAL_VALUE               2802
//   #define IDC_STATS_TCP_SPEED_VALUE               2803
//   #define IDC_STATS_UDP_TOTAL_VALUE               2804
//   #define IDC_STATS_UDP_SPEED_VALUE               2805
//   #define IDC_STATS_VIDEO_PACKETS_VALUE           2806
//   #define IDC_STATS_VIDEO_PAUSE_RESUME_VALUE      2807
//   #define IDC_STATS_VIDEO_PACKET_MIN_MAX_AVG      2808
//   #define IDC_STATS_AUDIO_PACKETS_VALUE           2809
//   #define IDC_STATS_AUDIO_PAUSE_RESUME_VALUE      2810
//   #define IDC_STATS_AUDIO_PACKET_MIN_MAX_AVG      2811
//   #define IDC_STATS_VIDEO_CAPTURER_ENCODER        2812
//   #define IDC_STATS_FPS_VALUE                     2813
//   #define IDC_STATS_MOUSE_SEND_DROP_VALUE         2814
//   #define IDC_STATS_KEY_SEND_VALUE                2815
//   #define IDC_STATS_TEXT_SEND_VALUE               2816
//   #define IDC_STATS_CLIPBOARD_READ_SEND_VALUE     2817
//   #define IDC_STATS_CURSOR_SHAPE_COUNT_CACHE      2818
//   #define IDC_STATS_CURSOR_SHAPE_CACHE_SIZE       2819
//   #define IDC_STATS_CURSOR_POS_COUNT_VALUE        2820
//

#ifndef CLIENT_WIN32_STATISTICS_DIALOG_H
#define CLIENT_WIN32_STATISTICS_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class StatisticsDialog
{
public:
    struct Stats
    {
        // Session.
        int64_t session_duration_sec = 0;

        // Transport byte counters.
        int64_t tcp_bytes_received = 0;
        int64_t tcp_bytes_sent = 0;
        int64_t tcp_rx_bps = 0;
        int64_t tcp_tx_bps = 0;

        int64_t udp_bytes_received = 0;
        int64_t udp_bytes_sent = 0;
        int64_t udp_rx_bps = 0;
        int64_t udp_tx_bps = 0;

        // Video stream.
        int64_t video_packet_count = 0;
        int64_t video_pause_count = 0;
        int64_t video_resume_count = 0;
        int64_t video_packet_min_bytes = 0;
        int64_t video_packet_max_bytes = 0;
        int64_t video_packet_avg_bytes = 0;
        int video_capturer_ms = 0;
        int video_encoder_ms = 0;
        int fps = 0;

        // Audio stream.
        int64_t audio_packet_count = 0;
        int64_t audio_pause_count = 0;
        int64_t audio_resume_count = 0;
        int64_t audio_packet_min_bytes = 0;
        int64_t audio_packet_max_bytes = 0;
        int64_t audio_packet_avg_bytes = 0;

        // Input / clipboard.
        int64_t mouse_events_sent = 0;
        int64_t mouse_events_dropped = 0;
        int64_t key_events_sent = 0;
        int64_t text_events_sent = 0;
        int64_t clipboard_events_read = 0;
        int64_t clipboard_events_sent = 0;

        // Cursor.
        int64_t cursor_shape_count = 0;
        int64_t cursor_shape_cache_hits = 0;
        int64_t cursor_shape_cache_size = 0;
        int64_t cursor_pos_count = 0;
    };

    StatisticsDialog(HINSTANCE instance, HWND parent);
    ~StatisticsDialog() = default;

    StatisticsDialog(const StatisticsDialog&) = delete;
    StatisticsDialog& operator=(const StatisticsDialog&) = delete;

    // Shows the dialog modally. Returns when the user presses OK/Close.
    bool exec();

    // Pushes a fresh snapshot of the statistics into the dialog. Safe to call
    // before exec() (values are cached) or while the dialog is visible (from
    // the same thread that owns the dialog).
    void updateStats(const Stats& stats);

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void refreshControls(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    HWND hwnd_ = nullptr;
    Stats stats_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_STATISTICS_DIALOG_H
