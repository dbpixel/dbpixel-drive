# Drive, um produto DBPixel

Aplicativo de sincronização do Armazenamento em Nuvem da DBPixel (drive.dbpixel.com.br), para Windows.

Baseado no [ownCloud Desktop Client](https://github.com/owncloud/client) v6.0.3 (commit 7c8209b), distribuído sob a
GNU GPL versão 2 (veja `COPYING`). Este repositório é o código-fonte completo das versões publicadas em
[Releases](https://github.com/dbpixel/dbpixel-drive/releases).

## O que muda em relação ao ownCloud

- Marca Drive (`branding/`: nome, ícones, cores do produto, servidor já preenchido).
- Painel da bandeja, assistente de primeiro acesso e configurações redesenhados (`src/gui/qml/`, `src/gui/traypanel.*`).
- Atualização automática própria (`src/gui/drivesupport.*`): o Drive lê `drive-update.json` da última release,
  baixa o instalador, confere o SHA-256 e instala em silêncio quando não há sincronização em andamento.
- Relatório de problema: junta o registro dos últimos dias com o relato do cliente e manda o link ao suporte.

## Compilar

Com o KDE Craft em `C:\Git\_craft` (alvo `windows-cl-msvc2022-x86_64`):

```powershell
C:\Git\_craft\craft.ps1 -c --no-cache --compile owncloud/owncloud-client
C:\Git\_craft\craft.ps1 -c --no-cache --install owncloud/owncloud-client
C:\Git\_craft\craft.ps1 -c --no-cache --qmerge owncloud/owncloud-client
C:\Git\_craft\craft.ps1 -c --package owncloud/owncloud-client
```

## Publicar uma versão

```powershell
.\scripts\publicar-versao.ps1 -Notas "o que mudou"
```

Sobe o número em `branding/BUILD`, compila, gera o instalador e cria a release com `drive-update.json`.
Os clientes instalam sozinhos ao abrir o Drive ou em até 2 horas.
