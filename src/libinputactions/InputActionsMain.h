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

#include "dbus/IntegratedDBusInterface.h"
#include <QObject>

namespace InputActions
{

class PointerPositionGetter;
class VariableManager;
class WindowProvider;

class InputActionsMain : public QObject
{
    Q_OBJECT

public:
    InputActionsMain();
    ~InputActionsMain() override;

    void setMissingImplementations();
    void initialize();
    void suspend();

    virtual void registerGlobalVariables(VariableManager *variableManager, std::shared_ptr<PointerPositionGetter> pointerPositionGetter = {},
                                         std::shared_ptr<WindowProvider> windowProvider = {});

private slots:
    void onConfigChanged(const QString &config);

private:
    template<typename T1, typename T2>
    void setMissingImplementation(std::shared_ptr<T1> &member)
    {
        if (!member) {
            member = std::make_shared<T2>();
        }
    }

    template<typename T>
    void setMissingImplementation(std::shared_ptr<T> &member)
    {
        if (!member) {
            member = std::make_shared<T>();
        }
    }

    template<typename T1, typename T2>
    void setMissingImplementation(std::unique_ptr<T1> &member)
    {
        if (!member) {
            member = std::make_unique<T2>();
        }
    }

    template<typename T>
    void setMissingImplementation(std::unique_ptr<T> &member)
    {
        if (!member) {
            member = std::make_unique<T>();
        }
    }

    IntegratedDBusInterface m_dbusInterface;
};

inline InputActionsMain *g_inputActions;

}