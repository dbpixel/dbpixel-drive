# Publica uma versão nova do Drive: compila, gera o instalador e cria a release no GitHub.
# Todos os clientes procuram a versão nova ao abrir e a cada 2 horas, baixam e instalam sozinhos.
#
# Uso:  .\scripts\publicar-versao.ps1 -Notas "Corrige o envio de planilhas abertas no Excel"
#       .\scripts\publicar-versao.ps1 -Notas "..." -SemPublicar   (só gera o instalador e o drive-update.json em dist\)
param(
    [Parameter(Mandatory = $true)][string]$Notas,
    [switch]$SemPublicar,
    [string]$Repo = "dbpixel/dbpixel-drive"
)
$ErrorActionPreference = "Stop"
$raiz = Split-Path -Parent $PSScriptRoot
$craft = "C:\Git\_craft\craft.ps1"

# 1. número da versão: 6.0.3.<BUILD>, sobe um a cada publicação
$buildFile = Join-Path $raiz "branding\BUILD"
$build = [int](Get-Content $buildFile -TotalCount 1) + 1
Set-Content -Path $buildFile -Value $build -Encoding ascii
$versao = "6.0.3.$build"
Write-Host "Versão $versao" -ForegroundColor Cyan

# 2. compila e empacota (a ação vem antes do nome do pacote)
foreach ($passo in @("--compile", "--install", "--qmerge", "--package")) {
    & $craft -c --no-cache $passo owncloud/owncloud-client
    if ($LASTEXITCODE -ne 0) { throw "craft $passo falhou" }
}

# 3. instalador e descrição da versão
$dist = Join-Path $raiz "dist"
New-Item -ItemType Directory -Force $dist | Out-Null
$instalador = Join-Path $dist "Drive-$versao-setup.exe"
Copy-Item "C:\Git\_craft\binaries\owncloud-client-HEAD-master-windows-cl-msvc2022-x86_64.exe" $instalador -Force
$sha = (Get-FileHash $instalador -Algorithm SHA256).Hash.ToLower()
$url = "https://github.com/$Repo/releases/download/v$versao/Drive-$versao-setup.exe"
$manifesto = Join-Path $dist "drive-update.json"
$json = [ordered]@{ version = $versao; url = $url; sha256 = $sha; notes = $Notas; date = (Get-Date -Format "yyyy-MM-dd") } | ConvertTo-Json
[IO.File]::WriteAllText($manifesto, $json, (New-Object Text.UTF8Encoding $false))  # sem BOM
Write-Host "Instalador: $instalador"
Write-Host "SHA-256:    $sha"

if ($SemPublicar) {
    Write-Host "Gerado sem publicar (os clientes só atualizam quando a release existir)." -ForegroundColor Yellow
    return
}

# 4. código-fonte (GPL) e release: o arquivo drive-update.json da última release é o que os clientes leem
Push-Location $raiz
try {
    git add -A
    git commit -m "Drive $versao: $Notas"
    git tag "v$versao"
    git push origin HEAD --tags
    gh release create "v$versao" $instalador $manifesto --repo $Repo --title "Drive $versao" --notes $Notas --latest
    if ($LASTEXITCODE -ne 0) { throw "gh release falhou" }
} finally {
    Pop-Location
}
Write-Host "Publicado. Os clientes instalam a versão $versao sozinhos nas próximas 2 horas (ou ao abrir o Drive)." -ForegroundColor Green
