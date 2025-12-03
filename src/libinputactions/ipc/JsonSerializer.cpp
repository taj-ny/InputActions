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

#include "JsonSerializer.h"
#include "messages.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>

namespace InputActions
{

void JsonSerializer::deserialize(const QString &json, QObject &object)
{
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return;
    }
    const auto jsonObject = document.object();

    const auto metaObject = object.metaObject();
    for (auto i = 0; i < metaObject->propertyCount(); ++i) {
        const auto property = metaObject->property(i);
        const auto jsonProperty = jsonObject[property.name()];

        if (!jsonProperty.isUndefined()) {
            property.write(&object, jsonProperty.toVariant());
        }
    }
}

std::shared_ptr<Message> JsonSerializer::deserializeMessage(const QString &json)
{
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return {};
    }

    const auto jsonObject = document.object();
    const auto &type = jsonObject["type"];
    if (type.isUndefined() || !type.isDouble()) {
        return {};
    }

    std::shared_ptr<Message> message;
    switch (static_cast<MessageType>(type.toInt())) {
        case MessageType::BeginSessionRequest:
            message = std::make_shared<BeginSessionRequestMessage>();
            break;
        case MessageType::BeginSessionResponse:
            message = std::make_shared<BeginSessionResponseMessage>();
            break;
        case MessageType::EnvironmentState:
            message = std::make_shared<EnvironmentStateMessage>();
            break;
        case MessageType::GenericResponse:
            message = std::make_shared<ResponseMessage>();
            break;
        case MessageType::HandshakeRequest:
            message = std::make_shared<HandshakeRequestMessage>();
            break;
        case MessageType::HandshakeResponse:
            message = std::make_shared<HandshakeResponseMessage>();
            break;
        case MessageType::InvokePlasmaGlobalShortcutRequest:
            message = std::make_shared<InvokePlasmaGlobalShortcutRequestMessage>();
            break;
        case MessageType::LoadConfigRequest:
            message = std::make_shared<LoadConfigRequestMessage>();
            break;
        case MessageType::LoadConfigResponse:
            message = std::make_shared<LoadConfigResponseMessage>();
            break;
        case MessageType::RecordStrokeRequest:
            message = std::make_shared<RecordStrokeRequestMessage>();
            break;
        case MessageType::RecordStrokeResponse:
            message = std::make_shared<RecordStrokeResponseMessage>();
            break;
        case MessageType::SendNotification:
            message = std::make_shared<SendNotificationMessage>();
            break;
        case MessageType::StartProcessRequestMessage:
            message = std::make_shared<StartProcessRequestMessage>();
            break;
        case MessageType::StartProcessResponseMessage:
            message = std::make_shared<StartProcessResponseMessage>();
            break;
        case MessageType::SuspendRequest:
            message = std::make_shared<SuspendRequestMessage>();
            break;
        case MessageType::VariableListRequest:
            message = std::make_shared<VariableListRequestMessage>();
            break;
        case MessageType::VariableListResponse:
            message = std::make_shared<VariableListResponseMessage>();
            break;
    }
    Q_ASSERT(message);
    if (!message) {
        return {};
    }

    deserialize(json, *message.get());
    return message;
}

QString JsonSerializer::serialize(const QObject &object)
{
    QVariantMap map;
    const auto metaObject = object.metaObject();
    for (auto i = 0; i < metaObject->propertyCount(); ++i) {
        const auto property = metaObject->property(i);
        if (strcmp(property.name(), "objectName") == 0) {
            continue;
        }

        map[property.name()] = property.read(&object);
    }
    return QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::JsonFormat::Compact);
}

}