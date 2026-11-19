/*
 * Drive: integração com o Explorador de Arquivos (Windows): atalho no painel lateral e coluna Status.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "gui/owncloudguilib.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QThreadPool>

#include <functional>
#include <memory>
#include <string>

namespace OCC {

class Folder;
class SyncFileStatus;

/**
 * Registra cada pasta sincronizada no Windows como pasta de nuvem (API de arquivos de nuvem), só com arquivos completos:
 * o Explorer mostra o Drive no painel lateral e a coluna Status (sincronizado / pendente), como o OneDrive. Nada fica
 * "somente online" e a sincronização continua igual; aqui só se atualiza o estado de cada arquivo.
 *
 * Se o Windows recusar o registro (rede, FAT32...), cai no atalho simples do painel lateral, sem status.
 * Tirar o registro devolve os arquivos ao estado comum; isso acontece ao remover a pasta ou ao desinstalar.
 */
class ExplorerPane : public QObject
{
    Q_OBJECT
public:
    explicit ExplorerPane(QObject *parent = nullptr);
    ~ExplorerPane() override;

    /// GUID fixo do atalho simples (o desinstalador usa o mesmo)
    static constexpr auto clsid = "{6D1A3B5E-8C2F-4E7A-9B1D-2F4C6E8A0B13}";

    /// Usado pelo desinstalador (--drive-remover-explorer): tira o Drive do Explorer
    OWNCLOUDGUI_EXPORT static void unregisterAll();

    void update();

private:
    struct Root
    {
        QPointer<Folder> folder;
        std::wstring id;
        std::wstring path;
        long long connectionKey = 0;
        bool swept = false;
        /// lido e escrito só na thread de trabalho: o registro deu certo e a pasta ainda está ativa
        std::shared_ptr<bool> active;
    };

    void addFolder(Folder *folder);
    void removeFolder(Folder *folder);
    void onStatus(const QString &systemFileName, SyncFileStatus status);
    void sweep(Root &root);
    void run(std::function<void()> &&task);

    static void registerClassic(const QString &path);
    static void unregisterClassic();

    QHash<QByteArray, Root> _roots;
    /// uma única thread, em ordem: registro, varredura e mudanças de estado nunca se cruzam
    QThreadPool _worker;
};

}
