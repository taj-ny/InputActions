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

#include "ProcessRunner.h"
#include <libinputactions/variables/VariableManager.h>
#include <map>

namespace InputActions
{

void ProcessRunner::startProcess(const QString &program, const QStringList &arguments, bool wait)
{
    startProcess(program, arguments, g_variableManager->extraProcessEnvironment(arguments.join(" ")), wait);
}

QString ProcessRunner::startProcessReadOutput(const QString &program, const QStringList &arguments)
{
    return startProcessReadOutput(program, arguments, g_variableManager->extraProcessEnvironment(arguments.join(" ")));
}

void ProcessRunner::startProcess(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment, bool wait)
{
    auto *process = new QProcess;
    connect(process, &QProcess::finished, this, [process]() {
        process->deleteLater();
    });
    process->setProgram("/bin/sh");
    process->setArguments(arguments);
    setProcessEnvironment(*process, extraEnvironment);
    process->start();
    if (wait) {
        process->waitForFinished();
    }
}

QString ProcessRunner::startProcessReadOutput(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment)
{
    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments(arguments);
    setProcessEnvironment(process, extraEnvironment);
    process.start();
    process.waitForFinished();
    return process.readAllStandardOutput();
}

void ProcessRunner::setProcessEnvironment(QProcess &process, std::map<QString, QString> environmentVariables)
{
    auto environment = QProcessEnvironment::systemEnvironment();
    for (const auto &[key, value] : environmentVariables) {
        environment.insert(key, value);
    }
    process.setProcessEnvironment(environment);
}

}
