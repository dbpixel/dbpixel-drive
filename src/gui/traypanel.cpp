/*
 * Painel da bandeja do Drive (um produto DBPixel).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui/traypanel.h"

#include "gui/accountmanager.h"
#include "gui/accountstate.h"
#include "gui/application.h"
#include "gui/folderman.h"
#include "gui/openfilemanager.h"
#include "gui/owncloudgui.h"
#include "gui/qmlutils.h"
#include "gui/quotainfo.h"
#include "gui/settingsdialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QScreen>
#include <QVBoxLayout>

using namespace OCC;

namespace {
// o suporte da DBPixel no WhatsApp: (13) 4042-3797
const auto helpWhatsApp = QStringLiteral("https://wa.me/551340423797");
const int maxRecent = 8;

AccountStatePtr firstAccount()
{
    const auto accounts = AccountManager::instance()->accounts();
    return accounts.isEmpty() ? AccountStatePtr() : accounts.first();
}

QVariantMap fileEntry(Folder *folder, const SyncFileItem &item)
{
    const QFileInfo fi(item.destination());
    QString dir = fi.path();
    if (dir == QLatin1String(".")) {
        dir = folder ? folder->shortGuiLocalPath() : QString();
    }
    return {
        {QStringLiteral("name"), fi.fileName()},
        {QStringLiteral("folder"), dir.replace(QLatin1Char('/'), QStringLiteral(" / "))},
        {QStringLiteral("ext"), fi.suffix().toLower()},
        {QStringLiteral("up"), item._direction == SyncFileItem::Up},
        {QStringLiteral("path"), folder ? folder->path() + item.destination() : QString()},
    };
}
}

TrayPanel::TrayPanel(ownCloudGui *gui)
    : QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint)
    , _gui(gui)
    , _view(new QmlUtils::OCQuickWidget(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle(QStringLiteral("Drive"));
    setFixedSize(404, 600);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    _view->setAttribute(Qt::WA_AlwaysStackOnTop);
    _view->setClearColor(Qt::transparent);
    _view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    layout->addWidget(_view);
    _view->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/TrayPanel.qml")), this);

    // a situação muda por vários caminhos; um temporizador curto junta as atualizações
    _refreshTimer.setSingleShot(true);
    _refreshTimer.setInterval(150);
    connect(&_refreshTimer, &QTimer::timeout, this, &TrayPanel::refresh);
    auto later = [this] { _refreshTimer.start(); };

    connect(ProgressDispatcher::instance(), &ProgressDispatcher::progressInfo, this, &TrayPanel::onProgress);
    connect(ProgressDispatcher::instance(), &ProgressDispatcher::itemCompleted, this, &TrayPanel::onItemCompleted);
    connect(FolderMan::instance(), &FolderMan::folderSyncStateChange, this, later);
    connect(FolderMan::instance(), &FolderMan::folderListChanged, this, later);
    connect(AccountManager::instance(), &AccountManager::accountAdded, this, [this, later] {
        connectQuota();
        later();
    });
    connect(AccountManager::instance(), &AccountManager::accountRemoved, this, later);
    connectQuota();
    refresh();
}

void TrayPanel::connectQuota()
{
    if (_quota) {
        return;
    }
    if (auto account = firstAccount()) {
        _quota = account->quotaInfo();
        if (_quota) {
            connect(_quota, &QuotaInfo::quotaUpdated, this, [this] { Q_EMIT changed(); });
        }
        connect(account.data(), &AccountState::stateChanged, this, [this] { _refreshTimer.start(); });
    }
}

QString TrayPanel::accountName() const
{
    const auto account = firstAccount();
    return account ? account->account()->davDisplayName() : QString();
}

QString TrayPanel::accountInitials() const
{
    const auto parts = accountName().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QString out;
    for (const auto &p : parts) {
        out += p.at(0).toUpper();
        if (out.size() == 2) {
            break;
        }
    }
    return out;
}

QString TrayPanel::quotaText() const
{
    if (!_quota || _quota->lastQuotaUsedBytes() <= 0) {
        return {};
    }
    const QLocale l;
    const auto used = l.formattedDataSize(_quota->lastQuotaUsedBytes(), 1, QLocale::DataSizeTraditionalFormat);
    if (_quota->lastQuotaTotalBytes() > 0) {
        return QStringLiteral("%1 de %2").arg(used, l.formattedDataSize(_quota->lastQuotaTotalBytes(), 0, QLocale::DataSizeTraditionalFormat));
    }
    return QStringLiteral("%1 usados").arg(used);
}

double TrayPanel::quotaFraction() const
{
    if (!_quota || _quota->lastQuotaTotalBytes() <= 0) {
        return -1;
    }
    return qBound(0.0, double(_quota->lastQuotaUsedBytes()) / double(_quota->lastQuotaTotalBytes()), 1.0);
}

QString TrayPanel::quotaPercentText() const
{
    const auto f = quotaFraction();
    return f < 0 ? QString() : QStringLiteral("%1% usado").arg(qRound(f * 100));
}

bool TrayPanel::paused() const
{
    const auto &folders = FolderMan::instance()->folders();
    if (folders.isEmpty()) {
        return false;
    }
    return std::all_of(folders.cbegin(), folders.cend(), [](Folder *f) { return f->syncPaused(); });
}

QString TrayPanel::timeText(const QDateTime &t)
{
    const QLocale l;
    const auto today = QDate::currentDate();
    if (t.date() == today) {
        return l.toString(t.time(), QStringLiteral("HH:mm"));
    }
    if (t.date() == today.addDays(-1)) {
        return QStringLiteral("ontem");
    }
    return l.toString(t.date(), QStringLiteral("dd/MM"));
}

void TrayPanel::onProgress(Folder *folder, const ProgressInfo &progress)
{
    // pausado: o que ainda chega da sincronização interrompida não volta a mostrar "Sincronizando"
    if (folder && folder->syncPaused()) {
        _hasProgress = false;
        _current.clear();
        _refreshTimer.start();
        return;
    }
    // arquivos em andamento, com o percentual de cada um
    QVariantList cur;
    for (const auto &p : progress._currentItems) {
        if (p._item.isDirectory()) {
            continue;
        }
        auto e = fileEntry(folder, p._item);
        const auto total = p._progress.completed() + p._progress.remaining();
        e[QStringLiteral("progress")] = total > 0 ? double(p._progress.completed()) / double(total) : 0.0;
        e[QStringLiteral("status")] = QStringLiteral("%1%").arg(total > 0 ? qRound(100.0 * p._progress.completed() / total) : 0);
        cur.append(e);
        if (cur.size() == 3) {
            break;
        }
    }
    _current = cur;

    const QLocale l;
    if (progress.totalSize() > 0 && progress.status() == ProgressInfo::Propagation) {
        _progress = qBound(0.0, double(progress.completedSize()) / double(progress.totalSize()), 1.0);
        const auto left = qMax<qint64>(0, progress.totalFiles() - progress.completedFiles());
        _hasProgress = true;
        _title = left == 1 ? QStringLiteral("Sincronizando 1 arquivo") : QStringLiteral("Sincronizando %1 arquivos").arg(left);
        QString eta;
        const auto secs = progress.optimisticEta() / 1000;
        if (secs > 0) {
            eta = secs < 60 ? QStringLiteral(" · menos de 1 minuto") : QStringLiteral(" · cerca de %1 min").arg((secs + 59) / 60);
        }
        _detail = QStringLiteral("%1 de %2%3")
                      .arg(l.formattedDataSize(progress.completedSize(), 1, QLocale::DataSizeTraditionalFormat),
                          l.formattedDataSize(progress.totalSize(), 1, QLocale::DataSizeTraditionalFormat), eta);
        _state = QStringLiteral("sync");
        Q_EMIT changed();
    }
    if (progress.status() == ProgressInfo::Done) {
        _hasProgress = false;
        _current.clear();
        _refreshTimer.start();
    }
}

void TrayPanel::onItemCompleted(Folder *folder, const SyncFileItemPtr &item)
{
    if (!item || item->isDirectory() || item->instruction() == CSYNC_INSTRUCTION_NONE) {
        return;
    }
    const auto path = item->destination();
    auto samePath = [path](const QVariant &v) { return v.toMap().value(QStringLiteral("rel")).toString() == path; };
    _problems.erase(std::remove_if(_problems.begin(), _problems.end(), samePath), _problems.end());
    _recent.erase(std::remove_if(_recent.begin(), _recent.end(), samePath), _recent.end());

    auto e = fileEntry(folder, *item);
    e[QStringLiteral("rel")] = path;
    if (item->hasErrorStatus() || item->_status == SyncFileItem::Conflict) {
        e[QStringLiteral("message")] = item->_errorString.isEmpty() ? QStringLiteral("Não foi possível sincronizar este arquivo.") : item->_errorString;
        _problems.prepend(e);
    } else if (item->instruction() != CSYNC_INSTRUCTION_REMOVE) {
        e[QStringLiteral("status")] = item->_direction == SyncFileItem::Up ? QStringLiteral("Enviado") : QStringLiteral("Baixado");
        e[QStringLiteral("time")] = timeText(QDateTime::currentDateTime());
        e[QStringLiteral("progress")] = -1;
        _recent.prepend(e);
        while (_recent.size() > maxRecent) {
            _recent.removeLast();
        }
    }
    Q_EMIT changed();
}

void TrayPanel::refresh()
{
    const auto account = firstAccount();
    const auto &folders = FolderMan::instance()->folders();
    if (!account) {
        _state = QStringLiteral("empty");
        _title = QStringLiteral("Nenhuma conta conectada");
        _detail = QStringLiteral("Abra as configurações para entrar");
    } else if (account->isSignedOut()) {
        _state = QStringLiteral("signedout");
        _title = QStringLiteral("Entre de novo para continuar");
        _detail = QStringLiteral("Seus arquivos neste PC estão seguros");
    } else if (!account->isConnected()) {
        _state = QStringLiteral("offline");
        _title = QStringLiteral("Sem conexão com o Drive");
        _detail = QStringLiteral("Tentando de novo sozinho, assim que a rede voltar");
    } else if (paused()) {
        _state = QStringLiteral("paused");
        _title = QStringLiteral("Sincronização pausada");
        _detail = QStringLiteral("As alterações ficam esperando até você retomar");
    } else if (_hasProgress && FolderMan::instance()->isAnySyncRunning()) {
        return; // onProgress já mantém título e detalhe
    } else if (folders.isEmpty()) {
        _state = QStringLiteral("empty");
        _title = QStringLiteral("Nenhuma pasta sincronizada");
        _detail = QStringLiteral("Adicione uma pasta nas configurações");
    } else if (!_problems.isEmpty()) {
        _state = QStringLiteral("problem");
        _title = _problems.size() == 1 ? QStringLiteral("1 arquivo não sincronizou") : QStringLiteral("%1 arquivos não sincronizaram").arg(_problems.size());
        _detail = QStringLiteral("O resto está em dia");
    } else if (FolderMan::instance()->isAnySyncRunning()) {
        _state = QStringLiteral("sync");
        _title = QStringLiteral("Verificando alterações");
        _detail = QStringLiteral("Comparando este PC com a nuvem");
    } else {
        const auto overall = FolderMan::trayOverallStatus(folders);
        _state = QStringLiteral("ok");
        _title = QStringLiteral("Seus arquivos estão em dia");
        _detail = overall.lastSyncDone.isValid() ? QStringLiteral("Sincronizado %1 às %2")
                                                      .arg(overall.lastSyncDone.date() == QDate::currentDate() ? QStringLiteral("hoje") : timeText(overall.lastSyncDone),
                                                          QLocale().toString(overall.lastSyncDone.time(), QStringLiteral("HH:mm")))
                                                : QStringLiteral("Tudo sincronizado");
    }
    if (_state != QLatin1String("sync")) {
        _progress = -1;
    }
    Q_EMIT changed();
}

void TrayPanel::toggle(const QRect &tray)
{
    if (isVisible()) {
        hide();
        return;
    }
    refresh();
    if (_quota) {
        _quota->setActive(true);
    }
    // ancora no canto da área de trabalho onde fica o ícone (normalmente embaixo, à direita)
    QScreen *screen = tray.isValid() ? QGuiApplication::screenAt(tray.center()) : nullptr;
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect area = screen->availableGeometry();
    const int margin = 12;
    QPoint pos(area.right() - width() - margin, area.bottom() - height() - margin);
    if (tray.isValid()) {
        pos.setX(qBound(area.left() + margin, tray.center().x() - width() / 2, area.right() - width() - margin));
        if (tray.center().y() < area.center().y()) {
            pos.setY(area.top() + margin); // barra de tarefas no alto da tela
        }
    }
    move(pos);
    show();
    raise();
    activateWindow();
}

bool TrayPanel::event(QEvent *e)
{
    // fecha ao clicar fora, como os painéis do Windows
    if (e->type() == QEvent::WindowDeactivate) {
        hide();
        if (_quota) {
            _quota->setActive(false);
        }
    }
    return QWidget::event(e);
}

void TrayPanel::openFolder()
{
    const auto &folders = FolderMan::instance()->folders();
    if (!folders.isEmpty()) {
        _gui->slotFolderOpenAction(folders.first());
    }
    hide();
}

void TrayPanel::openFile(const QString &path)
{
    if (!path.isEmpty()) {
        showInFileManager(path);
    }
    hide();
}

void TrayPanel::openWeb()
{
    if (auto account = firstAccount()) {
        QDesktopServices::openUrl(account->account()->url());
    }
    hide();
}

void TrayPanel::togglePause()
{
    const bool pause = !paused();
    for (auto *f : FolderMan::instance()->folders()) {
        f->setSyncPaused(pause);
        // marcar como pausada não para o que já está rodando: interrompe a sincronização em andamento
        if (pause) {
            f->slotTerminateSync(QStringLiteral("Sincronização pausada pelo usuário"));
        }
    }
    _refreshTimer.start();
}

void TrayPanel::signIn()
{
    if (auto account = firstAccount()) {
        account->signIn();
    }
    hide();
}

void TrayPanel::openSettings()
{
    hide();
    _gui->slotShowSettings();
}

void TrayPanel::openIssues()
{
    hide();
    _gui->slotShowSettings();
    _gui->settingsDialog()->setCurrentPage(SettingsDialog::SettingsPage::Activity);
}

void TrayPanel::openHelp()
{
    QDesktopServices::openUrl(QUrl(helpWhatsApp));
    hide();
}

void TrayPanel::quit()
{
    hide();
    qApp->quit();
}
