#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include "modding_api.h"

#pragma comment(lib, "psapi.lib")

// Forward all exports to the original DLL
#pragma comment(linker, "/export:WinHttpAddRequestHeaders=winhttp_orig.WinHttpAddRequestHeaders,@1")
#pragma comment(linker, "/export:WinHttpAddRequestHeadersEx=winhttp_orig.WinHttpAddRequestHeadersEx,@2")
#pragma comment(linker, "/export:WinHttpAutoProxySvcMain=winhttp_orig.WinHttpAutoProxySvcMain,@3")
#pragma comment(linker, "/export:WinHttpCheckPlatform=winhttp_orig.WinHttpCheckPlatform,@4")
#pragma comment(linker, "/export:WinHttpCloseHandle=winhttp_orig.WinHttpCloseHandle,@5")
#pragma comment(linker, "/export:WinHttpConnect=winhttp_orig.WinHttpConnect,@6")
#pragma comment(linker, "/export:WinHttpConnectionDeletePolicyEntries=winhttp_orig.WinHttpConnectionDeletePolicyEntries,@7")
#pragma comment(linker, "/export:WinHttpConnectionDeleteProxyInfo=winhttp_orig.WinHttpConnectionDeleteProxyInfo,@8")
#pragma comment(linker, "/export:WinHttpConnectionFreeNameList=winhttp_orig.WinHttpConnectionFreeNameList,@9")
#pragma comment(linker, "/export:WinHttpConnectionFreeProxyInfo=winhttp_orig.WinHttpConnectionFreeProxyInfo,@10")
#pragma comment(linker, "/export:WinHttpConnectionFreeProxyList=winhttp_orig.WinHttpConnectionFreeProxyList,@11")
#pragma comment(linker, "/export:WinHttpConnectionGetNameList=winhttp_orig.WinHttpConnectionGetNameList,@12")
#pragma comment(linker, "/export:WinHttpConnectionGetProxyInfo=winhttp_orig.WinHttpConnectionGetProxyInfo,@13")
#pragma comment(linker, "/export:WinHttpConnectionGetProxyList=winhttp_orig.WinHttpConnectionGetProxyList,@14")
#pragma comment(linker, "/export:WinHttpConnectionOnlyConvert=winhttp_orig.WinHttpConnectionOnlyConvert,@15")
#pragma comment(linker, "/export:WinHttpConnectionOnlyReceive=winhttp_orig.WinHttpConnectionOnlyReceive,@16")
#pragma comment(linker, "/export:WinHttpConnectionOnlySend=winhttp_orig.WinHttpConnectionOnlySend,@17")
#pragma comment(linker, "/export:WinHttpConnectionSetPolicyEntries=winhttp_orig.WinHttpConnectionSetPolicyEntries,@18")
#pragma comment(linker, "/export:WinHttpConnectionSetProxyInfo=winhttp_orig.WinHttpConnectionSetProxyInfo,@19")
#pragma comment(linker, "/export:WinHttpConnectionUpdateIfIndexTable=winhttp_orig.WinHttpConnectionUpdateIfIndexTable,@20")
#pragma comment(linker, "/export:WinHttpCrackUrl=winhttp_orig.WinHttpCrackUrl,@21")
#pragma comment(linker, "/export:WinHttpCreateProxyList=winhttp_orig.WinHttpCreateProxyList,@22")
#pragma comment(linker, "/export:WinHttpCreateProxyManager=winhttp_orig.WinHttpCreateProxyManager,@23")
#pragma comment(linker, "/export:WinHttpCreateProxyResolver=winhttp_orig.WinHttpCreateProxyResolver,@24")
#pragma comment(linker, "/export:WinHttpCreateProxyResult=winhttp_orig.WinHttpCreateProxyResult,@25")
#pragma comment(linker, "/export:WinHttpCreateUiCompatibleProxyString=winhttp_orig.WinHttpCreateUiCompatibleProxyString,@26")
#pragma comment(linker, "/export:WinHttpCreateUrl=winhttp_orig.WinHttpCreateUrl,@27")
#pragma comment(linker, "/export:WinHttpDetectAutoProxyConfigUrl=winhttp_orig.WinHttpDetectAutoProxyConfigUrl,@28")
#pragma comment(linker, "/export:WinHttpFreeProxyResult=winhttp_orig.WinHttpFreeProxyResult,@29")
#pragma comment(linker, "/export:WinHttpFreeProxyResultEx=winhttp_orig.WinHttpFreeProxyResultEx,@30")
#pragma comment(linker, "/export:WinHttpFreeProxySettings=winhttp_orig.WinHttpFreeProxySettings,@31")
#pragma comment(linker, "/export:WinHttpFreeProxySettingsEx=winhttp_orig.WinHttpFreeProxySettingsEx,@32")
#pragma comment(linker, "/export:WinHttpFreeQueryConnectionGroupResult=winhttp_orig.WinHttpFreeQueryConnectionGroupResult,@33")
#pragma comment(linker, "/export:WinHttpGetDefaultProxyConfiguration=winhttp_orig.WinHttpGetDefaultProxyConfiguration,@34")
#pragma comment(linker, "/export:WinHttpGetIEProxyConfigForCurrentUser=winhttp_orig.WinHttpGetIEProxyConfigForCurrentUser,@35")
#pragma comment(linker, "/export:WinHttpGetProxyForUrl=winhttp_orig.WinHttpGetProxyForUrl,@36")
#pragma comment(linker, "/export:WinHttpGetProxyForUrlEx=winhttp_orig.WinHttpGetProxyForUrlEx,@37")
#pragma comment(linker, "/export:WinHttpGetProxyForUrlEx2=winhttp_orig.WinHttpGetProxyForUrlEx2,@38")
#pragma comment(linker, "/export:WinHttpGetProxyForUrlHvsi=winhttp_orig.WinHttpGetProxyForUrlHvsi,@39")
#pragma comment(linker, "/export:WinHttpGetProxyResult=winhttp_orig.WinHttpGetProxyResult,@40")
#pragma comment(linker, "/export:WinHttpGetProxyResultEx=winhttp_orig.WinHttpGetProxyResultEx,@41")
#pragma comment(linker, "/export:WinHttpGetProxySettingsEx=winhttp_orig.WinHttpGetProxySettingsEx,@42")
#pragma comment(linker, "/export:WinHttpGetProxySettingsResultEx=winhttp_orig.WinHttpGetProxySettingsResultEx,@43")
#pragma comment(linker, "/export:WinHttpGetProxySettingsVersion=winhttp_orig.WinHttpGetProxySettingsVersion,@44")
#pragma comment(linker, "/export:WinHttpGetTunnelSocket=winhttp_orig.WinHttpGetTunnelSocket,@45")
#pragma comment(linker, "/export:WinHttpOpen=winhttp_orig.WinHttpOpen,@46")
#pragma comment(linker, "/export:WinHttpOpenRequest=winhttp_orig.WinHttpOpenRequest,@47")
#pragma comment(linker, "/export:WinHttpPacJsWorkerMain=winhttp_orig.WinHttpPacJsWorkerMain,@48")
#pragma comment(linker, "/export:WinHttpProbeConnectivity=winhttp_orig.WinHttpProbeConnectivity,@49")
#pragma comment(linker, "/export:WinHttpProtocolCompleteUpgrade=winhttp_orig.WinHttpProtocolCompleteUpgrade,@50")
#pragma comment(linker, "/export:WinHttpProtocolReceive=winhttp_orig.WinHttpProtocolReceive,@51")
#pragma comment(linker, "/export:WinHttpProtocolSend=winhttp_orig.WinHttpProtocolSend,@52")
#pragma comment(linker, "/export:WinHttpQueryAuthSchemes=winhttp_orig.WinHttpQueryAuthSchemes,@53")
#pragma comment(linker, "/export:WinHttpQueryConnectionGroup=winhttp_orig.WinHttpQueryConnectionGroup,@54")
#pragma comment(linker, "/export:WinHttpQueryDataAvailable=winhttp_orig.WinHttpQueryDataAvailable,@55")
#pragma comment(linker, "/export:WinHttpQueryHeaders=winhttp_orig.WinHttpQueryHeaders,@56")
#pragma comment(linker, "/export:WinHttpQueryHeadersEx=winhttp_orig.WinHttpQueryHeadersEx,@57")
#pragma comment(linker, "/export:WinHttpQueryOption=winhttp_orig.WinHttpQueryOption,@58")
#pragma comment(linker, "/export:WinHttpReadData=winhttp_orig.WinHttpReadData,@59")
#pragma comment(linker, "/export:WinHttpReadDataEx=winhttp_orig.WinHttpReadDataEx,@60")
#pragma comment(linker, "/export:WinHttpReadProxySettings=winhttp_orig.WinHttpReadProxySettings,@61")
#pragma comment(linker, "/export:WinHttpReadProxySettingsHvsi=winhttp_orig.WinHttpReadProxySettingsHvsi,@62")
#pragma comment(linker, "/export:WinHttpReceiveResponse=winhttp_orig.WinHttpReceiveResponse,@63")
#pragma comment(linker, "/export:WinHttpRefreshProxySettings=winhttp_orig.WinHttpRefreshProxySettings,@64")
#pragma comment(linker, "/export:WinHttpRegisterProxyChangeNotification=winhttp_orig.WinHttpRegisterProxyChangeNotification,@65")
#pragma comment(linker, "/export:WinHttpResetAutoProxy=winhttp_orig.WinHttpResetAutoProxy,@66")
#pragma comment(linker, "/export:WinHttpResolverGetProxyForUrl=winhttp_orig.WinHttpResolverGetProxyForUrl,@67")
#pragma comment(linker, "/export:WinHttpSaveProxyCredentials=winhttp_orig.WinHttpSaveProxyCredentials,@68")
#pragma comment(linker, "/export:WinHttpSendRequest=winhttp_orig.WinHttpSendRequest,@69")
#pragma comment(linker, "/export:WinHttpSetCredentials=winhttp_orig.WinHttpSetCredentials,@70")
#pragma comment(linker, "/export:WinHttpSetDefaultProxyConfiguration=winhttp_orig.WinHttpSetDefaultProxyConfiguration,@71")
#pragma comment(linker, "/export:WinHttpSetOption=winhttp_orig.WinHttpSetOption,@72")
#pragma comment(linker, "/export:WinHttpSetProxySettingsPerUser=winhttp_orig.WinHttpSetProxySettingsPerUser,@73")
#pragma comment(linker, "/export:WinHttpSetSecureLegacyServersAppCompat=winhttp_orig.WinHttpSetSecureLegacyServersAppCompat,@74")
#pragma comment(linker, "/export:WinHttpSetStatusCallback=winhttp_orig.WinHttpSetStatusCallback,@75")
#pragma comment(linker, "/export:WinHttpSetTimeouts=winhttp_orig.WinHttpSetTimeouts,@76")
#pragma comment(linker, "/export:WinHttpTimeFromSystemTime=winhttp_orig.WinHttpTimeFromSystemTime,@77")
#pragma comment(linker, "/export:WinHttpTimeToSystemTime=winhttp_orig.WinHttpTimeToSystemTime,@78")
#pragma comment(linker, "/export:WinHttpUnregisterProxyChangeNotification=winhttp_orig.WinHttpUnregisterProxyChangeNotification,@79")
#pragma comment(linker, "/export:WinHttpWebSocketClose=winhttp_orig.WinHttpWebSocketClose,@80")
#pragma comment(linker, "/export:WinHttpWebSocketCompleteUpgrade=winhttp_orig.WinHttpWebSocketCompleteUpgrade,@81")
#pragma comment(linker, "/export:WinHttpWebSocketQueryCloseStatus=winhttp_orig.WinHttpWebSocketQueryCloseStatus,@82")
#pragma comment(linker, "/export:WinHttpWebSocketReceive=winhttp_orig.WinHttpWebSocketReceive,@83")
#pragma comment(linker, "/export:WinHttpWebSocketSend=winhttp_orig.WinHttpWebSocketSend,@84")
#pragma comment(linker, "/export:WinHttpWebSocketShutdown=winhttp_orig.WinHttpWebSocketShutdown,@85")
#pragma comment(linker, "/export:WinHttpWriteData=winhttp_orig.WinHttpWriteData,@86")
#pragma comment(linker, "/export:WinHttpWriteProxySettings=winhttp_orig.WinHttpWriteProxySettings,@87")

bool IsTargetExecutable() {
    char path[MAX_PATH];
    if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) {
        return false;
    }
    
    // Extract filename from full path
    const char* exeName = strrchr(path, '\\');
    exeName = exeName ? exeName + 1 : path;
    
    // Check if this is MIO.exe (case-insensitive)
    return _stricmp(exeName, "MIO.exe") == 0;
}

void LoadMods() {
    LogMessage("Loading mods from ./mods/ directory...");
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(".\\mods\\*.dll", &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        LogMessage("No mods found or mods directory doesn't exist.");
        return;
    }
    
    int modCount = 0;
    do {
        char modPath[MAX_PATH];
        sprintf_s(modPath, ".\\mods\\%s", findData.cFileName);
        
        HMODULE hMod = LoadLibraryA(modPath);
        if (hMod) {
            char msg[256];
            sprintf_s(msg, "Loaded mod: %s", findData.cFileName);
            LogMessage(msg);
            
            // Try to call the mod's initialization function
            typedef void (*ModInitFunc)();
            ModInitFunc modInit = (ModInitFunc)GetProcAddress(hMod, "ModInit");
            if (modInit) {
                sprintf_s(msg, "Initializing %s...", findData.cFileName);
                LogMessage(msg);
                modInit();
            } else {
                sprintf_s(msg, "Warning: %s has no ModInit() function", findData.cFileName);
                LogMessage(msg);
            }
            
            modCount++;
        } else {
            char msg[256];
            sprintf_s(msg, "Failed to load: %s (Error: %d)", findData.cFileName, GetLastError());
            LogMessage(msg);
        }
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
    
    char msg[256];
    sprintf_s(msg, "Loaded %d mod(s)", modCount);
    LogMessage(msg);
}

void InitializeModAPI() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    printf("==============================================\n");
    printf("        Game Modding API v%d.%d.%d\n", 
           MODDING_API_VERSION_MAJOR, 
           MODDING_API_VERSION_MINOR, 
           MODDING_API_VERSION_PATCH);
    printf("==============================================\n");
    
    LogMessage("Modding API initialized!");
    
    // Initialize addresses from Cheat Engine findings
    InitializeAddresses();
    
    printf("==============================================\n\n");
    
    // Create mods directory if it doesn't exist
    CreateDirectoryA(".\\mods", NULL);
    
    // Load all mods
    LoadMods();
    
    printf("\n==============================================\n");
    printf("Modding API ready!\n");
    printf("==============================================\n\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        
        // Only initialize modding API if this is MIO.exe
        if (!IsTargetExecutable()) {
            return TRUE;  // Still load the DLL, just don't initialize mods
        }
        
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)InitializeModAPI, hModule, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}