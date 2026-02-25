#include "modding_api.h"
#include <dwmapi.h>
#include <psapi.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dwmapi.lib")

// Forward all exports to the original DLL
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpAddRequestHeaders=C:\\Windows\\System32\\WINHTTP.WinHttpAddRequestHeaders,@1")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpAddRequestHeadersEx=C:\\Windows\\System32\\WINHTTP.WinHttpAddRequestHeadersEx,@2")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpAutoProxySvcMain=C:\\Windows\\System32\\WINHTTP.WinHttpAutoProxySvcMain,@3")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCheckPlatform=C:\\Windows\\System32\\WINHTTP.WinHttpCheckPlatform,@4")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCloseHandle=C:\\Windows\\System32\\WINHTTP.WinHttpCloseHandle,@5")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnect=C:\\Windows\\System32\\WINHTTP.WinHttpConnect,@6")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionDeletePolicyEntries=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionDeletePolicyEntries,@7")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionDeleteProxyInfo=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionDeleteProxyInfo,@8")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionFreeNameList=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionFreeNameList,@9")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionFreeProxyInfo=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionFreeProxyInfo,@10")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionFreeProxyList=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionFreeProxyList,@11")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionGetNameList=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionGetNameList,@12")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionGetProxyInfo=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionGetProxyInfo,@13")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionGetProxyList=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionGetProxyList,@14")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionOnlyConvert=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionOnlyConvert,@15")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionOnlyReceive=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionOnlyReceive,@16")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionOnlySend=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionOnlySend,@17")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionSetPolicyEntries=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionSetPolicyEntries,@18")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionSetProxyInfo=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionSetProxyInfo,@19")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpConnectionUpdateIfIndexTable=C:\\Windows\\System32\\WINHTTP.WinHttpConnectionUpdateIfIndexTable,@20")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCrackUrl=C:\\Windows\\System32\\WINHTTP.WinHttpCrackUrl,@21")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateProxyList=C:\\Windows\\System32\\WINHTTP.WinHttpCreateProxyList,@22")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateProxyManager=C:\\Windows\\System32\\WINHTTP.WinHttpCreateProxyManager,@23")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateProxyResolver=C:\\Windows\\System32\\WINHTTP.WinHttpCreateProxyResolver,@24")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateProxyResult=C:\\Windows\\System32\\WINHTTP.WinHttpCreateProxyResult,@25")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateUiCompatibleProxyString=C:\\Windows\\System32\\WINHTTP.WinHttpCreateUiCompatibleProxyString,@26")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpCreateUrl=C:\\Windows\\System32\\WINHTTP.WinHttpCreateUrl,@27")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpDetectAutoProxyConfigUrl=C:\\Windows\\System32\\WINHTTP.WinHttpDetectAutoProxyConfigUrl,@28")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpFreeProxyResult=C:\\Windows\\System32\\WINHTTP.WinHttpFreeProxyResult,@29")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpFreeProxyResultEx=C:\\Windows\\System32\\WINHTTP.WinHttpFreeProxyResultEx,@30")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpFreeProxySettings=C:\\Windows\\System32\\WINHTTP.WinHttpFreeProxySettings,@31")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpFreeProxySettingsEx=C:\\Windows\\System32\\WINHTTP.WinHttpFreeProxySettingsEx,@32")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpFreeQueryConnectionGroupResult=C:\\Windows\\System32\\WINHTTP.WinHttpFreeQueryConnectionGroupResult,@33")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetDefaultProxyConfiguration=C:\\Windows\\System32\\WINHTTP.WinHttpGetDefaultProxyConfiguration,@34")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetIEProxyConfigForCurrentUser=C:\\Windows\\System32\\WINHTTP.WinHttpGetIEProxyConfigForCurrentUser,@35")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyForUrl=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyForUrl,@36")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyForUrlEx=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyForUrlEx,@37")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyForUrlEx2=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyForUrlEx2,@38")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyForUrlHvsi=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyForUrlHvsi,@39")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyResult=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyResult,@40")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxyResultEx=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxyResultEx,@41")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxySettingsEx=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxySettingsEx,@42")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxySettingsResultEx=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxySettingsResultEx,@43")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetProxySettingsVersion=C:\\Windows\\System32\\WINHTTP.WinHttpGetProxySettingsVersion,@44")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpGetTunnelSocket=C:\\Windows\\System32\\WINHTTP.WinHttpGetTunnelSocket,@45")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpOpen=C:\\Windows\\System32\\WINHTTP.WinHttpOpen,@46")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpOpenRequest=C:\\Windows\\System32\\WINHTTP.WinHttpOpenRequest,@47")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpPacJsWorkerMain=C:\\Windows\\System32\\WINHTTP.WinHttpPacJsWorkerMain,@48")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpProbeConnectivity=C:\\Windows\\System32\\WINHTTP.WinHttpProbeConnectivity,@49")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpProtocolCompleteUpgrade=C:\\Windows\\System32\\WINHTTP.WinHttpProtocolCompleteUpgrade,@50")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpProtocolReceive=C:\\Windows\\System32\\WINHTTP.WinHttpProtocolReceive,@51")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpProtocolSend=C:\\Windows\\System32\\WINHTTP.WinHttpProtocolSend,@52")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryAuthSchemes=C:\\Windows\\System32\\WINHTTP.WinHttpQueryAuthSchemes,@53")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryConnectionGroup=C:\\Windows\\System32\\WINHTTP.WinHttpQueryConnectionGroup,@54")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryDataAvailable=C:\\Windows\\System32\\WINHTTP.WinHttpQueryDataAvailable,@55")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryHeaders=C:\\Windows\\System32\\WINHTTP.WinHttpQueryHeaders,@56")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryHeadersEx=C:\\Windows\\System32\\WINHTTP.WinHttpQueryHeadersEx,@57")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpQueryOption=C:\\Windows\\System32\\WINHTTP.WinHttpQueryOption,@58")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpReadData=C:\\Windows\\System32\\WINHTTP.WinHttpReadData,@59")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpReadDataEx=C:\\Windows\\System32\\WINHTTP.WinHttpReadDataEx,@60")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpReadProxySettings=C:\\Windows\\System32\\WINHTTP.WinHttpReadProxySettings,@61")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpReadProxySettingsHvsi=C:\\Windows\\System32\\WINHTTP.WinHttpReadProxySettingsHvsi,@62")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpReceiveResponse=C:\\Windows\\System32\\WINHTTP.WinHttpReceiveResponse,@63")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpRefreshProxySettings=C:\\Windows\\System32\\WINHTTP.WinHttpRefreshProxySettings,@64")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpRegisterProxyChangeNotification=C:\\Windows\\System32\\WINHTTP.WinHttpRegisterProxyChangeNotification,@65")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpResetAutoProxy=C:\\Windows\\System32\\WINHTTP.WinHttpResetAutoProxy,@66")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpResolverGetProxyForUrl=C:\\Windows\\System32\\WINHTTP.WinHttpResolverGetProxyForUrl,@67")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSaveProxyCredentials=C:\\Windows\\System32\\WINHTTP.WinHttpSaveProxyCredentials,@68")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSendRequest=C:\\Windows\\System32\\WINHTTP.WinHttpSendRequest,@69")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetCredentials=C:\\Windows\\System32\\WINHTTP.WinHttpSetCredentials,@70")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetDefaultProxyConfiguration=C:\\Windows\\System32\\WINHTTP.WinHttpSetDefaultProxyConfiguration,@71")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetOption=C:\\Windows\\System32\\WINHTTP.WinHttpSetOption,@72")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetProxySettingsPerUser=C:\\Windows\\System32\\WINHTTP.WinHttpSetProxySettingsPerUser,@73")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetSecureLegacyServersAppCompat=C:\\Windows\\System32\\WINHTTP.WinHttpSetSecureLegacyServersAppCompat,@74")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetStatusCallback=C:\\Windows\\System32\\WINHTTP.WinHttpSetStatusCallback,@75")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpSetTimeouts=C:\\Windows\\System32\\WINHTTP.WinHttpSetTimeouts,@76")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpTimeFromSystemTime=C:\\Windows\\System32\\WINHTTP.WinHttpTimeFromSystemTime,@77")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpTimeToSystemTime=C:\\Windows\\System32\\WINHTTP.WinHttpTimeToSystemTime,@78")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpUnregisterProxyChangeNotification=C:\\Windows\\System32\\WINHTTP.WinHttpUnregisterProxyChangeNotification,@79")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketClose=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketClose,@80")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketCompleteUpgrade=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketCompleteUpgrade,@81")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketQueryCloseStatus=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketQueryCloseStatus,@82")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketReceive=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketReceive,@83")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketSend=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketSend,@84")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWebSocketShutdown=C:\\Windows\\System32\\WINHTTP.WinHttpWebSocketShutdown,@85")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWriteData=C:\\Windows\\System32\\WINHTTP.WinHttpWriteData,@86")
#pragma comment(                                                               \
    linker,                                                                    \
    "/export:WinHttpWriteProxySettings=C:\\Windows\\System32\\WINHTTP.WinHttpWriteProxySettings,@87")

bool IsTargetExecutable() {
  char path[MAX_PATH];
  if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) {
    return false;
  }

  // Extract filename from full path
  const char *exeName = strrchr(path, '\\');
  exeName = exeName ? exeName + 1 : path;

  // Check if this is MIO.exe (case-insensitive)
  return _stricmp(exeName, "MIO.exe") == 0;
}

void DisableDWM() {
  DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
  HKEY hKey;
  LSTATUS lResult;
  const wchar_t *subKeyPath = L"Software\\Microsoft\\Windows "
                              L"NT\\CurrentVersion\\AppCompatFlags\\Layers";
  const wchar_t *valueName =
      L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\MIO\\mio.exe";
  const wchar_t *valueData = L"~ DISABLEDXMAXIMIZEDWINDOWEDMODE";

  lResult = RegCreateKeyExW(HKEY_CURRENT_USER, subKeyPath, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                            &hKey, NULL);

  if (lResult != ERROR_SUCCESS) {
    printf("Error creating/opening registry key! GUI mods might not work.\n");
    return;
  }

  DWORD dataSize = (DWORD)((wcslen(valueData) + 1) * sizeof(wchar_t));

  lResult =
      RegSetValueExW(hKey, valueName, 0, REG_SZ, (LPBYTE)valueData, dataSize);

  if (lResult != ERROR_SUCCESS) {
    printf("Error setting registry key! GUI mods might not work.\n");
    return;
  } else {
    printf("DWM rendering disabled successfully. Please relaunch your game if "
           "GUI mods continue to not work.\n");
  }

  RegCloseKey(hKey);
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
        sprintf_s(msg, "Warning: %s has no ModInit() function",
                  findData.cFileName);
        LogMessage(msg);
      }

      modCount++;
    } else {
      char msg[256];
      sprintf_s(msg, "Failed to load: %s (Error: %d)", findData.cFileName,
                GetLastError());
      LogMessage(msg);
    }
  } while (FindNextFileA(hFind, &findData));

  FindClose(hFind);

  char msg[256];
  sprintf_s(msg, "Loaded %d mod(s)", modCount);
  LogMessage(msg);
}

void InitializeModAPI() {
  // Create mods directory if it doesn't exist
  CreateDirectoryA(".\\mods", NULL);

  // Create modconfig directory if it doesn't exist
  CreateDirectoryA(".\\modconfig", NULL);

  AllocConsole();
  FILE *f;
  freopen_s(&f, "CONOUT$", "w", stdout);
  FILE *fp;
  freopen_s(&fp, ".\\mods\\error.txt", "w", stderr);
  setvbuf(stderr, nullptr, _IONBF, 0);

  printf("==============================================\n");
  printf("        Game Modding API v%d.%d.%d\n", MODDING_API_VERSION_MAJOR,
         MODDING_API_VERSION_MINOR, MODDING_API_VERSION_PATCH);
  printf("==============================================\n");

  LogMessage("Modding API initialized!");

  // Initialize addresses from Cheat Engine findings
  InitializeAddresses();

  printf("Disabling DWM...");
  DisableDWM(); // Needed for GUI mods on some systems

  printf("==============================================\n\n");

  // Load all mods
  LoadMods();

  printf("\n==============================================\n");
  printf("Modding API ready!\n");
  printf("==============================================\n\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(hModule);

    // Only initialize modding API if this is MIO.exe
    if (!IsTargetExecutable()) {
      return TRUE; // Still load the DLL, just don't initialize mods
    }

    CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)InitializeModAPI, hModule,
                 0, nullptr);
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}