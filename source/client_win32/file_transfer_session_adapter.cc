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

#include "client_win32/file_transfer_session_adapter.h"

#include "client/client_file_transfer.h"
#include "client_win32/file_transfer_session_window.h"

namespace aspia::client_win32 {

FileTransferSessionAdapter::FileTransferSessionAdapter(client::ClientFileTransfer* client,
                                                       FileTransferSessionWindow* window,
                                                       QObject* parent)
    : QObject(parent),
      client_(client),
      window_(window)
{
    Q_ASSERT(client_);
    Q_ASSERT(window_);

    connect(client_, &client::ClientFileTransfer::sig_errorOccurred,
            this, &FileTransferSessionAdapter::onErrorOccurred,
            Qt::QueuedConnection);

    connect(client_, &client::ClientFileTransfer::sig_driveListReply,
            this, &FileTransferSessionAdapter::onDriveListReply,
            Qt::QueuedConnection);

    connect(client_, &client::ClientFileTransfer::sig_fileListReply,
            this, &FileTransferSessionAdapter::onFileListReply,
            Qt::QueuedConnection);

    connect(client_, &client::ClientFileTransfer::sig_createDirectoryReply,
            this, &FileTransferSessionAdapter::onCreateDirectoryReply,
            Qt::QueuedConnection);

    connect(client_, &client::ClientFileTransfer::sig_renameReply,
            this, &FileTransferSessionAdapter::onRenameReply,
            Qt::QueuedConnection);
}

FileTransferSessionAdapter::~FileTransferSessionAdapter() = default;

// ---------------------------------------------------------------------------
// Qt signal handlers
// ---------------------------------------------------------------------------

void FileTransferSessionAdapter::onErrorOccurred(proto::file_transfer::ErrorCode /*error_code*/)
{
    // Phase 3: show an error dialog / status bar message.
}

void FileTransferSessionAdapter::onDriveListReply(
    common::FileTask::Target /*target*/,
    proto::file_transfer::ErrorCode /*error_code*/,
    const proto::file_transfer::DriveList& /*drive_list*/)
{
    // Phase 3: populate the drive combo in the appropriate FilePanel.
}

void FileTransferSessionAdapter::onFileListReply(
    common::FileTask::Target /*target*/,
    proto::file_transfer::ErrorCode /*error_code*/,
    const proto::file_transfer::List& /*file_list*/)
{
    // Phase 3: reload the file list in the appropriate FilePanel.
}

void FileTransferSessionAdapter::onCreateDirectoryReply(
    common::FileTask::Target /*target*/,
    proto::file_transfer::ErrorCode /*error_code*/)
{
    // Phase 3: refresh the affected panel on success.
}

void FileTransferSessionAdapter::onRenameReply(
    common::FileTask::Target /*target*/,
    proto::file_transfer::ErrorCode /*error_code*/)
{
    // Phase 3: refresh the affected panel on success.
}

} // namespace aspia::client_win32
