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

#include "kcm.h"

#include <KPluginFactory>

#include <QClipboard>
#include <QtDBus/QDBusInterface>
#include <QFile>
#include <QObject>

namespace KWin
{

K_PLUGIN_CLASS(KWinGesturesKCM)

KWinGesturesKCM::KWinGesturesKCM(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());
    setButtons(Button::NoAdditionalButton);

    QFile about(":/effects/inputactions_kwin/kcm/about.html");
    if (about.open(QIODevice::ReadOnly)) {
        const auto html = about.readAll().replace("${version}", PROJECT_VERSION);
        ui.aboutText->setHtml(html);
    }

    connect(ui.copy, &QPushButton::pressed, this, &KWinGesturesKCM::slotCopyPoints);
    connect(ui.recordStroke, &QPushButton::pressed, this, &KWinGesturesKCM::slotRecordStroke);
}

void KWinGesturesKCM::slotCopyPoints()
{
    QApplication::clipboard()->setText(ui.strokePoints->text());
}

void KWinGesturesKCM::slotRecordStroke()
{
    QDBusInterface interface("org.inputactions", "/");
    auto call = interface.asyncCall("recordStroke");
    call.waitForFinished();
    ui.strokePoints->setText(call.reply().arguments().at(0).toString());
}

}

#include "kcm.moc"
#include "moc_kcm.cpp"
