/*
 * Drive: chamadas da API de arquivos de nuvem do Windows (cfapi) usadas para mostrar o status no Explorer.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <string>

namespace OCC::CloudFiles {

/// Resultado de uma chamada: vazio quando deu certo, senão a mensagem do erro
using Error = std::wstring;

struct SyncRootInfo
{
    std::wstring id; ///< "Provedor!SID!conta"
    std::wstring path; ///< pasta sincronizada
    std::wstring displayName;
    std::wstring iconResource; ///< "C:\...\Drive.exe,0"
    std::wstring version;
};

/// Monta o id no formato que o Windows espera: provedor!SID do usuário!conta
std::wstring syncRootId(const std::wstring &provider, const std::wstring &account);

/// Registra a pasta como raiz de sincronização só com arquivos completos (nada fica "somente online")
Error registerSyncRoot(const SyncRootInfo &info);
/// Tira o registro e devolve os arquivos ao estado comum (o Windows desfaz os espaços reservados)
Error unregisterSyncRoot(const std::wstring &id);
/// Tira todas as raízes do provedor; devolve quantas eram
int unregisterAll(const std::wstring &provider);
/// Caminho registrado para o id, vazio se não estiver registrado
std::wstring registeredPath(const std::wstring &id);

/// Conecta o processo à raiz: recusa liberar espaço (desidratar) e recusa baixar sob demanda
Error connect(const std::wstring &path, long long *connectionKey);
void disconnect(long long connectionKey);

enum class State { NotPlaceholder, InSync, NotInSync, Missing };
State state(const std::wstring &path);

/// Sincronizado: converte em espaço reservado completo (se ainda não for) e marca. Pendente: só desmarca um espaço
/// reservado, porque um arquivo comum já aparece como pendente.
Error markInSync(const std::wstring &path, bool inSync);

/// Marca tudo como "sempre disponível neste dispositivo" (o que é verdade: nada fica só na nuvem). Sem isso o Explorer
/// mostra pastas vazias, e as pastas acima delas, com o ícone de nuvem. O que for criado dentro herda a marca.
Error pinAll(const std::wstring &root);

/// O arquivo no disco ainda tem a data de modificação (segundos Unix) e o tamanho da última sincronização
bool unchanged(const std::wstring &path, long long modtime, long long size);

/// Desfaz o espaço reservado (volta a ser um arquivo comum)
Error revert(const std::wstring &path);

}
