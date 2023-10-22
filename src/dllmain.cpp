#include "stdafx.h"
#include "helper.hpp"

using namespace std;

HMODULE baseModule = GetModuleHandle(NULL);

inipp::Ini<char> ini;

// INI Variables
bool bAspectFix;
bool bHUDFix;
bool bResUnlock;
bool bMovieFix;
int iCustomResX;
int iCustomResY;
int iInjectionDelay;
int iAspectFix;
int iHUDFix;
int iMovieFix;

// Variables
float fNewX;
float fNewY;
float fNativeAspect = (float)16/9;
float fPi = 3.14159265358979323846f;
float fNewAspect;
float fAspectDivisional;
float fAspectMultiplier;
float fDMC3_DefaultCamAspect = 0.5833333135f;
float fDMC3_DefaultTextWidth = 0.0029296875f;
float fDMC3_DefaultHUDWidth = (float)1;
float fHUDWidth;
float fHUDOffset;
short iResX;
short iResY;
string sExeName;
string sGameName;
string sExePath;
string sGameVersion;
string sFixVer = "0.7";

// DMC 1: Aspect Ratio Hook
DWORD64 DMC1_AspectRatioReturnJMP;
void __declspec(naked) DMC1_AspectRatio_CC()
{
    __asm
    {
        movaps xmm0, xmm2
        mulss xmm0, [fAspectDivisional]
        movaps[rdx + 0x10], xmm1
        jmp[DMC1_AspectRatioReturnJMP]
    }
}

// DMC 2: Aspect Ratio Hook
DWORD64 DMC2_AspectRatioReturnJMP;
float fDMC2_AspectWidth;
void __declspec(naked) DMC2_AspectRatio_CC()
{
    __asm
    {
        cvtdq2ps xmm1, xmm1
        cvtdq2ps xmm2, xmm2
        movss xmm2, [fDMC2_AspectWidth]
        mulss xmm0, xmm1
        divss xmm0, xmm2
        jmp[DMC2_AspectRatioReturnJMP]
    }
}

// DMC 3: Aspect Ratio Hook
DWORD64 DMC3_AspectRatioReturnJMP;
void __declspec(naked) DMC3_AspectRatio_CC()
{
    __asm
    {
        movss xmm9, [fDMC3_DefaultCamAspect]
        mulss xmm9, [fAspectMultiplier]
        movss [rax + 0x64], xmm9
        movaps [rcx + 0x00000080], xmm11
        jmp[DMC3_AspectRatioReturnJMP]
    }
}

// DMC 2: HUD Width
DWORD64 DMC2_HUDWidthReturnJMP;
int iDMC2_NewHUDWidth;
void __declspec(naked) DMC2_HUDWidth_CC()
{
    __asm
    {
        movzx eax, word ptr[iDMC2_NewHUDWidth]
        mov [rdx], eax
        movzx eax, word ptr[rcx + 0x00000092]
        jmp[DMC2_HUDWidthReturnJMP]
    }
}

// DMC 3: Text Width
DWORD64 DMC3_TextWidthReturnJMP;
float fDMC3_NewTextWidth;
void __declspec(naked) DMC3_TextWidth_CC()
{
    __asm
    {
        movss xmm6, [fDMC3_NewTextWidth]
        mulss xmm0, xmm4
        cvtdq2ps xmm2, xmm2
        addss xmm0, xmm5
        mulss xmm2, xmm4
        jmp[DMC3_TextWidthReturnJMP]
    }
}

// DMC 3: Combo Meter
DWORD64 DMC3_ComboMeterReturnJMP;
float fDMC3_ComboMeterValue;
void __declspec(naked) DMC3_ComboMeter_CC()
{
    __asm
    {
        mov r9d, 0x00FFFFF1
        divss xmm3, [fAspectMultiplier]
        subss xmm0, xmm4
        cvttss2si rax, xmm3
        cvttss2si edx, xmm0
        jmp[DMC3_ComboMeterReturnJMP]
    }
}


// DMC 1: Movie Hook
DWORD64 DMC1_MovieReturnJMP;
void __declspec(naked) DMC1_Movie_CC()
{
    __asm
    {
        cvtsi2ss xmm0, rax
        movss xmm1, [fHUDOffset]
        movss [rsp + 0x28], xmm1
        movss xmm1, [fHUDWidth]
        movss [rsp + 0x30], xmm1
        movss [rsp + 0x34], xmm0
        jmp[DMC1_MovieReturnJMP]
    }
}

// DMC 2: Movie Hook
DWORD64 DMC2_MovieReturnJMP;
void __declspec(naked) DMC2_Movie_CC()
{
    __asm
    {
        cvtsi2ss xmm0, rax
        movss xmm1, [fHUDOffset]
        movss [rbp - 0x11], xmm1
        movss xmm1, [fHUDWidth]
        movss [rbp - 0x09], xmm1
        movss [rbp - 0x05], xmm0
        jmp[DMC2_MovieReturnJMP]
    }
}

// DMC 3: Movie Hook
DWORD64 DMC3_MovieReturnJMP;
void __declspec(naked) DMC3_Movie_CC()
{
    __asm
    {
        cvtsi2ss xmm0, rax
        movss xmm1, [fHUDOffset]
        movss[rsp + 0x30], xmm1
        movss xmm1, [fHUDWidth]
        movss[rsp + 0x38], xmm1
        movss[rsp + 0x3C], xmm0
        jmp[DMC3_MovieReturnJMP]
    }
}


void Logging()
{
    loguru::add_file("DMCHDFix.log", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::set_thread_name("Main");

    LOG_F(INFO, "DMCHDFix v%s loaded", sFixVer.c_str());
}

void ReadConfig()
{
    // Initialise config
    std::ifstream iniFile(".\\DMCHDFix.ini");
    if (!iniFile)
    {
        LOG_F(ERROR, "Failed to load config file.");
    }
    else
    {
        ini.parse(iniFile);
    }

    inipp::get_value(ini.sections["DMCHDFix Parameters"], "InjectionDelay", iInjectionDelay);
    inipp::get_value(ini.sections["Resolution Unlock"], "Enabled", bResUnlock);
    inipp::get_value(ini.sections["Fix Aspect Ratio"], "Enabled", bAspectFix);
    iAspectFix = (int)bAspectFix;
    inipp::get_value(ini.sections["Fix HUD"], "Enabled", bHUDFix);
    iHUDFix = (int)bHUDFix;
    inipp::get_value(ini.sections["Fix FMVs"], "Enabled", bMovieFix);
    iMovieFix = (int)bMovieFix;

    // Custom resolution
    if (iCustomResX > 0 && iCustomResY > 0)
    {
        fNewX = (float)iCustomResX;
        fNewY = (float)iCustomResY;
        fNewAspect = (float)iCustomResX / (float)iCustomResY;
    }
    else
    {
        // Grab desktop resolution
        RECT desktop;
        GetWindowRect(GetDesktopWindow(), &desktop);
        fNewX = (float)desktop.right;
        fNewY = (float)desktop.bottom;
        iCustomResX = (int)desktop.right;
        iCustomResY = (int)desktop.bottom;
        fNewAspect = (float)desktop.right / (float)desktop.bottom;
    }

    // Log config parse
    LOG_F(INFO, "Config Parse: iInjectionDelay: %dms", iInjectionDelay);
    LOG_F(INFO, "Config Parse: bResUnlock: %d", bResUnlock);
    LOG_F(INFO, "Config Parse: bAspectFix: %d", bAspectFix);
    LOG_F(INFO, "Config Parse: bHUDFix: %d", bHUDFix);
    LOG_F(INFO, "Config Parse: bMovieFix: %d", bMovieFix);
}

void DetectGame()
{
    // Get game name and exe path
    LPWSTR exePath = new WCHAR[_MAX_PATH];
    GetModuleFileName(baseModule, exePath, MAX_PATH);
    wstring exePathWString(exePath);
    sExePath = string(exePathWString.begin(), exePathWString.end());
    sExeName = sExePath.substr(sExePath.find_last_of("/\\") + 1);

    string sdmc1 = string("dmcLauncher.exe");
    LOG_F(INFO, "Game Name: %s", sExeName.c_str());
    LOG_F(INFO, "Game Path: %s", sExePath.c_str());

    if (sExeName == "dmcLauncher.exe")
    {
        LOG_F(INFO, "Detected game is: DMC HD Launcher");
    }
    else if (sExeName == "dmc1.exe")
    {
        LOG_F(INFO, "Detected game is: Devil May Cry 1");
    }
    else if (sExeName == "dmc2.exe")
    {
        LOG_F(INFO, "Detected game is: Devil May Cry 2");
    }
    else if (sExeName == "dmc3.exe")
    {
        LOG_F(INFO, "Detected game is: Devil May Cry 3");
    }
}

void ResolutionUnlock()
{
    if (sExeName == "dmcLauncher.exe" && bResUnlock)
    {
        // Launcher: Disable 1080p limit on results from EnumDisplaySettings
        uint8_t* Launcher_ResLimitScanResult = Memory::PatternScan(baseModule, "?? ?? 80 07 00 00 ?? ?? ?? ?? ?? ?? ?? ?? ?? 38 04 00 00");
        if (Launcher_ResLimitScanResult)
        {
            DWORD64 Launcher_ResLimitAddress = (uintptr_t)Launcher_ResLimitScanResult + 0x2;
            LOG_F(INFO, "Launcher: Resolution Unlock: Address is 0x%" PRIxPTR, (uintptr_t)Launcher_ResLimitAddress);

            Memory::Write(Launcher_ResLimitAddress, INT_MAX);             // 1920
            Memory::Write(Launcher_ResLimitAddress + 0xD, INT_MAX);       // 1080
            LOG_F(INFO, "Launcher: Resolution Unlock: Wrote new resolution limit.");
        }
        else if (!Launcher_ResLimitScanResult)
        {
            LOG_F(INFO, "Launcher: Resolution Unlock: Pattern scan failed.");
        }
    }
}

void ResolutionCheck()
{
    if (sExeName == "dmcLauncher.exe")
    {
        // Launcher: Grab launcher current resolution
        // TODO: Make this a hook so it can update? 
        uint8_t* Launcher_CurrResolutionScanResult = Memory::PatternScan(baseModule, "48 ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 0F ?? ?? 33 ?? FF");
        if (Launcher_CurrResolutionScanResult)
        {
            DWORD64 Launcher_CurrResolutionAddress = (uintptr_t)Launcher_CurrResolutionScanResult;
            LOG_F(INFO, "Launcher: Current Resolution: Address is 0x%" PRIxPTR, (uintptr_t)Launcher_CurrResolutionAddress);

            DWORD64 Launcher_CurrResolutionValues = Memory::GetAbsolute(Launcher_CurrResolutionAddress + 0x7);
            LOG_F(INFO, "Launcher: Current Resolution: Values address is 0x%" PRIxPTR, (uintptr_t)Launcher_CurrResolutionValues);
            iResX = *reinterpret_cast<short*>(Launcher_CurrResolutionValues);
            iResY = *reinterpret_cast<short*>(Launcher_CurrResolutionValues + 0x4);
            LOG_F(INFO, "Launcher: Current Resolution: Resolution is %dx%d", iResX, iResY);

            // Calculate Launcher values
            fNewAspect = (float)iResX / iResY;
            LOG_F(INFO, "Launcher: Current Resolution: Aspect ratio = %.4f", fNewAspect);
            fAspectDivisional = (float)iResY / iResX;
            LOG_F(INFO, "Launcher: Current Resolution: Aspect ratio divisional = %.4f", fAspectDivisional);
            fAspectMultiplier = (((float)iResX / iResY) / fNativeAspect);
            LOG_F(INFO, "Launcher: Current Resolution: Aspect multiplier = %.4f.", fAspectMultiplier);
            fHUDWidth = (float)iResY * fNativeAspect;
            LOG_F(INFO, "Launcher: Current Resolution: fHUDWidth = %.4f.", fHUDWidth);
            fHUDOffset = (float)(iResX - fHUDWidth) / 2;
            LOG_F(INFO, "Launcher: Current Resolution: fHUDOffset = %4f.", fHUDOffset);

        }
        else if (!Launcher_CurrResolutionScanResult)
        {
            LOG_F(INFO, "Launcher: Current Resolution: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc1.exe")
    {
        // DMC 1: Grab current res. Don't need to update later since the game doesn't support resolution changing outside launcher.
        uint8_t* DMC1_CurrResolutionScanResult = Memory::PatternScan(baseModule, "FF ?? ?? ?? ?? ?? 0F B7 ?? ?? ?? ?? ?? 4C ?? ?? ?? 48 8B ?? ?? ?? ?? ??");
        if (DMC1_CurrResolutionScanResult)
        {
            DWORD64 DMC1_CurrResolutionAddress = (uintptr_t)DMC1_CurrResolutionScanResult;
            LOG_F(INFO, "DMC 1: Current Resolution: Address is 0x%" PRIxPTR, (uintptr_t)DMC1_CurrResolutionAddress);

            DWORD64 DMC1_CurrResolutionValues = Memory::GetAbsolute(DMC1_CurrResolutionAddress + 0x9);
            iResX = *reinterpret_cast<short*>(DMC1_CurrResolutionValues);
            iResY = *reinterpret_cast<short*>(DMC1_CurrResolutionValues + 0x2);
            if (iResX == 0 || iResY == 0)
            {
                LOG_F(INFO, "DMC 1: Current Resolution: Failed to read resolution. Try increasing the injection delay.");
                // Grab desktop resolution
                RECT desktop;
                GetWindowRect(GetDesktopWindow(), &desktop);
                iResX = desktop.right;
                iResY = desktop.bottom;
                LOG_F(INFO, "DMC 1: Current Resolution: Using desktop resolution instead.");
            }
            LOG_F(INFO, "DMC 1: Current Resolution: Resolution is %dx%d", iResX, iResY);

            // Calculate DMC1 values
            fNewAspect = (float)iResX / iResY;
            LOG_F(INFO, "DMC 1: Current Resolution: New aspect ratio = %.4f", fNewAspect);
            fAspectDivisional = (float)iResY / iResX;
            LOG_F(INFO, "DMC 1: Current Resolution: Aspect ratio divisional = %.4f", fAspectDivisional);
            fAspectMultiplier = (((float)iResX / iResY) / fNativeAspect);
            LOG_F(INFO, "DMC 1: Current Resolution: Aspect multiplier = %.4f.", fAspectMultiplier);
            fHUDWidth = (float)iResY * fNativeAspect;
            LOG_F(INFO, "DMC 1: Current Resolution: fHUDWidth = %.4f.", fHUDWidth);
            fHUDOffset = (float)(iResX - fHUDWidth) / 2;
            LOG_F(INFO, "DMC 1: Current Resolution: fHUDOffset = %4f.", fHUDOffset);
        }
        else if (!DMC1_CurrResolutionScanResult)
        {
            LOG_F(INFO, "DMC 1: Current Resolution: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc2.exe")
    {
        // DMC 2: Grab current res. Don't need to update later since the game doesn't support resolution changing outside launcher.
        uint8_t* DMC2_CurrResolutionScanResult = Memory::PatternScan(baseModule, "48 ?? ?? 28 48 8D ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D ?? ?? ?? ?? ?? E8 ?? ?? ?? ??");
        if (DMC2_CurrResolutionScanResult)
        {
            DWORD64 DMC2_CurrResolutionAddress = (uintptr_t)DMC2_CurrResolutionScanResult;
            LOG_F(INFO, "DMC 2: Current Resolution: Address is 0x%" PRIxPTR, (uintptr_t)DMC2_CurrResolutionAddress);

            DWORD64 DMC2_CurrResolutionValues = Memory::GetAbsolute(DMC2_CurrResolutionAddress + 0x7);
            iResX = *reinterpret_cast<short*>(DMC2_CurrResolutionValues + 0x90);
            iResY = *reinterpret_cast<short*>(DMC2_CurrResolutionValues + 0x92);
            if (iResX == 0 || iResY == 0)
            {
                LOG_F(INFO, "DMC 2: Current Resolution: Failed to read resolution. Try increasing the injection delay.");
                // Grab desktop resolution
                RECT desktop;
                GetWindowRect(GetDesktopWindow(), &desktop);
                iResX = desktop.right;
                iResY = desktop.bottom;
                LOG_F(INFO, "DMC 2: Current Resolution: Using desktop resolution instead.");
            }
            LOG_F(INFO, "DMC 2: Current Resolution: Resolution is %dx%d", iResX, iResY);

            // Calculate DMC2 values
            fNewAspect = (float)iResX / iResY;
            LOG_F(INFO, "DMC 2: Current Resolution: Aspect ratio = %.4f", fNewAspect);
            fAspectDivisional = (float)iResY / iResX;
            LOG_F(INFO, "DMC 2: Current Resolution: Aspect ratio divisional = %.4f", fAspectDivisional);
            fAspectMultiplier = (((float)iResX / iResY) / fNativeAspect);
            LOG_F(INFO, "DMC 2: Current Resolution: Aspect multiplier = %.4f.", fAspectMultiplier);
            fHUDWidth = (float)iResY * fNativeAspect;
            LOG_F(INFO, "DMC 2: Current Resolution: fHUDWidth = %.4f.", fHUDWidth);
            fHUDOffset = (float)(iResX - fHUDWidth) / 2;
            LOG_F(INFO, "DMC 2: Current Resolution: fHUDOffset = %4f.", fHUDOffset);
        }
        else if (!DMC2_CurrResolutionScanResult)
        {
            LOG_F(INFO, "DMC 2: Current Resolution: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc3.exe")
    {
        // DMC 3: Grab current res. Don't need to update later since the game doesn't support resolution changing outside launcher.
        uint8_t* DMC3_CurrResolutionScanResult = Memory::PatternScan(baseModule, "C7 ?? ?? ?? ?? ?? 80 07 00 00 C7 ?? ?? ?? ?? ?? 38 04 00 00");
        if (DMC3_CurrResolutionScanResult)
        {
            DWORD64 DMC3_CurrResolutionAddress = (uintptr_t)DMC3_CurrResolutionScanResult;
            LOG_F(INFO, "DMC 3: Current Resolution: Address is 0x%" PRIxPTR, (uintptr_t)DMC3_CurrResolutionAddress);

            DWORD64 DMC3_CurrResolutionValues = Memory::GetAbsolute(DMC3_CurrResolutionAddress + 0x2);
            iResX = *reinterpret_cast<short*>(DMC3_CurrResolutionValues + 0x4);
            iResY = *reinterpret_cast<short*>(DMC3_CurrResolutionValues + 0x8);
            if (iResX == 0 || iResY == 0)
            {
                LOG_F(INFO, "DMC 3: Current Resolution: Failed to read resolution. Try increasing the injection delay.");
                // Grab desktop resolution
                RECT desktop;
                GetWindowRect(GetDesktopWindow(), &desktop);
                iResX = desktop.right;
                iResY = desktop.bottom;
                LOG_F(INFO, "DMC 3: Current Resolution: Using desktop resolution instead.");
            }
            LOG_F(INFO, "DMC 3: Current Resolution: Resolution is %dx%d", iResX, iResY);

            // Calculate DMC3 values
            fNewAspect = (float)iResX / iResY;
            LOG_F(INFO, "DMC 3: Current Resolution: Aspect ratio = %.4f", fNewAspect);
            fAspectDivisional = (float)iResY / iResX;
            LOG_F(INFO, "DMC 3: Current Resolution: Aspect ratio divisional = %.4f", fAspectDivisional);
            fAspectMultiplier = (((float)iResX / iResY) / fNativeAspect);
            LOG_F(INFO, "DMC 3: Current Resolution: Aspect multiplier = %.4f.", fAspectMultiplier);
            fHUDWidth = (float)iResY * fNativeAspect;
            LOG_F(INFO, "DMC 3: Current Resolution: fHUDWidth = %.4f.", fHUDWidth);
            fHUDOffset = (float)(iResX - fHUDWidth) / 2;
            LOG_F(INFO, "DMC 3: Current Resolution: fHUDOffset = %4f.", fHUDOffset);
        }
        else if (!DMC3_CurrResolutionScanResult)
        {
            LOG_F(INFO, "DMC 3: Current Resolution: Pattern scan failed.");
        }
    }
}

void AspectFOVFix()
{
    if (sExeName == "dmc1.exe" && bAspectFix)
    {
        // DMC 1: Fix aspect ratio
        uint8_t* DMC1_AspectRatioScanResult = Memory::PatternScan(baseModule, "0F 28 ?? F3 0F 59 ?? ?? ?? ?? ?? 0F 29 ?? ?? 0f 28 ?? ?? ?? ?? ??");
        if (DMC1_AspectRatioScanResult)
        {
            DWORD64 DMC1_AspectRatioAddress = (uintptr_t)DMC1_AspectRatioScanResult;
            int DMC1_AspectRatioHookLength = Memory::GetHookLength((char*)DMC1_AspectRatioAddress, 13);
            DMC1_AspectRatioReturnJMP = DMC1_AspectRatioAddress + DMC1_AspectRatioHookLength;
            Memory::DetourFunction64((void*)DMC1_AspectRatioAddress, DMC1_AspectRatio_CC, DMC1_AspectRatioHookLength);

            LOG_F(INFO, "DMC 1: Aspect Ratio: Hook length is %d bytes", DMC1_AspectRatioHookLength);
            LOG_F(INFO, "DMC 1: Aspect Ratio: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC1_AspectRatioAddress);
        }
        else if (!DMC1_AspectRatioScanResult)
        {
            LOG_F(INFO, "DMC 1: Aspect Ratio: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc2.exe" && bAspectFix)
    {
        // DMC 2: Fix aspect ratio. Only necessary when used with hud width hook but we'll enable it anyway.
        uint8_t* DMC2_AspectRatioScanResult = Memory::PatternScan(baseModule, "48 ?? ?? ?? ?? ?? ?? 0F ?? ?? 0F ?? ?? F3 0F ?? ?? F3 0F ?? ?? F3 0F ?? ?? ??");
        if (DMC2_AspectRatioScanResult)
        {
            fDMC2_AspectWidth = (float)iResX;
            DWORD64 DMC2_AspectRatioAddress = (uintptr_t)DMC2_AspectRatioScanResult + 0x7;
            int DMC2_AspectRatioHookLength = 14;
            DMC2_AspectRatioReturnJMP = DMC2_AspectRatioAddress + DMC2_AspectRatioHookLength;
            Memory::DetourFunction64((void*)DMC2_AspectRatioAddress, DMC2_AspectRatio_CC, DMC2_AspectRatioHookLength);

            LOG_F(INFO, "DMC 2: Aspect Ratio: Hook length is %d bytes", DMC2_AspectRatioHookLength);
            LOG_F(INFO, "DMC 2: Aspect Ratio: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC2_AspectRatioAddress);
        }
        else if (!DMC2_AspectRatioScanResult)
        {
            LOG_F(INFO, "DMC 2: Aspect Ratio: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc3.exe" && bAspectFix)
    {
        // DMC 3: Fix aspect ratio.
        uint8_t* DMC3_AspectRatioScanResult = Memory::PatternScan(baseModule, "F3 44 ?? ?? ?? ?? 44 0F ?? ?? ?? ?? 00 00 44 0F ?? ?? ?? ?? ?? ??");
        if (DMC3_AspectRatioScanResult)
        {
            DWORD64 DMC3_AspectRatioAddress = (uintptr_t)DMC3_AspectRatioScanResult;
            int DMC3_AspectRatioHookLength = Memory::GetHookLength((char*)DMC3_AspectRatioAddress, 13);
            DMC3_AspectRatioReturnJMP = DMC3_AspectRatioAddress + DMC3_AspectRatioHookLength;
            Memory::DetourFunction64((void*)DMC3_AspectRatioAddress, DMC3_AspectRatio_CC, DMC3_AspectRatioHookLength);

            LOG_F(INFO, "DMC 3: Aspect Ratio: Hook length is %d bytes", DMC3_AspectRatioHookLength);
            LOG_F(INFO, "DMC 3: Aspect Ratio: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC3_AspectRatioAddress);
        }
        else if (!DMC3_AspectRatioScanResult)
        {
            LOG_F(INFO, "DMC 3: Aspect Ratio: Pattern scan failed.");
        }
    }
}

void HUDFix()
{
    if (sExeName == "dmcLauncher.exe" && bHUDFix)
    {
        // Launcher: Fix HUD aspect ratio in launcher
        uint8_t* Launcher_HUDScanResult = Memory::PatternScan(baseModule, "45 ?? ?? ?? F3 44 ?? ?? ?? ?? ?? ?? ?? F3 44 ?? ?? ?? ?? ?? ?? ?? F3 44 ?? ?? ?? ?? ?? ?? ??");
        if (Launcher_HUDScanResult)
        {
            DWORD64 Launcher_HUDAddress = Memory::GetAbsolute((uintptr_t)Launcher_HUDScanResult + 0x9);
            LOG_F(INFO, "Launcher: HUD Fix: Address is 0x%" PRIxPTR, (uintptr_t)Launcher_HUDAddress);

            fNewAspect = (float)iResX / iResY;
            if (fNewAspect > fNativeAspect)
            {
                Memory::Write(Launcher_HUDAddress, (float)(1080 * fNewAspect));
                LOG_F(INFO, "Launcher: HUD Fix: Wrote new width of 1080 * %.4f.", fNewAspect);
            }
            else if (fNewAspect < fNativeAspect)
            {
                Memory::Write(Launcher_HUDAddress, (float)(1920 / fNewAspect));
                LOG_F(INFO, "Launcher: HUD Fix: Wrote new width of 1080 / %.4f.", fNewAspect);
            }
        }
        else if (!Launcher_HUDScanResult)
        {
            LOG_F(INFO, "Launcher: HUD Fix: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc1.exe" && bHUDFix)
    {
        // DMC 1: HUD width
        uint8_t* DMC1_HUDWidthScanResult = Memory::PatternScan(baseModule, "0F 29 ?? ?? ?? F3 0F ?? ?? ?? ?? ?? ?? F3 0F ?? ?? ?? ?? F3 0F ?? ?? ?? ?? ?? ?? 44 0F ?? ?? ?? ??");
        if (DMC1_HUDWidthScanResult)
        {
            DWORD64 DMC1_HUDWidthAddress = (uintptr_t)DMC1_HUDWidthScanResult + 0x9;
            LOG_F(INFO, "DMC 1: HUD Width: Address is 0x%" PRIxPTR, (uintptr_t)DMC1_HUDWidthAddress);
            DWORD64 DMC1_HUDWidthValue = Memory::GetAbsolute(DMC1_HUDWidthAddress);
            LOG_F(INFO, "DMC 1: HUD Width: Value address is 0x%" PRIxPTR, (uintptr_t)DMC1_HUDWidthValue);

            float fDMC1_DefaultHUDWidth = *reinterpret_cast<float*>(DMC1_HUDWidthValue);
            LOG_F(INFO, "DMC 1: HUD Width: Default HUD width =  %.8f.", fDMC1_DefaultHUDWidth);
            float fDMC1_NewHUDWidth = fDMC1_DefaultHUDWidth / fAspectMultiplier;
            Memory::Write(DMC1_HUDWidthValue, fDMC1_NewHUDWidth);
            LOG_F(INFO, "DMC 1: HUD Width: New HUD width = %.8f.", fDMC1_NewHUDWidth);
        }
        else if (!DMC1_HUDWidthScanResult)
        {
            LOG_F(INFO, "DMC 1: HUD Width: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc2.exe" && bHUDFix)
    {
        // DMC 2: HUD width
        uint8_t* DMC2_HUDWidthScanResult = Memory::PatternScan(baseModule, "0F ?? ?? ?? ?? 00 00 89 ?? 0F ?? ?? ?? ?? 00 00 41 ?? ??");
        if (DMC2_HUDWidthScanResult)
        {
            iDMC2_NewHUDWidth = (int)fHUDWidth;

            DWORD64 DMC2_HUDWidthAddress = (uintptr_t)DMC2_HUDWidthScanResult;
            int DMC2_HUDWidthHookLength = Memory::GetHookLength((char*)DMC2_HUDWidthAddress, 13);
            DMC2_HUDWidthReturnJMP = DMC2_HUDWidthAddress + DMC2_HUDWidthHookLength;
            Memory::DetourFunction64((void*)DMC2_HUDWidthAddress, DMC2_HUDWidth_CC, DMC2_HUDWidthHookLength);

            LOG_F(INFO, "DMC 2: HUD Width: Hook length is %d bytes", DMC2_HUDWidthHookLength);
            LOG_F(INFO, "DMC 2: HUD Width: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC2_HUDWidthAddress);
        }
        else if (!DMC2_HUDWidthScanResult)
        {
            LOG_F(INFO, "DMC 2: HUD Width: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc3.exe" && bHUDFix)
    {
        // DMC 3: HUD width
        uint8_t* DMC3_HUDWidthScanResult = Memory::PatternScan(baseModule, "0F ?? ?? C7 ?? ?? 00 00 80 3F F3 ?? ?? ?? C7 ?? ?? 55 55 15 3F");
        if (DMC3_HUDWidthScanResult)
        {
            DWORD64 DMC3_HUDWidthAddress = (uintptr_t)DMC3_HUDWidthScanResult + 0x6;
            LOG_F(INFO, "DMC 3: HUD Width: Address is 0x%" PRIxPTR, (uintptr_t)DMC3_HUDWidthAddress);

            float DMC3_fNewHUDWidth = (float)1 / (((float)iResX / iResY) / fNativeAspect);
            Memory::Write(DMC3_HUDWidthAddress, DMC3_fNewHUDWidth);
            LOG_F(INFO, "DMC 3: HUD Width: Menu width value = %.4f", DMC3_fNewHUDWidth);
        }
        else if (!DMC3_HUDWidthScanResult)
        {
            LOG_F(INFO, "DMC 3: HUD Width: Pattern scan failed.");
        }

        // DMC 3: Text width
        uint8_t* DMC3_TextWidthScanResult = Memory::PatternScan(baseModule, "0F B7 ?? ?? ?? ?? ?? F3 0F ?? ?? 0F ?? ?? F3 0F ?? ?? F3 0F ?? ??");
        if (DMC3_TextWidthScanResult)
        {
            fDMC3_NewTextWidth = fDMC3_DefaultTextWidth / fAspectMultiplier;
            LOG_F(INFO, "DMC 3: Text Width: New text width = %.4f", fDMC3_NewTextWidth);

            DWORD64 DMC3_TextWidthAddress = (uintptr_t)DMC3_TextWidthScanResult + 0x7;
            int DMC3_TextWidthHookLength = Memory::GetHookLength((char*)DMC3_TextWidthAddress, 13);
            DMC3_TextWidthReturnJMP = DMC3_TextWidthAddress + DMC3_TextWidthHookLength;
            Memory::DetourFunction64((void*)DMC3_TextWidthAddress, DMC3_TextWidth_CC, DMC3_TextWidthHookLength);

            LOG_F(INFO, "DMC 3: Text Width: Hook length is %d bytes", DMC3_TextWidthHookLength);
            LOG_F(INFO, "DMC 3: Text Width: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC3_TextWidthAddress);
        }
        else if (!DMC3_TextWidthScanResult)
        {
            LOG_F(INFO, "DMC 3: Text Width: Pattern scan failed.");
        }

        // DMC 3: Combo meter
        uint8_t* DMC3_ComboMeterScanResult = Memory::PatternScan(baseModule, "41 ?? ?? ?? ?? 00 F3 48 ?? ?? ?? F3 0F ?? ?? 66 ?? ?? ?? ?? E8 ?? ?? ?? ??");
        if (DMC3_ComboMeterScanResult)
        {
            fDMC3_ComboMeterValue = fHUDOffset * (float)0.1125;
            LOG_F(INFO, "DMC 3: Combo Meter: New combo meter width = %.4f", fDMC3_ComboMeterValue);

            DWORD64 DMC3_ComboMeterAddress = (uintptr_t)DMC3_ComboMeterScanResult;
            int DMC3_ComboMeterHookLength = Memory::GetHookLength((char*)DMC3_ComboMeterAddress, 13);
            DMC3_ComboMeterReturnJMP = DMC3_ComboMeterAddress + DMC3_ComboMeterHookLength;
            Memory::DetourFunction64((void*)DMC3_ComboMeterAddress, DMC3_ComboMeter_CC, DMC3_ComboMeterHookLength);

            LOG_F(INFO, "DMC 3: Combo Meter: Hook length is %d bytes", DMC3_ComboMeterHookLength);
            LOG_F(INFO, "DMC 3: Combo Meter: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC3_ComboMeterAddress);
        }
        else if (!DMC3_ComboMeterScanResult)
        {
            LOG_F(INFO, "DMC 3: Combo Meter: Pattern scan failed.");
        }
    }
}

void MovieFix()
{
    if ( sExeName == "dmc1.exe" && bMovieFix)
    {
        // DMC 1: Movie fix
        uint8_t* DMC1_MovieScanResult = Memory::PatternScan(baseModule, "F3 48 ?? ?? ?? F3 0F ?? ?? ?? ?? F3 0F ?? ?? ?? ?? 48 ?? ?? FF ?? ?? ?? ?? 00");
        if (DMC1_MovieScanResult)
        {
            DWORD64 DMC1_MovieAddress = (uintptr_t)DMC1_MovieScanResult;
            int DMC1_MovieHookLength = Memory::GetHookLength((char*)DMC1_MovieAddress, 13);
            DMC1_MovieReturnJMP = DMC1_MovieAddress + DMC1_MovieHookLength;
            Memory::DetourFunction64((void*)DMC1_MovieAddress, DMC1_Movie_CC, DMC1_MovieHookLength);

            LOG_F(INFO, "DMC 1: Movie: Hook length is %d bytes", DMC1_MovieHookLength);
            LOG_F(INFO, "DMC 1: Movie: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC1_MovieAddress);
        }
        else if (!DMC1_MovieScanResult)
        {
            LOG_F(INFO, "DMC 1: Movie: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc2.exe" && bMovieFix)
    {
        // DMC 2: Movie fix
        uint8_t* DMC2_MovieScanResult = Memory::PatternScan(baseModule, "41 ?? ?? ?? F3 48 ?? ?? C0 F3 0F ?? ?? ?? F3 0F ?? ?? ?? 48 ?? ?? FF ?? ?? ?? ?? 00");
        if (DMC2_MovieScanResult)
        {
            DWORD64 DMC2_MovieAddress = (uintptr_t)DMC2_MovieScanResult + 0x4;
            int DMC2_MovieHookLength = Memory::GetHookLength((char*)DMC2_MovieAddress, 13);
            DMC2_MovieReturnJMP = DMC2_MovieAddress + DMC2_MovieHookLength;
            Memory::DetourFunction64((void*)DMC2_MovieAddress, DMC2_Movie_CC, DMC2_MovieHookLength);

            LOG_F(INFO, "DMC 2: Movie: Hook length is %d bytes", DMC2_MovieHookLength);
            LOG_F(INFO, "DMC 2: Movie: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC2_MovieAddress);
        }
        else if (!DMC2_MovieScanResult)
        {
            LOG_F(INFO, "DMC 2: Movie: Pattern scan failed.");
        }
    }
    else if (sExeName == "dmc3.exe" && bMovieFix)
    {
        // DMC 3: Movie fix
        uint8_t* DMC3_MovieScanResult = Memory::PatternScan(baseModule, "F3 48 ?? ?? ?? F3 0F ?? ?? ?? ?? F3 0F ?? ?? ?? ?? 48 ?? ?? FF ?? ?? ?? ?? 00");
        if (DMC3_MovieScanResult)
        {
            DWORD64 DMC3_MovieAddress = (uintptr_t)DMC3_MovieScanResult;
            int DMC3_MovieHookLength = Memory::GetHookLength((char*)DMC3_MovieAddress, 13);
            DMC3_MovieReturnJMP = DMC3_MovieAddress + DMC3_MovieHookLength;
            Memory::DetourFunction64((void*)DMC3_MovieAddress, DMC3_Movie_CC, DMC3_MovieHookLength);

            LOG_F(INFO, "DMC 3: Movie: Hook length is %d bytes", DMC3_MovieHookLength);
            LOG_F(INFO, "DMC 3: Movie: Hook address is 0x%" PRIxPTR, (uintptr_t)DMC3_MovieAddress);
        }
        else if (!DMC3_MovieScanResult)
        {
            LOG_F(INFO, "DMC 3: Movie: Pattern scan failed.");
        }
    }
}

DWORD __stdcall Main(void*)
{
    Logging();
    ReadConfig();
    DetectGame();
    Sleep(iInjectionDelay);
    ResolutionUnlock();
    ResolutionCheck();
    AspectFOVFix();
    HUDFix();
    MovieFix();
    return true; // end thread
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE mainHandle = CreateThread(NULL, 0, Main, 0, NULL, 0);

        if (mainHandle)
        {
            CloseHandle(mainHandle);
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

