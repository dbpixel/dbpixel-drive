/*
 * Drive: chamadas da API de arquivos de nuvem do Windows (cfapi) usadas para mostrar o status no Explorer.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui/cloudfiles_win.h"

#include <windows.h>
#include <winternl.h>
#include <sddl.h>
#include <cfapi.h>

#include <thread>

#ifndef CF_SIZE_OF_OP_PARAM
#define CF_SIZE_OF_OP_PARAM(field) (FIELD_OFFSET(CF_OPERATION_PARAMETERS, field) + RTL_FIELD_SIZE(CF_OPERATION_PARAMETERS, field))
#endif

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Provider.h>

namespace OCC::CloudFiles {

namespace {
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Provider;

    // resposta que recusa o pedido do Windows (STATUS_UNSUCCESSFUL)
    constexpr NTSTATUS recusado = static_cast<NTSTATUS>(0xC0000001L);

    std::wstring message(HRESULT hr)
    {
        wchar_t *buffer = nullptr;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, static_cast<DWORD>(hr), 0,
            reinterpret_cast<wchar_t *>(&buffer), 0, nullptr);
        std::wstring out = buffer ? buffer : L"";
        LocalFree(buffer);
        while (!out.empty() && (out.back() == L'\n' || out.back() == L'\r' || out.back() == L' ')) {
            out.pop_back();
        }
        wchar_t code[16];
        swprintf_s(code, L"0x%08X", static_cast<unsigned>(hr));
        return out.empty() ? code : out + L" (" + code + L")";
    }

    std::wstring longPath(const std::wstring &path)
    {
        if (path.rfind(L"\\\\", 0) == 0) {
            return path;
        }
        return L"\\\\?\\" + path;
    }

    /// Executa no apartamento MTA: as chamadas assíncronas do WinRT não podem esperar na thread da interface
    template <typename F>
    Error inMta(F &&f)
    {
        // Mantém o MTA vivo até o fim do processo. Desligar o COM logo depois de Unregister() descarrega DLLs que o
        // Windows ainda usa em segundo plano e derruba o processo.
        static const bool mtaKeptAlive = [] {
            // uma thread que entra no MTA e nunca sai segura o apartamento aberto
            std::thread([] {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                Sleep(INFINITE);
            }).detach();
            return true;
        }();
        (void)mtaKeptAlive;
        Error error;
        std::thread([&] {
            try {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                f();
            } catch (const winrt::hresult_error &e) {
                error = std::wstring(e.message().c_str()) + L" (" + message(e.code()) + L")";
            } catch (...) {
                error = L"erro desconhecido";
            }
        }).join();
        return error;
    }

    struct Handle
    {
        HANDLE h = INVALID_HANDLE_VALUE;
        ~Handle()
        {
            if (h != INVALID_HANDLE_VALUE) {
                CloseHandle(h);
            }
        }
        explicit operator bool() const { return h != INVALID_HANDLE_VALUE; }
    };

    void open(Handle &handle, const std::wstring &path, DWORD access)
    {
        // não bloqueia quem estiver com o arquivo aberto (Word, Excel...)
        handle.h = CreateFileW(longPath(path).c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    }

    void CALLBACK onFetchData(const CF_CALLBACK_INFO *info, const CF_CALLBACK_PARAMETERS *params)
    {
        // todos os arquivos ficam completos no computador; não há o que baixar sob demanda
        CF_OPERATION_INFO op = {};
        op.StructSize = sizeof(op);
        op.Type = CF_OPERATION_TYPE_TRANSFER_DATA;
        op.ConnectionKey = info->ConnectionKey;
        op.TransferKey = info->TransferKey;
        op.RequestKey = info->RequestKey;
        CF_OPERATION_PARAMETERS p = {};
        p.ParamSize = CF_SIZE_OF_OP_PARAM(TransferData);
        p.TransferData.CompletionStatus = recusado;
        p.TransferData.Offset = params->FetchData.RequiredFileOffset;
        p.TransferData.Length = params->FetchData.RequiredLength;
        CfExecute(&op, &p);
    }

    void CALLBACK onNotifyDehydrate(const CF_CALLBACK_INFO *info, const CF_CALLBACK_PARAMETERS *)
    {
        // "Liberar espaço" não é suportado: o arquivo continua no computador
        CF_OPERATION_INFO op = {};
        op.StructSize = sizeof(op);
        op.Type = CF_OPERATION_TYPE_ACK_DEHYDRATE;
        op.ConnectionKey = info->ConnectionKey;
        op.TransferKey = info->TransferKey;
        op.RequestKey = info->RequestKey;
        CF_OPERATION_PARAMETERS p = {};
        p.ParamSize = CF_SIZE_OF_OP_PARAM(AckDehydrate);
        p.AckDehydrate.Flags = CF_OPERATION_ACK_DEHYDRATE_FLAG_NONE;
        p.AckDehydrate.CompletionStatus = recusado;
        p.AckDehydrate.FileIdentity = info->FileIdentity;
        p.AckDehydrate.FileIdentityLength = info->FileIdentityLength;
        CfExecute(&op, &p);
    }
}

std::wstring syncRootId(const std::wstring &provider, const std::wstring &account)
{
    std::wstring sid;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        DWORD size = 0;
        GetTokenInformation(token, TokenUser, nullptr, 0, &size);
        std::string buffer(size, '\0');
        if (GetTokenInformation(token, TokenUser, buffer.data(), size, &size)) {
            wchar_t *str = nullptr;
            if (ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER *>(buffer.data())->User.Sid, &str)) {
                sid = str;
                LocalFree(str);
            }
        }
        CloseHandle(token);
    }
    return provider + L"!" + sid + L"!" + account;
}

Error registerSyncRoot(const SyncRootInfo &root)
{
    return inMta([&] {
        StorageProviderSyncRootInfo info;
        info.Id(root.id);
        info.Path(StorageFolder::GetFolderFromPathAsync(root.path).get());
        info.DisplayNameResource(root.displayName);
        info.IconResource(root.iconResource);
        info.Version(root.version);
        // só arquivos completos: nada é baixado sob demanda nem liberado automaticamente
        info.HydrationPolicy(StorageProviderHydrationPolicy::Full);
        info.HydrationPolicyModifier(StorageProviderHydrationPolicyModifier::None);
        info.PopulationPolicy(StorageProviderPopulationPolicy::AlwaysFull);
        info.InSyncPolicy(StorageProviderInSyncPolicy::FileCreationTime | StorageProviderInSyncPolicy::DirectoryCreationTime
            | StorageProviderInSyncPolicy::FileLastWriteTime | StorageProviderInSyncPolicy::DirectoryLastWriteTime);
        info.HardlinkPolicy(StorageProviderHardlinkPolicy::None);
        info.ShowSiblingsAsGroup(false);
        info.AllowPinning(false);
        info.Context(winrt::Windows::Security::Cryptography::CryptographicBuffer::ConvertStringToBinary(
            root.id, winrt::Windows::Security::Cryptography::BinaryStringEncoding::Utf8));
        StorageProviderSyncRootManager::Register(info);
    });
}

Error unregisterSyncRoot(const std::wstring &id)
{
    return inMta([&] { StorageProviderSyncRootManager::Unregister(id); });
}

int unregisterAll(const std::wstring &provider)
{
    int count = 0;
    inMta([&] {
        const std::wstring prefix = provider + L"!";
        for (const auto &root : StorageProviderSyncRootManager::GetCurrentSyncRoots()) {
            const std::wstring id = root.Id().c_str();
            if (id.rfind(prefix, 0) == 0) {
                StorageProviderSyncRootManager::Unregister(id);
                ++count;
            }
        }
    });
    return count;
}

std::wstring registeredPath(const std::wstring &id)
{
    std::wstring path;
    inMta([&] {
        try {
            path = StorageProviderSyncRootManager::GetSyncRootInformationForId(id).Path().Path().c_str();
        } catch (...) {
        }
    });
    return path;
}

Error connect(const std::wstring &path, long long *connectionKey)
{
    static const CF_CALLBACK_REGISTRATION callbacks[] = {
        {CF_CALLBACK_TYPE_FETCH_DATA, onFetchData},
        {CF_CALLBACK_TYPE_NOTIFY_DEHYDRATE, onNotifyDehydrate},
        CF_CALLBACK_REGISTRATION_END,
    };
    CF_CONNECTION_KEY key = {};
    const HRESULT hr = CfConnectSyncRoot(path.c_str(), callbacks, nullptr, CF_CONNECT_FLAG_NONE, &key);
    if (FAILED(hr)) {
        return message(hr);
    }
    *connectionKey = key.Internal;
    return {};
}

void disconnect(long long connectionKey)
{
    CF_CONNECTION_KEY key = {connectionKey};
    CfDisconnectSyncRoot(key);
}

State state(const std::wstring &path)
{
    Handle handle;
    open(handle, path, FILE_READ_ATTRIBUTES);
    if (!handle) {
        return State::Missing;
    }
    // CF_PLACEHOLDER_BASIC_INFO seguido da identidade (pequena)
    alignas(8) unsigned char buffer[sizeof(CF_PLACEHOLDER_BASIC_INFO) + 256];
    DWORD returned = 0;
    const HRESULT hr = CfGetPlaceholderInfo(handle.h, CF_PLACEHOLDER_INFO_BASIC, buffer, sizeof(buffer), &returned);
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        return State::NotPlaceholder;
    }
    const auto *info = reinterpret_cast<const CF_PLACEHOLDER_BASIC_INFO *>(buffer);
    return info->InSyncState == CF_IN_SYNC_STATE_IN_SYNC ? State::InSync : State::NotInSync;
}

Error markInSync(const std::wstring &path, bool inSync)
{
    const auto current = state(path);
    if (current == State::Missing) {
        return {}; // já não existe (apagado, ou retido pelo antivírus): não há o que marcar
    }
    // um arquivo comum já aparece como pendente; só vira espaço reservado quando estiver sincronizado
    if ((current == State::InSync && inSync) || (current != State::InSync && !inSync)) {
        return {};
    }
    Handle handle;
    open(handle, path, FILE_WRITE_ATTRIBUTES);
    if (!handle) {
        return message(HRESULT_FROM_WIN32(GetLastError()));
    }
    HRESULT hr;
    if (current == State::NotPlaceholder) {
        static const wchar_t identity[] = L"D";
        hr = CfConvertToPlaceholder(handle.h, identity, sizeof(identity), inSync ? CF_CONVERT_FLAG_MARK_IN_SYNC : CF_CONVERT_FLAG_NONE, nullptr, nullptr);
    } else {
        hr = CfSetInSyncState(handle.h, inSync ? CF_IN_SYNC_STATE_IN_SYNC : CF_IN_SYNC_STATE_NOT_IN_SYNC, CF_SET_IN_SYNC_FLAG_NONE, nullptr);
    }
    return FAILED(hr) ? message(hr) : Error();
}

Error pinAll(const std::wstring &root)
{
    Handle handle;
    open(handle, root, FILE_WRITE_ATTRIBUTES);
    if (!handle) {
        return message(HRESULT_FROM_WIN32(GetLastError()));
    }
    const HRESULT hr = CfSetPinState(handle.h, CF_PIN_STATE_PINNED, CF_SET_PIN_FLAG_RECURSE, nullptr);
    return FAILED(hr) ? message(hr) : Error();
}

bool unchanged(const std::wstring &path, long long modtime, long long size)
{
    WIN32_FILE_ATTRIBUTE_DATA data = {};
    if (!GetFileAttributesExW(longPath(path).c_str(), GetFileExInfoStandard, &data)) {
        return false;
    }
    const auto ticks = (static_cast<long long>(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime;
    const auto unix = (ticks - 116444736000000000LL) / 10000000LL;
    const auto bytes = (static_cast<long long>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    return unix == modtime && bytes == size;
}

Error revert(const std::wstring &path)
{
    if (state(path) == State::NotPlaceholder) {
        return {};
    }
    Handle handle;
    open(handle, path, FILE_WRITE_ATTRIBUTES);
    if (!handle) {
        return message(HRESULT_FROM_WIN32(GetLastError()));
    }
    const HRESULT hr = CfRevertPlaceholder(handle.h, CF_REVERT_FLAG_NONE, nullptr);
    return FAILED(hr) ? message(hr) : Error();
}

}
