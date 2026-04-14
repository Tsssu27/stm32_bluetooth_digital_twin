# get_hal_library.ps1
# Download STM32CubeF1 HAL files from their CORRECT sub-module repositories.
#
# STM32CubeF1 uses git submodules, so raw files are NOT in the main repo.
# Actual source repos:
#   CMSIS Core      : ARM-software/CMSIS_5 (already downloaded correctly via Core/Include)
#   CMSIS Device F1 : STMicroelectronics/cmsis_device_f1
#   HAL Driver F1   : STMicroelectronics/stm32f1xx-hal-driver

param(
    [string]$WorkDir = $PSScriptRoot
)

$ErrorActionPreference = "Stop"

$sep = "-" * 52
Write-Host $sep -ForegroundColor Cyan
Write-Host "  STM32F1xx HAL Library Downloader" -ForegroundColor Cyan
Write-Host "  Device: cmsis_device_f1 (master)" -ForegroundColor Cyan
Write-Host "  HAL   : stm32f1xx-hal-driver (master)" -ForegroundColor Cyan
Write-Host $sep -ForegroundColor Cyan

$DriversDir   = Join-Path $WorkDir "Drivers"
$CMSISDevDir  = Join-Path $DriversDir "CMSIS\Device\ST\STM32F1xx\Include"
$HALIncDir    = Join-Path $DriversDir "STM32F1xx_HAL_Driver\Inc"
$HALLegDir    = Join-Path $DriversDir "STM32F1xx_HAL_Driver\Inc\Legacy"
$HALSrcDir    = Join-Path $DriversDir "STM32F1xx_HAL_Driver\Src"

Write-Host "`n[1/3] Creating directories..." -ForegroundColor Yellow
foreach ($d in @($CMSISDevDir, $HALIncDir, $HALLegDir, $HALSrcDir)) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
}

# Correct base URLs (submodule repos, branch = master)
$DevBase = "https://raw.githubusercontent.com/STMicroelectronics/cmsis_device_f1/master"
$HALBase = "https://raw.githubusercontent.com/STMicroelectronics/stm32f1xx-hal-driver/master"

$ok    = 0
$fail  = 0

function Get-File {
    param([string]$Url, [string]$Dest)
    $fn = Split-Path $Url -Leaf
    Write-Host "  GET $fn" -NoNewline
    try {
        Invoke-WebRequest -Uri $Url -OutFile $Dest -UseBasicParsing -TimeoutSec 30
        Write-Host " [OK]" -ForegroundColor Green
        $script:ok++
    } catch {
        Write-Host " [FAIL] $($_.Exception.Message)" -ForegroundColor Red
        $script:fail++
    }
}

# ── CMSIS Device F1 ───────────────────────────────────────────────────────
Write-Host "`n[2/3] CMSIS Device headers (cmsis_device_f1)..." -ForegroundColor Yellow
@(
    "Include/stm32f100xb.h",
    "Include/stm32f100xe.h",
    "Include/stm32f103xb.h",
    "Include/stm32f103xe.h",
    "Include/stm32f1xx.h",
    "Include/system_stm32f1xx.h"
) | ForEach-Object {
    $fn = Split-Path $_ -Leaf
    Get-File "$DevBase/$_" (Join-Path $CMSISDevDir $fn)
}

# ── HAL Driver Inc ────────────────────────────────────────────────────────
Write-Host "`n[3/3] STM32F1xx HAL Driver (stm32f1xx-hal-driver)..." -ForegroundColor Yellow

@(
    "Inc/stm32f1xx_hal.h",
    "Inc/stm32f1xx_hal_def.h",
    "Inc/stm32f1xx_hal_rcc.h",
    "Inc/stm32f1xx_hal_rcc_ex.h",
    "Inc/stm32f1xx_hal_gpio.h",
    "Inc/stm32f1xx_hal_gpio_ex.h",
    "Inc/stm32f1xx_hal_flash.h",
    "Inc/stm32f1xx_hal_flash_ex.h",
    "Inc/stm32f1xx_hal_dma.h",
    "Inc/stm32f1xx_hal_cortex.h",
    "Inc/stm32f1xx_hal_exti.h",
    "Inc/stm32f1xx_hal_tim.h",
    "Inc/stm32f1xx_hal_tim_ex.h",
    "Inc/stm32f1xx_hal_i2c.h",
    "Inc/stm32f1xx_hal_pwr.h"
) | ForEach-Object {
    $fn = Split-Path $_ -Leaf
    Get-File "$HALBase/$_" (Join-Path $HALIncDir $fn)
}

Get-File "$HALBase/Inc/Legacy/stm32_hal_legacy.h" (Join-Path $HALLegDir "stm32_hal_legacy.h")

@(
    "Src/stm32f1xx_hal.c",
    "Src/stm32f1xx_hal_rcc.c",
    "Src/stm32f1xx_hal_rcc_ex.c",
    "Src/stm32f1xx_hal_gpio.c",
    "Src/stm32f1xx_hal_gpio_ex.c",
    "Src/stm32f1xx_hal_flash.c",
    "Src/stm32f1xx_hal_flash_ex.c",
    "Src/stm32f1xx_hal_dma.c",
    "Src/stm32f1xx_hal_cortex.c",
    "Src/stm32f1xx_hal_exti.c",
    "Src/stm32f1xx_hal_tim.c",
    "Src/stm32f1xx_hal_tim_ex.c",
    "Src/stm32f1xx_hal_i2c.c",
    "Src/stm32f1xx_hal_pwr.c"
) | ForEach-Object {
    $fn = Split-Path $_ -Leaf
    Get-File "$HALBase/$_" (Join-Path $HALSrcDir $fn)
}

Write-Host ""
Write-Host $sep -ForegroundColor $(if ($fail -eq 0) { "Green" } else { "Yellow" })
Write-Host "  Completed: $ok OK, $fail FAILED" -ForegroundColor $(if ($fail -eq 0) { "Green" } else { "Yellow" })
if ($fail -eq 0) {
    Write-Host "  All files downloaded. Run 'make' to build!" -ForegroundColor Green
} else {
    Write-Host "  Some files failed. Check network and retry." -ForegroundColor Yellow
}
Write-Host $sep -ForegroundColor $(if ($fail -eq 0) { "Green" } else { "Yellow" })
