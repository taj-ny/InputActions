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

#include <CLI/CLI.hpp>
#include <QDBusInterface>

void printResponse(QDBusPendingCall call)
{
    call.waitForFinished();
    const auto reply = call.reply();

    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        std::cerr << reply.errorMessage().toStdString() << '\n';
        return;
    }

    if (reply.arguments().size() > 0) {
        std::cout << reply.arguments().at(0).toString().toStdString() << '\n';
    }
}

void ensureInterfaceIsValid(const QDBusInterface &interface)
{
    if (!interface.isValid()) {
        std::cerr << "Failed to connect to the InputActions DBus interface\n";
        std::exit(1);
    }
}

int main(int argc, char **argv)
{
    CLI::App app;

    QDBusInterface dbusInterface("org.inputactions", "/", "org.inputactions");

    auto *config = app.add_subcommand("config", "Manage config")->require_subcommand();
    config->add_subcommand("reload", "Reload config")->callback([&dbusInterface]() {
        ensureInterfaceIsValid(dbusInterface);
        printResponse(dbusInterface.asyncCall("reloadConfig"));
    });

    app.add_subcommand("record-stroke", "Record a stroke using a mouse or touchpad")->callback([&dbusInterface]() {
        ensureInterfaceIsValid(dbusInterface);
        printResponse(dbusInterface.asyncCall("recordStroke"));
    });
    app.add_subcommand("resume", "Resume InputActions")->callback([&dbusInterface]() {
        ensureInterfaceIsValid(dbusInterface);
        printResponse(dbusInterface.asyncCall("reloadConfig"));
    });
    app.add_subcommand("suspend", "Suspend InputActions")->callback([&dbusInterface]() {
        ensureInterfaceIsValid(dbusInterface);
        printResponse(dbusInterface.asyncCall("suspend"));
    });

    auto *variables = app.add_subcommand("variables", "Manage variables")->require_subcommand();

    auto *variablesList = variables->add_subcommand("list", "List variables");
    std::string variablesListFilter;
    variablesList->add_option("-f,--filter", variablesListFilter, "Only show variables that match the specified regular expression")->default_str("");
    variablesList->callback([&dbusInterface, &variablesListFilter]() {
        ensureInterfaceIsValid(dbusInterface);
        printResponse(dbusInterface.asyncCall("variables", QString::fromStdString(variablesListFilter)));
    });

    CLI11_PARSE(app, argc, argv);

    if (argc == 1) {
        std::cout << app.help() << '\n';
    }

    return 0;
}