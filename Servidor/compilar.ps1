Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Set-Location -Path $PSScriptRoot

if (-not (Test-Path -Path 'bin')) {
    New-Item -ItemType Directory -Path 'bin' | Out-Null
}

javac -encoding UTF-8 -d bin *.java

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host 'Servidor compilado. Ejecutar: java -cp bin Servidor.server'