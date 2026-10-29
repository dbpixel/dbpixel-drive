/*
 * Drive: atualização automática, relatório de problema e dados da página "Sobre" (protótipo C5).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "gui/owncloudguilib.h"

#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>
#include <QtQmlIntegration/QtQmlIntegration>

class QNetworkAccessManager;
class QNetworkReply;

namespace OCC {

/**
 * Atualização sem ação do cliente: procura uma versão nova ao abrir e a cada 2 horas, baixa o instalador,
 * confere o SHA-256 e, quando não há sincronização em andamento, fecha o Drive, instala em silêncio e abre
 * de novo. O relatório de problema junta o registro dos últimos dias com o relato do cliente, sobe para a
 * nuvem dele (pasta "Relatórios do Drive"), cria um link e abre o WhatsApp do suporte com o link.
 */
class OWNCLOUDGUI_EXPORT DriveSupport : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("C++ only")
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(UpdateState updateState READ updateState NOTIFY updateChanged)
    Q_PROPERTY(QString updateText READ updateText NOTIFY updateChanged)
    Q_PROPERTY(double updateProgress READ updateProgress NOTIFY updateChanged)
    Q_PROPERTY(ReportState reportState READ reportState NOTIFY reportChanged)
    Q_PROPERTY(QString reportText READ reportText NOTIFY reportChanged)
    Q_PROPERTY(QString logFolder READ logFolder CONSTANT)

public:
    enum class UpdateState { Idle, Checking, UpToDate, Downloading, Ready, Installing, Failed };
    Q_ENUM(UpdateState)
    // nomes diferentes dos de UpdateState: no QML os dois enums ficam no mesmo escopo (DriveSupport.X)
    enum class ReportState { ReportIdle, ReportSending, ReportSent, ReportFailed };
    Q_ENUM(ReportState)

    static DriveSupport *instance();

    /// endereço do arquivo que descreve a última versão; DRIVE_UPDATE_URL troca (testes)
    static QUrl manifestUrl();
    static constexpr auto whatsAppNumber = "551340423797";

    QString version() const;
    UpdateState updateState() const { return _updateState; }
    QString updateText() const { return _updateText; }
    double updateProgress() const { return _updateProgress; }
    ReportState reportState() const { return _reportState; }
    QString reportText() const { return _reportText; }
    QString logFolder() const;

    /// começa as verificações periódicas (chamado uma vez ao abrir o Drive)
    void start();

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void sendReport(const QString &description);
    Q_INVOKABLE void openWhatsApp(const QString &text = QString());
    Q_INVOKABLE void openSite();
    Q_INVOKABLE void openSource();
    Q_INVOKABLE void openLogFolder();
    Q_INVOKABLE void copySupportInfo();

Q_SIGNALS:
    void updateChanged();
    void reportChanged();

private:
    explicit DriveSupport(QObject *parent = nullptr);

    void check(bool userAsked);
    void setUpdate(UpdateState state, const QString &text, double progress = -1);
    void setReport(ReportState state, const QString &text);
    void manifestArrived(QNetworkReply *reply, bool userAsked);
    void download(const QUrl &url, const QByteArray &sha256, const QVersionNumber &version);
    void tryInstall();
    void reportAfterRestart();

    QString supportInfo() const;
    void uploadReport(const QString &zipPath, const QString &name, const QString &description);
    void reportFinished(const QString &description, const QString &link, const QString &name);

    QNetworkAccessManager *_nam;
    QTimer _checkTimer;
    QTimer _installTimer;
    QPointer<QNetworkReply> _download;
    UpdateState _updateState = UpdateState::Idle;
    QString _updateText;
    double _updateProgress = -1;
    QString _installerPath;
    QVersionNumber _pendingVersion;
    QDateTime _readySince;

    ReportState _reportState = ReportState::ReportIdle;
    QString _reportText;
};

}
