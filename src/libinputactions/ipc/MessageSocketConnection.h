/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#pragma once

#include "JsonSerializer.h"
#include "messages.h"
#include <QFutureInterface>
#include <QLocalSocket>
#include <QTimer>
#include <qloggingcategory.h>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_IPC)

namespace InputActions
{

static const QString INPUTACTIONS_IPC_SOCKET_PATH = "/var/run/inputactions/socket";

class Message;

struct FutureResponse
{
    std::chrono::steady_clock::time_point requestTimestamp;
    QFutureInterface<std::shared_ptr<ResponseMessage>> futureInterface;
};

/**
 * An interface for reading/writing messages from/to an InputActions socket.
 */
class MessageSocketConnection : public QObject
{
    Q_OBJECT

public:
    MessageSocketConnection(QLocalSocket *socket = {}, QObject *parent = {});

    /**
     * Thread-safe.
     */
    void sendMessage(const Message &message);
    /**
     * Thread-safe.
     */
    template<typename T>
    std::shared_ptr<T> sendMessageAndWaitForResponse(const RequestMessage &message)
    {
        if (!m_socket || !m_socket->isValid()) {
            return {};
        }

        m_futuresMutex.lock();
        auto &future = m_futures[message.requestId()];
        m_futuresMutex.unlock();
        future.requestTimestamp = std::chrono::steady_clock::now();
        future.futureInterface.reportStarted();

        write(m_serializer.serialize(message));

        future.futureInterface.waitForFinished();
        std::shared_ptr<T> result;
        if (future.futureInterface.resultCount() == 1) {
            result = std::static_pointer_cast<T>(future.futureInterface.takeResult());
        }

        m_futuresMutex.lock();
        m_futures.erase(message.requestId());
        m_futuresMutex.unlock();
        return result;
    }

    QLocalSocket *socket() const;

signals:
    void messageReceived(std::shared_ptr<Message> message);

private slots:
    void onReadyRead();
    void onFutureTimeoutTimerTick();

private:
    void write(QString data);

    QLocalSocket *m_socket;
    QByteArray m_buffer;
    JsonSerializer m_serializer;

    std::map<QString, FutureResponse> m_futures;
    std::mutex m_futuresMutex;

    QTimer m_futureTimeoutTimer;
};

}