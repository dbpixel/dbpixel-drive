/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
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

#ifndef MIRALL_GENERALSETTINGS_H
#define MIRALL_GENERALSETTINGS_H

#include <QMap>
#include <QWidget>
#include <QPointer>

namespace OCC {
class IgnoreListEditor;
class SyncLogDialog;

namespace Ui {
    class GeneralSettings;
}

/**
 * @brief The GeneralSettings class
 * @ingroup gui
 */
class GeneralSettings : public QWidget
{
    Q_OBJECT
    // Drive (protótipo C2): a página é desenhada em QML (GeneralPage.qml) sobre os mesmos controles
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY driveChanged)
    Q_PROPERTY(bool autostartLocked READ autostartLocked NOTIFY driveChanged)
    Q_PROPERTY(bool notifications READ notifications WRITE setNotifications NOTIFY driveChanged)
    Q_PROPERTY(bool moveToTrash READ moveToTrash WRITE setMoveToTrash NOTIFY driveChanged)
    Q_PROPERTY(bool moveToTrashAvailable READ moveToTrashAvailable CONSTANT)
    Q_PROPERTY(bool syncHidden READ syncHidden WRITE setSyncHidden NOTIFY driveChanged)
    Q_PROPERTY(bool confirmDeletes READ confirmDeletes WRITE setConfirmDeletes NOTIFY driveChanged)
    Q_PROPERTY(QStringList languages READ languages NOTIFY driveChanged)
    Q_PROPERTY(int languageIndex READ languageIndex WRITE setLanguageIndex NOTIFY driveChanged)
    Q_PROPERTY(bool languageNeedsRestart READ languageNeedsRestart NOTIFY driveChanged)

public:
    bool autostart() const;
    void setAutostart(bool on);
    bool autostartLocked() const;
    bool notifications() const;
    void setNotifications(bool on);
    bool moveToTrash() const;
    void setMoveToTrash(bool on);
    bool moveToTrashAvailable() const;
    bool syncHidden() const;
    void setSyncHidden(bool on);
    bool confirmDeletes() const;
    void setConfirmDeletes(bool on);
    QStringList languages() const;
    int languageIndex() const;
    void setLanguageIndex(int index);
    bool languageNeedsRestart() const { return _languageNeedsRestart; }
    Q_INVOKABLE void editIgnoredFiles();
    Q_INVOKABLE void openLogFolder();

    explicit GeneralSettings(QWidget *parent = nullptr);
    ~GeneralSettings() override;

    /// Drive (protótipo C3): a parte de Rede vira uma página própria nas configurações
    QWidget *takeNetworkSettings();

Q_SIGNALS:
    void showAbout();
    void syncOptionsChanged();
    void driveChanged();

private Q_SLOTS:
    void saveMiscSettings();
    void slotToggleLaunchOnStartup(bool);
    void slotToggleOptionalDesktopNotifications(bool);
    void slotIgnoreFilesEditor();
    void loadMiscSettings();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void reloadConfig();
    void loadLanguageNamesIntoDropdown();

    Ui::GeneralSettings *_ui;
    QPointer<IgnoreListEditor> _ignoreEditor;
    bool _currentlyLoading;
    bool _languageNeedsRestart = false;
};


} // namespace OCC
#endif // MIRALL_GENERALSETTINGS_H
