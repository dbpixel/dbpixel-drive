/*
 * Painel da bandeja do Drive (um produto DBPixel): abre no clique do ícone ao lado do relógio, no lugar da janela de
 * configurações, com a situação da sincronização, os arquivos recentes, o espaço usado e os atalhos.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "gui/owncloudguilib.h"

#include "libsync/progressdispatcher.h"
#include "libsync/syncfileitem.h"

#include <QPointer>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantList>
#include <QWidget>

namespace OCC {

class Folder;
class ownCloudGui;
class QuotaInfo;
namespace QmlUtils {
    class OCQuickWidget;
}

class OWNCLOUDGUI_EXPORT TrayPanel : public QWidget
{
    Q_OBJECT
    // ok, sync, problem, paused, offline, signedout, empty
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString detail READ detail NOTIFY changed)
    // 0..1, ou -1 quando não há envio/download em andamento
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(QString accountName READ accountName NOTIFY changed)
    Q_PROPERTY(QString accountInitials READ accountInitials NOTIFY changed)
    Q_PROPERTY(QString quotaText READ quotaText NOTIFY changed)
    Q_PROPERTY(QString quotaPercentText READ quotaPercentText NOTIFY changed)
    Q_PROPERTY(double quotaFraction READ quotaFraction NOTIFY changed)
    Q_PROPERTY(bool paused READ paused NOTIFY changed)
    // cada item: name, folder, ext, status, up (bool), progress (-1 ou 0..1), time, path
    Q_PROPERTY(QVariantList current READ current NOTIFY changed)
    Q_PROPERTY(QVariantList recent READ recent NOTIFY changed)
    // cada problema: name, folder, ext, message
    Q_PROPERTY(QVariantList problems READ problems NOTIFY changed)
    QML_ELEMENT
    QML_UNCREATABLE("C++")

public:
    explicit TrayPanel(ownCloudGui *gui);

    QString state() const { return _state; }
    QString title() const { return _title; }
    QString detail() const { return _detail; }
    double progress() const { return _progress; }
    QString accountName() const;
    QString accountInitials() const;
    QString quotaText() const;
    QString quotaPercentText() const;
    double quotaFraction() const;
    bool paused() const;
    QVariantList current() const { return _current; }
    QVariantList recent() const { return _recent; }
    QVariantList problems() const { return _problems; }

    /// Abre o painel junto ao ícone da bandeja, ou fecha se já estiver aberto.
    void toggle(const QRect &trayGeometry);

    Q_INVOKABLE void openFolder();
    Q_INVOKABLE void openFile(const QString &path);
    Q_INVOKABLE void openWeb();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void signIn();
    Q_INVOKABLE void openSettings();
    Q_INVOKABLE void openIssues();
    Q_INVOKABLE void openHelp();
    Q_INVOKABLE void quit();

Q_SIGNALS:
    void changed();

protected:
    bool event(QEvent *e) override;

private:
    void refresh();
    void onProgress(Folder *folder, const ProgressInfo &progress);
    void onItemCompleted(Folder *folder, const SyncFileItemPtr &item);
    void connectQuota();
    static QString timeText(const QDateTime &t);

    ownCloudGui *_gui;
    QmlUtils::OCQuickWidget *_view;
    QPointer<QuotaInfo> _quota;
    QTimer _refreshTimer;

    QString _state;
    QString _title;
    QString _detail;
    double _progress = -1;
    QVariantList _current;
    QVariantList _recent;
    QVariantList _problems;
    bool _hasProgress = false;
};

}
