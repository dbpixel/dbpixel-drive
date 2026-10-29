/*
 * Copyright (C) Fabian Müller <fmueller@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#pragma once

#include "abstractsetupwizardpage.h"
#include "setupwizardcontroller.h"

namespace Ui {
class AccountConfiguredWizardPage;
}

namespace OCC::Wizard {

class AccountConfiguredWizardPage : public AbstractSetupWizardPage
{
    Q_OBJECT
    // Drive: lidos e escritos pela tela de sincronização em QML (qml/drive/WizardSync.qml);
    // os widgets originais continuam guardando os valores, só ficam escondidos
    Q_PROPERTY(QString folder READ folder NOTIFY driveChanged)
    Q_PROPERTY(QString freeSpaceText READ freeSpaceText NOTIFY driveChanged)
    Q_PROPERTY(bool syncEverything READ syncEverything WRITE setSyncEverything NOTIFY driveChanged)

public:
    explicit AccountConfiguredWizardPage(
        const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir, bool vfsIsAvailable, bool enableVfsByDefault);
    ~AccountConfiguredWizardPage() noexcept override;

    QString syncTargetDir() const;

    SyncMode syncMode() const;

    bool validateInput() override;

    void setShowAdvancedSettings(bool showAdvancedSettings);

    QString folder() const;
    QString freeSpaceText() const;
    bool syncEverything() const;
    void setSyncEverything(bool everything);
    Q_INVOKABLE void chooseFolder();

Q_SIGNALS:
    void driveChanged();

private:
    ::Ui::AccountConfiguredWizardPage *_ui;
};

}
