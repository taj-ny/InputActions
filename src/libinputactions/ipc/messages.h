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

#pragma once

#include <QString>
#include <QUuid>
#include <QVariantMap>

namespace InputActions
{

static const int INPUTACTIONS_IPC_PROTOCOL_VERSION = 1;

class MessageSocketConnection;

enum class MessageType : int
{
    BeginSessionRequest,
    BeginSessionResponse,
    EnvironmentState,
    HandshakeRequest,
    HandshakeResponse,
    LoadConfigRequest,
    LoadConfigResponse,
    RecordStrokeRequest,
    RecordStrokeResponse,
    SendNotification,
    StartProcessRequestMessage,
    StartProcessResponseMessage,
    VariableListRequest,
    VariableListResponse,
};

template<typename T>
std::map<QString, T> toStdMap(QMap<QString, QVariant> map)
{
    std::map<QString, T> result;
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        result[it.key()] = it.value().value<T>();
    }
    return result;
}

template<typename T>
QMap<QString, QVariant> toQtMap(std::map<QString, T> map)
{
    QMap<QString, QVariant> result;
    for (const auto &[key, value] : map) {
        result[key] = value;
    }
    return result;
}

class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int type MEMBER m_type)

public:
    Message(MessageType type)
        : m_type(static_cast<int>(type))
    {
    }

    virtual ~Message() = default;

    MessageType type() const { return static_cast<MessageType>(m_type); }

    MessageSocketConnection *sender() const { return m_sender; }
    void setSender(MessageSocketConnection *value) { m_sender = value; }

protected:
    MessageSocketConnection *m_sender;

private:
    int m_type;
};

class ResponseMessage : public Message
{
    Q_OBJECT
    Q_PROPERTY(QString requestId MEMBER m_requestId)
    Q_PROPERTY(bool success MEMBER m_success)
    Q_PROPERTY(QString error MEMBER m_error)

public:
    ResponseMessage(MessageType type)
        : Message(type)
    {
    }

    const QString &requestId() const { return m_requestId; }
    void setRequestId(const QString &value) { m_requestId = value; }

    bool success() const { return m_success; }

    const QString &error() const { return m_error; }
    /**
     * Also sets success to false.
     */
    void setError(const QString &error);

private:
    QString m_requestId;

    bool m_success = true;
    QString m_error;
};

class RequestMessage : public Message
{
    Q_OBJECT
    Q_PROPERTY(QString requestId MEMBER m_requestId)

public:
    RequestMessage(MessageType type)
        : Message(type)
    {
    }

    void reply(ResponseMessage &message) const;

    const QString &requestId() const { return m_requestId; }

private:
    QString m_requestId = QUuid::createUuid().toString();
};

class BeginSessionRequestMessage : public RequestMessage
{
    Q_OBJECT
    Q_PROPERTY(QString tty MEMBER m_tty)

public:
    BeginSessionRequestMessage()
        : RequestMessage(MessageType::BeginSessionRequest)
    {
    }

    const QString &tty() const { return m_tty; }
    void setTty(const QString &value) { m_tty = value; }

private:
    QString m_tty;
};

class BeginSessionResponseMessage : public ResponseMessage
{
    Q_OBJECT

public:
    BeginSessionResponseMessage()
        : ResponseMessage(MessageType::BeginSessionResponse)
    {
    }
};

class HandshakeRequestMessage : public RequestMessage
{
    Q_OBJECT
    Q_PROPERTY(int protocolVersion MEMBER m_protocolVersion)

public:
    HandshakeRequestMessage()
        : RequestMessage(MessageType::HandshakeRequest)
    {
    }

    int protocolVersion() const { return m_protocolVersion; }

private:
    int m_protocolVersion = INPUTACTIONS_IPC_PROTOCOL_VERSION;
};

class HandshakeResponseMessage : public ResponseMessage
{
    Q_OBJECT

public:
    HandshakeResponseMessage()
        : ResponseMessage(MessageType::HandshakeResponse)
    {
    }
};

class EnvironmentStateMessage : public Message
{
    Q_OBJECT
    Q_PROPERTY(QString stateJson MEMBER m_stateJson)

public:
    EnvironmentStateMessage()
        : Message(MessageType::EnvironmentState)
    {
    }

    const QString &stateJson() const { return m_stateJson; }
    void setStateJson(const QString &value) { m_stateJson = value; }

private:
    QString m_stateJson;
};

class LoadConfigRequestMessage : public RequestMessage
{
    Q_OBJECT
    Q_PROPERTY(QString config MEMBER m_config)

public:
    LoadConfigRequestMessage()
        : RequestMessage(MessageType::LoadConfigRequest)
    {
    }

    const QString &config() const { return m_config; }
    void setConfig(const QString &value) { m_config = value; }

private:
    QString m_config;
};

class LoadConfigResponseMessage : public ResponseMessage
{
    Q_OBJECT

public:
    LoadConfigResponseMessage()
        : ResponseMessage(MessageType::LoadConfigResponse)
    {
    }
};

class RecordStrokeRequestMessage : public RequestMessage
{
    Q_OBJECT

public:
    RecordStrokeRequestMessage()
        : RequestMessage(MessageType::RecordStrokeRequest)
    {
    }
};

class RecordStrokeResponseMessage : public ResponseMessage
{
    Q_OBJECT
    Q_PROPERTY(QString stroke MEMBER m_stroke)

public:
    RecordStrokeResponseMessage()
        : ResponseMessage(MessageType::RecordStrokeResponse)
    {
    }

    const QString &stroke() const { return m_stroke; }
    void setStroke(const QString &value) { m_stroke = value; }

private:
    QString m_stroke;
};

class SendNotificationMessage : public Message
{
    Q_OBJECT
    Q_PROPERTY(QString title MEMBER m_title)
    Q_PROPERTY(QString content MEMBER m_content)

public:
    SendNotificationMessage()
        : Message(MessageType::SendNotification)
    {
    }

    const QString &title() const { return m_title; }
    void setTitle(const QString &value) { m_title = value; }

    const QString &content() const { return m_content; }
    void setContent(const QString &value) { m_content = value; }

private:
    QString m_title;
    QString m_content;
};

class StartProcessRequestMessage : public RequestMessage
{
    Q_OBJECT
    Q_PROPERTY(QString program MEMBER m_program)
    Q_PROPERTY(QStringList arguments MEMBER m_arguments)
    Q_PROPERTY(QVariantMap environment MEMBER m_environment)
    Q_PROPERTY(bool wait MEMBER m_wait)
    Q_PROPERTY(bool output MEMBER m_output)

public:
    StartProcessRequestMessage()
        : RequestMessage(MessageType::StartProcessRequestMessage)
    {
    }

    const QString &program() const { return m_program; }
    void setProgram(const QString &value) { m_program = value; }

    const QStringList &arguments() const { return m_arguments; }
    void setArguments(const QStringList &value) { m_arguments = value; }

    std::map<QString, QString> environment() const { return toStdMap<QString>(m_environment); }
    void setEnvironment(std::map<QString, QString> value) { m_environment = toQtMap(value); }

    /**
     * @return Wait for the process to exit before sending a reply.
     */
    bool wait() const { return m_wait; }
    void setWait(bool value) { m_wait = value; }

    /**
     * @return Wait for the process to exit and provide its output in the reply.
     */
    bool output() const { return m_output; }
    void setOutput(bool value) { m_output = value; }

private:
    QString m_program;
    QStringList m_arguments;
    QVariantMap m_environment;
    bool m_wait{};
    bool m_output{};
};

class StartProcessResponseMessage : public ResponseMessage
{
    Q_OBJECT
    Q_PROPERTY(QString output MEMBER m_output)

public:
    StartProcessResponseMessage()
        : ResponseMessage(MessageType::StartProcessResponseMessage)
    {
    }

    const QString &output() const { return m_output; }
    void setOutput(const QString &value) { m_output = value; }

private:
    QString m_output;
};

class VariableListRequestMessage : public RequestMessage
{
    Q_OBJECT
    Q_PROPERTY(QString filter MEMBER m_filter)

public:
    VariableListRequestMessage()
        : RequestMessage(MessageType::VariableListRequest)
    {
    }

    const QString &filter() const { return m_filter; }
    void setFilter(const QString &value) { m_filter = value; }

private:
    QString m_filter;
};

class VariableListResponseMessage : public ResponseMessage
{
    Q_OBJECT
    Q_PROPERTY(QString variables MEMBER m_variables)

public:
    VariableListResponseMessage()
        : ResponseMessage(MessageType::VariableListResponse)
    {
    }

    const QString &variables() const { return m_variables; }
    void setVariables(const QString &value) { m_variables = value; }

private:
    QString m_variables;
};

}