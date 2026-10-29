/*
 * Drive: atualização automática, relatório de problema e dados da página "Sobre".
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui/drivesupport.h"

#include "account.h"
#include "accountmanager.h"
#include "application.h"
#include "common/version.h"
#include "configfile.h"
#include "folderman.h"
#include "logger.h"
#include "networkjobs.h"
#include "ocssharejob.h"
#include "owncloudgui.h"
#include "sharepermissions.h"

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUrlQuery>

using namespace std::chrono_literals;

Q_LOGGING_CATEGORY(lcDriveSupport, "gui.drive.support", QtInfoMsg)

namespace OCC {

namespace {
    const auto attemptKeyC = QStringLiteral("DriveUpdate/attemptedVersion");
    const auto failedKeyC = QStringLiteral("DriveUpdate/failed/");
    const auto reportFolderC = QStringLiteral("Relatórios do Drive");
    // o registro de depuração cresce rápido: do arquivo atual vai só o final
    constexpr qint64 maxLogBytesC = 40 * 1024 * 1024;

    QString psQuote(QString s)
    {
        s.replace(QLatin1Char('\''), QStringLiteral("''"));
        return QLatin1Char('\'') + QDir::toNativeSeparators(s) + QLatin1Char('\'');
    }

    AccountStatePtr firstAccount()
    {
        const auto accounts = AccountManager::instance()->accounts();
        return accounts.isEmpty() ? AccountStatePtr() : accounts.first();
    }

    QString updatesDir()
    {
        return QDir::temp().absoluteFilePath(QStringLiteral("DBPixelDrive-atualizacao"));
    }
}

DriveSupport *DriveSupport::instance()
{
    static auto *s = new DriveSupport(qApp);
    return s;
}

QUrl DriveSupport::manifestUrl()
{
    const auto env = qEnvironmentVariable("DRIVE_UPDATE_URL");
    if (!env.isEmpty()) {
        return QUrl(env);
    }
    // a última versão publicada no GitHub (o código-fonte fica aberto por causa da GPL)
    return QUrl(QStringLiteral("https://github.com/dbpixel/dbpixel-drive/releases/latest/download/drive-update.json"));
}

DriveSupport::DriveSupport(QObject *parent)
    : QObject(parent)
    , _nam(new QNetworkAccessManager(this))
{
    _nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    _checkTimer.setInterval(2h);
    connect(&_checkTimer, &QTimer::timeout, this, [this] { check(false); });
    // instala assim que a sincronização em andamento terminar
    _installTimer.setInterval(30s);
    connect(&_installTimer, &QTimer::timeout, this, &DriveSupport::tryInstall);
}

QString DriveSupport::version() const
{
    return Version::displayString();
}

QString DriveSupport::logFolder() const
{
    return QDir::toNativeSeparators(Logger::instance()->temporaryFolderLogDirPath());
}

void DriveSupport::start()
{
    reportAfterRestart();
    QTimer::singleShot(20s, this, [this] { check(false); });
    _checkTimer.start();
}

void DriveSupport::setUpdate(UpdateState state, const QString &text, double progress)
{
    _updateState = state;
    _updateText = text;
    _updateProgress = progress;
    Q_EMIT updateChanged();
}

void DriveSupport::setReport(ReportState state, const QString &text)
{
    _reportState = state;
    _reportText = text;
    Q_EMIT reportChanged();
}

void DriveSupport::checkForUpdates()
{
    check(true);
}

void DriveSupport::check(bool userAsked)
{
    if (_updateState == UpdateState::Checking || _updateState == UpdateState::Downloading || _updateState == UpdateState::Ready
        || _updateState == UpdateState::Installing) {
        return;
    }
    const auto url = manifestUrl();
    qCInfo(lcDriveSupport) << "Procurando atualização em" << url;
    if (userAsked) {
        setUpdate(UpdateState::Checking, QStringLiteral("Procurando uma versão nova…"));
    }
    QUrl withParams = url;
    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("atual"), Version::versionWithBuildNumber().toString());
    withParams.setQuery(query);
    QNetworkRequest req(withParams);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    req.setTransferTimeout(30000);
    auto *reply = _nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, userAsked] { manifestArrived(reply, userAsked); });
}

void DriveSupport::manifestArrived(QNetworkReply *reply, bool userAsked)
{
    reply->deleteLater();
    const auto failed = [&](const QString &why) {
        qCWarning(lcDriveSupport) << "Não foi possível verificar atualizações:" << why;
        if (userAsked) {
            setUpdate(UpdateState::Failed, QStringLiteral("Não deu para procurar atualizações agora. O Drive tenta de novo sozinho mais tarde."));
        } else if (_updateState == UpdateState::Checking) {
            setUpdate(UpdateState::Idle, QString());
        }
    };
    if (reply->error() != QNetworkReply::NoError) {
        failed(reply->errorString());
        return;
    }
    QJsonParseError err;
    auto body = reply->readAll();
    if (body.startsWith(QByteArray::fromHex("EFBBBF"))) {
        body.remove(0, 3);
    }
    const auto json = QJsonDocument::fromJson(body, &err).object();
    const auto remote = QVersionNumber::fromString(json.value(QStringLiteral("version")).toString());
    const QUrl url(json.value(QStringLiteral("url")).toString());
    const auto sha = json.value(QStringLiteral("sha256")).toString().toLatin1().toLower();
    if (err.error != QJsonParseError::NoError || remote.isNull() || !url.isValid() || sha.size() != 64) {
        failed(QStringLiteral("descrição da versão inválida"));
        return;
    }
    const auto current = Version::versionWithBuildNumber();
    qCInfo(lcDriveSupport) << "Versão instalada" << current << "publicada" << remote;
    if (remote <= current) {
        setUpdate(UpdateState::UpToDate, QStringLiteral("Você está com a versão mais recente."));
        return;
    }
    // uma versão que não instalou duas vezes não é tentada de novo sozinha (evita fechar o Drive sem parar)
    const auto settings = ConfigFile::makeQSettings();
    if (!userAsked && settings.value(failedKeyC + remote.toString(), 0).toInt() >= 2) {
        qCWarning(lcDriveSupport) << "A versão" << remote << "falhou ao instalar antes; esperando uma versão nova";
        setUpdate(UpdateState::Failed, QStringLiteral("A versão %1 não instalou neste computador. Fale com o suporte.").arg(remote.toString()));
        return;
    }
    download(url, sha, remote);
}

void DriveSupport::download(const QUrl &url, const QByteArray &sha256, const QVersionNumber &version)
{
    QDir().mkpath(updatesDir());
    const auto target = QDir(updatesDir()).absoluteFilePath(QStringLiteral("Drive-%1-setup.exe").arg(version.toString()));

    const auto hashOf = [](const QString &path) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            return QByteArray();
        }
        QCryptographicHash h(QCryptographicHash::Sha256);
        h.addData(&f);
        return h.result().toHex();
    };
    const auto ready = [this, target, version] {
        _installerPath = target;
        _pendingVersion = version;
        _readySince = QDateTime::currentDateTimeUtc();
        setUpdate(UpdateState::Ready, QStringLiteral("A versão %1 foi baixada. O Drive instala sozinho quando terminar de sincronizar.").arg(version.toString()));
        _installTimer.start();
        tryInstall();
    };
    if (QFileInfo::exists(target) && hashOf(target) == sha256) {
        ready();
        return;
    }

    setUpdate(UpdateState::Downloading, QStringLiteral("Baixando a versão %1…").arg(version.toString()), 0);
    QNetworkRequest req(url);
    req.setTransferTimeout(120000);
    _download = _nam->get(req);
    connect(_download, &QNetworkReply::downloadProgress, this, [this, version](qint64 got, qint64 total) {
        if (total > 0) {
            setUpdate(UpdateState::Downloading, QStringLiteral("Baixando a versão %1… %2%").arg(version.toString()).arg(got * 100 / total), double(got) / total);
        }
    });
    connect(_download, &QNetworkReply::finished, this, [this, target, sha256, hashOf, ready, reply = _download.data()] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(lcDriveSupport) << "Falha ao baixar a atualização:" << reply->errorString();
            setUpdate(UpdateState::Failed, QStringLiteral("Não deu para baixar a versão nova agora. O Drive tenta de novo sozinho."));
            return;
        }
        QFile f(target + QStringLiteral(".part"));
        if (!f.open(QIODevice::WriteOnly) || f.write(reply->readAll()) < 0) {
            setUpdate(UpdateState::Failed, QStringLiteral("Não deu para salvar a versão nova (disco cheio?)."));
            return;
        }
        f.close();
        if (hashOf(f.fileName()) != sha256) {
            qCWarning(lcDriveSupport) << "Instalador baixado não confere com o SHA-256 publicado; descartado";
            f.remove();
            setUpdate(UpdateState::Failed, QStringLiteral("O arquivo baixado veio corrompido. O Drive tenta de novo sozinho."));
            return;
        }
        QFile::remove(target);
        f.rename(target);
        ready();
    });
}

void DriveSupport::tryInstall()
{
    if (_updateState != UpdateState::Ready || _installerPath.isEmpty()) {
        _installTimer.stop();
        return;
    }
    // não interrompe uma sincronização nem uma janela aberta pelo usuário; depois de 4 horas esperando,
    // instala mesmo assim (a sincronização continua de onde parou quando o Drive abre de novo)
    const bool waitedTooLong = _readySince.secsTo(QDateTime::currentDateTimeUtc()) > 4 * 3600;
    if (QApplication::activeModalWidget() || (FolderMan::instance()->isAnySyncRunning() && !waitedTooLong)) {
        return;
    }
    _installTimer.stop();

    auto settings = ConfigFile::makeQSettings();
    settings.setValue(attemptKeyC, _pendingVersion.toString());
    settings.sync();

    const auto exe = QCoreApplication::applicationFilePath();
    const auto localPrograms = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const bool perUser = QDir::fromNativeSeparators(exe).startsWith(QDir::fromNativeSeparators(localPrograms), Qt::CaseInsensitive);
    const auto scope = perUser ? QStringLiteral("/CurrentUser") : QStringLiteral("/AllUsers");
    qCInfo(lcDriveSupport) << "Instalando a versão" << _pendingVersion << "de" << _installerPath << scope;

    // o Drive fecha; o instalador só começa quando o processo terminou de verdade, roda em silêncio (com uma
    // segunda tentativa) e o Drive abre de novo. O resultado fica em instalacao.log para o próximo início.
    // (só aspas simples no script: o Windows repassa a linha de comando para o PowerShell)
    const auto log = psQuote(QDir(updatesDir()).absoluteFilePath(QStringLiteral("instalacao.log")));
    const auto command = QStringLiteral(
        "& { $log = %4; ('inicio ' + (Get-Date -Format o)) | Out-File $log -Append -Encoding utf8; "
        "Wait-Process -Id %5 -Timeout 90 -ErrorAction SilentlyContinue; Stop-Process -Id %5 -Force -ErrorAction SilentlyContinue; Start-Sleep -Seconds 2; "
        "foreach ($i in 1, 2) { $p = Start-Process -FilePath %1 -ArgumentList '/S','%2' -Wait -PassThru; "
        "('tentativa ' + $i + ' codigo ' + $p.ExitCode + ' ' + (Get-Date -Format o)) | Out-File $log -Append -Encoding utf8; "
        "if ($p.ExitCode -eq 0) { break }; Start-Sleep -Seconds 10 }; "
        "Start-Process -FilePath %3 }")
                             .arg(psQuote(_installerPath), scope, psQuote(exe), log, QString::number(QCoreApplication::applicationPid()));
    if (!QProcess::startDetached(QStringLiteral("powershell.exe"),
            {QStringLiteral("-NoProfile"), QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"), QStringLiteral("-WindowStyle"), QStringLiteral("Hidden"),
                QStringLiteral("-Command"), command})) {
        qCWarning(lcDriveSupport) << "Não foi possível iniciar o instalador";
        setUpdate(UpdateState::Failed, QStringLiteral("Não deu para instalar a versão nova. Fale com o suporte."));
        return;
    }
    setUpdate(UpdateState::Installing, QStringLiteral("Instalando a versão %1. O Drive abre de novo em instantes.").arg(_pendingVersion.toString()));
    QTimer::singleShot(800ms, qApp, &QApplication::quit);
}

void DriveSupport::reportAfterRestart()
{
    auto settings = ConfigFile::makeQSettings();
    const auto attempted = QVersionNumber::fromString(settings.value(attemptKeyC).toString());
    if (attempted.isNull()) {
        return;
    }
    settings.remove(attemptKeyC);
    {
        QFile log(QDir(updatesDir()).absoluteFilePath(QStringLiteral("instalacao.log")));
        if (log.open(QIODevice::ReadOnly)) {
            qCInfo(lcDriveSupport) << "Registro da instalação:" << QString::fromUtf8(log.readAll()).trimmed();
        }
    }
    if (Version::versionWithBuildNumber() >= attempted) {
        qCInfo(lcDriveSupport) << "Atualizado para a versão" << Version::versionWithBuildNumber();
        settings.remove(failedKeyC + attempted.toString());
        QDir(updatesDir()).removeRecursively();
        QTimer::singleShot(5s, this, [] {
            ocApp()->gui()->slotShowTrayMessage(QStringLiteral("Drive atualizado"),
                QStringLiteral("Agora você está com a versão %1.").arg(Version::displayString()));
        });
    } else {
        const auto key = failedKeyC + attempted.toString();
        settings.setValue(key, settings.value(key, 0).toInt() + 1);
        qCWarning(lcDriveSupport) << "A atualização para" << attempted << "não foi instalada; versão atual" << Version::versionWithBuildNumber();
    }
}

QString DriveSupport::supportInfo() const
{
    QStringList lines;
    lines << QStringLiteral("Drive %1 (%2)").arg(Version::displayString(), Version::gitSha().left(7));
    lines << QStringLiteral("Windows: %1 · %2").arg(QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture());
    lines << QStringLiteral("Computador: %1").arg(QSysInfo::machineHostName());
    if (const auto state = firstAccount()) {
        const auto account = state->account();
        lines << QStringLiteral("Conta: %1 em %2 (%3)").arg(account->davUser(), account->url().host(), state->isConnected() ? QStringLiteral("conectada") : QStringLiteral("sem conexão"));
    } else {
        lines << QStringLiteral("Conta: nenhuma configurada");
    }
    for (auto *f : FolderMan::instance()->folders()) {
        lines << QStringLiteral("Pasta: %1 -> %2 · %3%4")
                     .arg(QDir::toNativeSeparators(f->path()), f->remotePath(), f->isSyncRunning() ? QStringLiteral("sincronizando agora") : Utility::enumToDisplayName(f->syncResult().status()),
                         f->syncPaused() ? QStringLiteral(" · pausada") : QString());
    }
    return lines.join(QLatin1Char('\n'));
}

void DriveSupport::copySupportInfo()
{
    QApplication::clipboard()->setText(supportInfo());
}

void DriveSupport::openWhatsApp(const QString &text)
{
    QUrl url(QStringLiteral("https://wa.me/%1").arg(QLatin1String(whatsAppNumber)));
    if (!text.isEmpty()) {
        QUrlQuery q;
        q.addQueryItem(QStringLiteral("text"), QString::fromLatin1(QUrl::toPercentEncoding(text)));
        url.setQuery(q);
    }
    QDesktopServices::openUrl(url);
}

void DriveSupport::openSite()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://dbpixel.com.br")));
}

void DriveSupport::openSource()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/dbpixel/dbpixel-drive")));
}

void DriveSupport::openLogFolder()
{
    QDir().mkpath(logFolder());
    QDesktopServices::openUrl(QUrl::fromLocalFile(logFolder()));
}

void DriveSupport::sendReport(const QString &description)
{
    if (_reportState == ReportState::ReportSending) {
        return;
    }
    const auto text = description.trimmed();
    qCWarning(lcDriveSupport) << "Relatório de problema do cliente:" << text;
    // fica também um histórico neste computador
    {
        QFile log(ConfigFile::configPath() + QStringLiteral("relatorios.log"));
        if (log.open(QIODevice::Append | QIODevice::Text)) {
            log.write(QStringLiteral("[%1] %2\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate), QString(text).replace(QLatin1Char('\n'), QLatin1Char(' '))).toUtf8());
        }
    }

    const auto state = firstAccount();
    if (!state || !state->isConnected()) {
        setReport(ReportState::ReportFailed, QStringLiteral("Sem conexão com a nuvem agora. Fale com o suporte pelo WhatsApp e conte o que aconteceu."));
        return;
    }
    setReport(ReportState::ReportSending, QStringLiteral("Juntando o registro…"));

    const auto stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd-HHmmss"));
    const auto staging = QDir::temp().absoluteFilePath(QStringLiteral("DBPixelDrive-relatorio-%1").arg(stamp));
    QDir(staging).removeRecursively();
    QDir().mkpath(staging);

    {
        QFile f(QDir(staging).absoluteFilePath(QStringLiteral("relato.txt")));
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(QStringLiteral("O que aconteceu:\n%1\n\nEnviado em %2\n\n%3\n")
                        .arg(text.isEmpty() ? QStringLiteral("(sem descrição)") : text, QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm")), supportInfo())
                        .toUtf8());
        }
    }
    // os 4 registros mais recentes; do maior só o final
    QDir logs(Logger::instance()->temporaryFolderLogDirPath());
    const auto files = logs.entryInfoList(QDir::Files, QDir::Time);
    for (int i = 0; i < files.size() && i < 4; ++i) {
        QFile in(files[i].absoluteFilePath());
        QFile out(QDir(staging).absoluteFilePath(files[i].fileName()));
        if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly)) {
            if (in.size() > maxLogBytesC) {
                in.seek(in.size() - maxLogBytesC);
            }
            while (!in.atEnd()) {
                out.write(in.read(4 * 1024 * 1024));
            }
        }
    }
    const auto crash = QStringLiteral("%1/%2-crash.log").arg(QDir::tempPath(), qApp->applicationName());
    if (QFileInfo::exists(crash)) {
        QFile::copy(crash, QDir(staging).absoluteFilePath(QStringLiteral("crash.log")));
    }

    const auto user = state->account()->davUser();
    const auto name = QStringLiteral("relatorio-%1-%2.zip").arg(stamp, user);
    const auto zip = QDir::temp().absoluteFilePath(name);
    auto *zipper = new QProcess(this);
    connect(zipper, &QProcess::finished, this, [this, zipper, zip, name, text, staging](int code) {
        zipper->deleteLater();
        QDir(staging).removeRecursively();
        if (code != 0 || !QFileInfo::exists(zip)) {
            qCWarning(lcDriveSupport) << "Falha ao compactar o relatório" << zipper->readAllStandardError();
            setReport(ReportState::ReportFailed, QStringLiteral("Não deu para juntar o registro. Fale com o suporte pelo WhatsApp."));
            return;
        }
        uploadReport(zip, name, text);
    });
    zipper->start(QStringLiteral("powershell.exe"),
        {QStringLiteral("-NoProfile"), QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"), QStringLiteral("-Command"),
            QStringLiteral("Compress-Archive -Path %1 -DestinationPath %2 -CompressionLevel Optimal -Force").arg(psQuote(staging + QStringLiteral("/*")), psQuote(zip))});
}

void DriveSupport::uploadReport(const QString &zipPath, const QString &name, const QString &description)
{
    const auto state = firstAccount();
    if (!state) {
        setReport(ReportState::ReportFailed, QStringLiteral("Sem conta conectada."));
        return;
    }
    auto account = state->account();
    setReport(ReportState::ReportSending, QStringLiteral("Enviando o relatório para a sua nuvem…"));

    const auto put = [this, account, zipPath, name, description] {
        QFile f(zipPath);
        if (!f.open(QIODevice::ReadOnly)) {
            setReport(ReportState::ReportFailed, QStringLiteral("Não deu para ler o relatório."));
            return;
        }
        QNetworkRequest req;
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/zip"));
        auto *job = new SimpleNetworkJob(account, account->davUrl(), reportFolderC + QLatin1Char('/') + name, "PUT", f.readAll(), req, this);
        f.close();
        connect(job, &SimpleNetworkJob::finishedSignal, this, [this, job, account, zipPath, name, description] {
            const int status = job->httpStatusCode();
            if (status != 201 && status != 204) {
                qCWarning(lcDriveSupport) << "Envio do relatório falhou" << status;
                setReport(ReportState::ReportFailed, QStringLiteral("Não deu para enviar o relatório (erro %1). Fale com o suporte pelo WhatsApp.").arg(status));
                return;
            }
            QFile::remove(zipPath);
            // link para o suporte baixar, válido por 30 dias
            auto *share = OcsShareJob::createLinkShare(account, this, QLatin1Char('/') + reportFolderC + QLatin1Char('/') + name,
                QStringLiteral("Relatório para o suporte"), QString(), QDate::currentDate().addDays(30), SharePermissionRead);
            connect(share, &JsonApiJob::finishedSignal, this, [this, share, description, name] {
                QString link;
                if (share->ocsSuccess()) {
                    link = share->data().value(QStringLiteral("ocs")).toObject().value(QStringLiteral("data")).toObject().value(QStringLiteral("url")).toString();
                    // link de download direto: baixa o zip sem passar pela página do link público (que no servidor
                    // pode redirecionar para uma interface que não existe e mostrar "Arquivo não encontrado")
                    if (!link.isEmpty()) {
                        link += QStringLiteral("/download");
                    }
                } else {
                    qCWarning(lcDriveSupport) << "Link do relatório não foi criado" << share->ocsStatus() << share->ocsMessage();
                }
                reportFinished(description, link, name);
            });
            share->start();
        });
        job->start();
    };

    auto *mkcol = new MkColJob(account, account->davUrl(), reportFolderC, {}, this);
    // a pasta já existir (405) não é erro
    connect(mkcol, &MkColJob::finishedWithoutError, this, put);
    connect(mkcol, &MkColJob::finishedWithError, this, [this, put](QNetworkReply *reply) {
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 405) {
            put();
        } else {
            setReport(ReportState::ReportFailed, QStringLiteral("Não deu para criar a pasta do relatório na nuvem. Fale com o suporte pelo WhatsApp."));
        }
    });
    mkcol->start();
}

void DriveSupport::reportFinished(const QString &description, const QString &link, const QString &name)
{
    const auto state = firstAccount();
    const auto who = state ? QStringLiteral("%1 (%2)").arg(state->account()->davDisplayName(), state->account()->davUser()) : QString();
    QString message = QStringLiteral("Olá! Tive um problema no Drive.\n\n%1\n\n").arg(description.isEmpty() ? QStringLiteral("(sem descrição)") : description);
    if (!link.isEmpty()) {
        message += QStringLiteral("Relatório: %1\n").arg(link);
    } else {
        message += QStringLiteral("O relatório %1 está na pasta \"%2\" da minha nuvem.\n").arg(name, reportFolderC);
    }
    message += QStringLiteral("Conta: %1 · Versão %2").arg(who, Version::displayString());
    qCInfo(lcDriveSupport) << "Relatório enviado" << name << link;
    {
        QFile log(ConfigFile::configPath() + QStringLiteral("relatorios.log"));
        if (log.open(QIODevice::Append | QIODevice::Text)) {
            log.write(QStringLiteral("[%1] enviado %2 %3\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate), name, link).toUtf8());
        }
    }

    QApplication::clipboard()->setText(message);
    openWhatsApp(message);
    setReport(ReportState::ReportSent,
        QStringLiteral("Pronto. O relatório ficou na sua nuvem, em \"%1\", e o WhatsApp do suporte abriu com a mensagem pronta: é só enviar. A mensagem também foi copiada.")
            .arg(reportFolderC));
}

}
