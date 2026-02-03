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

#include "messages.h"
#include "MessageSocketConnection.h"

namespace InputActions
{

void ResponseMessage::setError(QString error)
{
    m_error = std::move(error);
    m_success = false;
}

void RequestMessageBase::sendResponse(const ResponseMessage &response) const
{
    m_sender->sendMessage(response);
}

}