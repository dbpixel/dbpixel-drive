/*
 * Drive: integração com o Explorador de Arquivos (Windows): atalho no painel lateral e coluna Status.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui/explorerpane_win.h"

#include "common/syncjournaldb.h"
#include "folder.h"
#include "folderman.h"
#include "gui/cloudfiles_win.h"
#include "libsync/syncengine.h"
#include "libsync/syncfilestatustracker.h"
#include "theme.h"

#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QSettings>

#include <atomic>
#include <memory>
#include <vector>

#include <shlobj.h>

Q_LOGGING_CATEGORY(lcExplorerPane, "gui.drive.explorerpane", QtInfoMsg)

namespace OCC {

namespace {
    const auto provider = L"DBPixelDrive";

    const QString classesKey()
    {
        return QStringLiteral("HKEY_CURRENT_USER\\Software\\Classes\\CLSID\\%1").arg(QLatin1String(ExplorerPane::clsid));
    }
    const QString namespaceKey()
    {
        return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\%1")
            .arg(QLatin1String(ExplorerPane::clsid));
    }
    const QString hideOnDesktopKey()
    {
        return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel");
    }

    QString fromW(const std::wstring &s)
    {
        return QString::fromStdWString(s);
    }

    /// Erros de estado se repetem arquivo a arquivo; registra só os primeiros para não encher o log
    void reportError(const std::wstring &path, const CloudFiles::Error &error)
    {
        static std::atomic<int> reported = 0;
        if (!error.empty() && reported++ < 20) {
            qCWarning(lcExplorerPane) << "Não foi possível atualizar o status no Explorer de" << fromW(path) << ":" << fromW(error);
        }
    }
}

ExplorerPane::ExplorerPane(QObject *parent)
    : QObject(parent)
{
    _worker.setMaxThreadCount(1);
    _worker.setExpiryTimeout(-1);
    connect(FolderMan::instance(), &FolderMan::folderListChanged, this, &ExplorerPane::update);
    connect(FolderMan::instance(), &FolderMan::folderRemoved, this, &ExplorerPane::removeFolder);
    update();
}

ExplorerPane::~ExplorerPane()
{
    _worker.clear();
    _worker.waitForDone();
    for (const auto &root : std::as_const(_roots)) {
        if (root.connectionKey) {
            CloudFiles::disconnect(root.connectionKey);
        }
    }
}

void ExplorerPane::unregisterAll()
{
    const int count = CloudFiles::unregisterAll(provider);
    qCInfo(lcExplorerPane) << "Pastas removidas do Explorer:" << count;
    unregisterClassic();
}

void ExplorerPane::run(std::function<void()> &&task)
{
    _worker.start(std::move(task));
}

void ExplorerPane::update()
{
    const auto &folders = FolderMan::instance()->folders();
    for (auto *folder : folders) {
        if (!_roots.contains(folder->id())) {
            addFolder(folder);
        }
    }
    // pastas descarregadas (ao sair ou ao trocar de conta) só desconectam; o registro continua
    for (auto it = _roots.begin(); it != _roots.end();) {
        if (!it->folder || !folders.contains(it->folder.data())) {
            const auto key = it->connectionKey;
            const auto active = it->active;
            run([key, active] {
                *active = false;
                if (key) {
                    CloudFiles::disconnect(key);
                }
            });
            it = _roots.erase(it);
        } else {
            ++it;
        }
    }
    if (folders.isEmpty()) {
        unregisterClassic();
    }
}

void ExplorerPane::addFolder(Folder *folder)
{
    Root root;
    root.folder = folder;
    root.path = QDir::toNativeSeparators(QDir::cleanPath(folder->path())).toStdWString();
    root.id = CloudFiles::syncRootId(provider, QString::fromUtf8(folder->id()).toStdWString());
    root.active = std::make_shared<bool>(false);

    auto name = Theme::instance()->appNameGUI();
    if (FolderMan::instance()->folders().size() > 1) {
        name += QStringLiteral(" - ") + folder->displayName();
    }
    CloudFiles::SyncRootInfo info;
    info.id = root.id;
    info.path = root.path;
    info.displayName = name.toStdWString();
    info.iconResource = QStringLiteral("%1,0").arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath())).toStdWString();
    info.version = QCoreApplication::applicationVersion().toStdWString();

    const auto folderId = folder->id();
    const auto active = root.active;
    run([this, info, folderId, active] {
        auto error = CloudFiles::registerSyncRoot(info);
        long long key = 0;
        if (error.empty()) {
            *active = true;
            const auto connectError = CloudFiles::connect(info.path, &key);
            if (!connectError.empty()) {
                // continua registrado (painel lateral), mas sem a coluna Status
                qCWarning(lcExplorerPane) << "Pasta registrada, mas sem conexão com o Explorer:" << fromW(connectError);
            }
        }
        QMetaObject::invokeMethod(
            this,
            [this, folderId, key, error, path = fromW(info.path)] {
                auto it = _roots.find(folderId);
                if (it != _roots.end()) {
                    it->connectionKey = key;
                } else if (key) {
                    CloudFiles::disconnect(key);
                }
                if (error.empty()) {
                    qCInfo(lcExplorerPane) << "Pasta registrada no Explorer com status:" << path;
                    unregisterClassic(); // o Windows já mostra a pasta no painel lateral
                } else {
                    qCWarning(lcExplorerPane) << "O Windows não aceitou a pasta como pasta de nuvem, usando o atalho simples:" << fromW(error);
                    registerClassic(path);
                }
            },
            Qt::QueuedConnection);
    });

    connect(&folder->syncEngine().syncFileStatusTracker(), &SyncFileStatusTracker::fileStatusChanged, this, &ExplorerPane::onStatus);
    connect(folder, &Folder::syncFinished, this, [this, folderId] {
        auto it = _roots.find(folderId);
        if (it != _roots.end() && !it->swept) {
            sweep(*it);
        }
    });
    _roots.insert(folderId, root);
}

void ExplorerPane::removeFolder(Folder *folder)
{
    // a pasta foi removida pela pessoa: tira o registro, e o Windows devolve os arquivos ao estado comum
    auto it = _roots.find(folder->id());
    if (it == _roots.end()) {
        return;
    }
    const auto key = it->connectionKey;
    const auto id = it->id;
    const auto active = it->active;
    _roots.erase(it);
    run([key, id, active] {
        *active = false;
        if (key) {
            CloudFiles::disconnect(key);
        }
        const auto error = CloudFiles::unregisterSyncRoot(id);
        if (!error.empty()) {
            qCWarning(lcExplorerPane) << "Não foi possível tirar a pasta do Explorer:" << fromW(error);
        }
    });
    if (_roots.isEmpty()) {
        unregisterClassic();
    }
}

void ExplorerPane::onStatus(const QString &systemFileName, SyncFileStatus status)
{
    bool inSync;
    switch (status.tag()) {
    case SyncFileStatus::StatusUpToDate:
        inSync = true;
        break;
    case SyncFileStatus::StatusSync:
    case SyncFileStatus::StatusWarning:
    case SyncFileStatus::StatusError:
        inSync = false;
        break;
    default:
        return; // ignorados e o banco de dados da sincronização ficam como estão
    }
    const auto path = QDir::toNativeSeparators(systemFileName).toStdWString();
    for (const auto &root : std::as_const(_roots)) {
        if (path == root.path || (path.size() > root.path.size() && path.compare(0, root.path.size(), root.path) == 0 && path[root.path.size()] == L'\\')) {
            const auto active = root.active;
            run([active, path, inSync] {
                if (*active) {
                    reportError(path, CloudFiles::markInSync(path, inSync));
                }
            });
            return;
        }
    }
}

void ExplorerPane::sweep(Root &root)
{
    // Depois da primeira sincronização, marca os arquivos que já estavam em dia (os que vieram de versões anteriores
    // ainda não são espaços reservados, e outros programas podem ter tirado a marca). Quem mudou desde a última
    // sincronização fica como está.
    root.swept = true;
    if (!root.folder) {
        return;
    }
    struct Entry
    {
        std::wstring path;
        bool isDirectory;
        qint64 modtime;
        qint64 size;
    };
    auto entries = std::make_shared<std::vector<Entry>>();
    const auto base = root.path + L"\\";
    auto &tracker = root.folder->syncEngine().syncFileStatusTracker();
    root.folder->journalDb()->getFilesBelowPath(QByteArray(), [&](const SyncJournalFileRecord &record) {
        // Só o que o Drive considera em dia; problemas, pendências e ignorados continuam como estão. "Nenhum" também
        // entra: é o que o rastreador devolve para atalhos .lnk (o Qt procura o destino do atalho, não o arquivo).
        const auto tag = tracker.fileStatus(QString::fromUtf8(record._path)).tag();
        if ((record.isDirectory() || record.isFile()) && (tag == SyncFileStatus::StatusUpToDate || tag == SyncFileStatus::StatusNone)) {
            entries->push_back(
                {base + QDir::toNativeSeparators(QString::fromUtf8(record._path)).toStdWString(), record.isDirectory(), record._modtime, record._fileSize});
        }
    });
    const auto active = root.active;
    run([entries, active, rootPath = root.path] {
        if (!*active) {
            return;
        }
        int converted = 0;
        int changed = 0;
        int failed = 0;
        for (const auto &entry : *entries) {
            // já marcado; um "pendente" que não mudou desde a sincronização foi mexido por fora (o Explorer, por exemplo)
            // e volta a ser marcado
            if (CloudFiles::state(entry.path) == CloudFiles::State::InSync) {
                continue;
            }
            if (!entry.isDirectory && !CloudFiles::unchanged(entry.path, entry.modtime, entry.size)) {
                ++changed;
                continue;
            }
            const auto error = CloudFiles::markInSync(entry.path, true);
            reportError(entry.path, error);
            error.empty() ? ++converted : ++failed;
        }
        const auto pinError = CloudFiles::pinAll(rootPath);
        if (!pinError.empty()) {
            qCWarning(lcExplorerPane) << "Não foi possível marcar a pasta como sempre disponível:" << fromW(pinError);
        }
        qCInfo(lcExplorerPane) << "Status no Explorer: itens marcados" << converted << "alterados desde a última sincronização" << changed << "falhas"
                               << failed << "de" << entries->size();
    });
}

void ExplorerPane::registerClassic(const QString &path)
{
    QSettings cls(classesKey(), QSettings::NativeFormat);
    if (cls.value(QStringLiteral("Instance/InitPropertyBag/TargetFolderPath")).toString() == path) {
        return; // já está certo
    }
    qCInfo(lcExplorerPane) << "Registrando o Drive no painel do Explorer para" << path;
    const auto name = Theme::instance()->appNameGUI();
    const auto exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    const auto shell32 = QDir::toNativeSeparators(QDir(qEnvironmentVariable("SystemRoot", QStringLiteral("C:\\Windows"))).filePath(QStringLiteral("System32/shell32.dll")));

    // pasta virtual que o Explorer mostra como uma pasta comum (a mesma técnica do OneDrive antes da API de nuvem)
    cls.setValue(QStringLiteral("Default"), name);
    cls.setValue(QStringLiteral("System.IsPinnedToNameSpaceTree"), 1);
    cls.setValue(QStringLiteral("SortOrderIndex"), 0x42);
    cls.setValue(QStringLiteral("DefaultIcon/Default"), QStringLiteral("%1,0").arg(exe));
    cls.setValue(QStringLiteral("InProcServer32/Default"), shell32);
    cls.setValue(QStringLiteral("InProcServer32/ThreadingModel"), QStringLiteral("Both"));
    cls.setValue(QStringLiteral("Instance/CLSID"), QStringLiteral("{0E5AAE11-A475-4c5b-AB00-C66DE400274E}"));
    cls.setValue(QStringLiteral("Instance/InitPropertyBag/Attributes"), 0x11);
    cls.setValue(QStringLiteral("Instance/InitPropertyBag/TargetFolderPath"), path);
    cls.setValue(QStringLiteral("ShellFolder/FolderValueFlags"), 0x28);
    cls.setValue(QStringLiteral("ShellFolder/Attributes"), static_cast<int>(0xF080004D));
    cls.sync();

    QSettings ns(namespaceKey(), QSettings::NativeFormat);
    ns.setValue(QStringLiteral("Default"), name);
    ns.sync();
    // não mostra o atalho na área de trabalho, só no painel lateral
    QSettings hide(hideOnDesktopKey(), QSettings::NativeFormat);
    hide.setValue(QLatin1String(clsid), 1);
    hide.sync();

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}

void ExplorerPane::unregisterClassic()
{
    QSettings cls(classesKey(), QSettings::NativeFormat);
    if (cls.childKeys().isEmpty() && cls.childGroups().isEmpty()) {
        return;
    }
    qCInfo(lcExplorerPane) << "Removendo o atalho simples do painel do Explorer";
    cls.remove(QString());
    QSettings ns(namespaceKey(), QSettings::NativeFormat);
    ns.remove(QString());
    QSettings hide(hideOnDesktopKey(), QSettings::NativeFormat);
    hide.remove(QLatin1String(clsid));
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}

}
