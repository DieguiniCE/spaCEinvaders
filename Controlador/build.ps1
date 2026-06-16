param()

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $repoRoot 'build'
$sdkPath = $env:PICO_SDK_PATH

if ([string]::IsNullOrWhiteSpace($sdkPath)) {
    $sdkPath = 'C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/pico-sdk'
}

$cmake = (Get-Command cmake.exe -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source)
if (-not $cmake) {
    $cmakeCandidates = @(
        'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe',
        'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/bin/cmake.exe'
    )

    foreach ($candidate in $cmakeCandidates) {
        if (Test-Path $candidate) {
            $cmake = $candidate
            break
        }
    }
}

if (-not $cmake) {
    throw 'Unable to find cmake.exe. Install CMake or add it to PATH.'
}

$ninja = (Get-Command ninja.exe -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source)
if (-not $ninja) {
    $ninjaCandidates = @(
        'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe',
        'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/bin/ninja.exe'
    )

    foreach ($candidate in $ninjaCandidates) {
        if (Test-Path $candidate) {
            $ninja = $candidate
            break
        }
    }
}

if (-not $ninja) {
    throw 'Unable to find ninja.exe. Install Ninja or use the Visual Studio bundled copy.'
}

& $cmake -G Ninja `
    -S $repoRoot `
    -B $buildDir `
    -DPICO_SDK_PATH="$sdkPath" `
    -DCMAKE_MAKE_PROGRAM="$ninja"

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $cmake --build $buildDir --parallel
exit $LASTEXITCODE