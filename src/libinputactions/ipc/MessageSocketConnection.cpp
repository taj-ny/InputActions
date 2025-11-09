/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "MessageSocketConnection.h"
#include "utils/ThreadUtils.h"
#include <QThread>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_IPC, "inputactions.ipc", QtWarningMsg)

namespace InputActions
{

static const std::chrono::milliseconds RESPONSE_TIMEOUT{10000L}; // Timeout must not be too low due to stroke recording

MessageSocketConnection::MessageSocketConnection(QLocalSocket *socket, QObject *parent)
    : QObject(parent)
    , m_socket(socket)
{
    if (!m_socket) {
        return;
    }

    connect(m_socket, &QLocalSocket::readyRead, this, &MessageSocketConnection::onReadyRead);
    connect(&m_futureTimeoutTimer, &QTimer::timeout, this, &MessageSocketConnection::onFutureTimeoutTimerTick);
    m_futureTimeoutTimer.setInterval(1000);
    m_futureTimeoutTimer.start();
}

void MessageSocketConnection::sendMessage(const Message &message)
{
    if (!m_socket) {
        return;
    }

    write(m_serializer.serialize(message));
}

QLocalSocket *MessageSocketConnection::socket() const
{
    return m_socket;
}

void MessageSocketConnection::onReadyRead()
{
    // readAll rarely causes an infinite loop for some reason, do not use
    m_buffer += m_socket->read(m_socket->bytesAvailable());

    while (true) {
        auto newLineIndex = m_buffer.indexOf('\n');
        if (newLineIndex == -1) {
            break;
        }

        const auto json = QString::fromUtf8(m_buffer.left(newLineIndex));
        qDebug(INPUTACTIONS_IPC).noquote().nospace() << "IN: " << json;

        m_buffer.remove(0, newLineIndex + 1);
        auto message = m_serializer.deserializeMessage(json);
        if (!message) {
            qWarning(INPUTACTIONS_IPC).noquote().nospace() << "Received malformed message: " << json;
            continue;
        }

        message->setSender(this);
        Q_EMIT messageReceived(message);

        std::lock_guard lock(m_futuresMutex);
        if (auto response = std::dynamic_pointer_cast<ResponseMessage>(message); response && m_futures.contains(response->requestId())) {
            if (m_futures.contains(response->requestId())) {
                m_futures[response->requestId()].futureInterface.reportResult(response);
                m_futures[response->requestId()].futureInterface.reportFinished();
            }
        }
    }
}

void MessageSocketConnection::onFutureTimeoutTimerTick()
{
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard lock(m_futuresMutex);

    ThreadUtils::runOnThread(
        m_socket->thread(),
        [this, &now]() {
            for (auto &[_, future] : m_futures) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - future.requestTimestamp) > RESPONSE_TIMEOUT) {
                    future.futureInterface.reportFinished();
                }
            }
        },
        true);
}

void MessageSocketConnection::write(QString data)
{
    if (!m_socket || !m_socket->isValid()) {
        return;
    }

    qDebug(INPUTACTIONS_IPC).noquote().nospace() << "OUT: " << data;
    if (!data.endsWith('\n')) {
        data += '\n';
    }

    ThreadUtils::runOnThread(m_socket->thread(), [this, data]() {
        QTextStream stream(m_socket);
        stream.setEncoding(QStringConverter::Utf8);
        stream << data;
        stream.flush();
    });
}

}