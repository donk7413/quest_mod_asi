#include <string_view>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std::literals;
using namespace std::chrono_literals;

constexpr auto MODNAME = "quest_mod"sv;

static std::filesystem::path rootDir;
static HANDLE modInstanceMutex { nullptr };
static unsigned long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

BOOL APIENTRY DllMain(HMODULE module, DWORD reasonForCall, LPVOID) {

    DisableThreadLibraryCalls(module);

    switch(reasonForCall) {
        case DLL_PROCESS_ATTACH: {

            // Check for correct product name
            wchar_t exePathBuf[MAX_PATH] { 0 };
            GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, std::size(exePathBuf));
            std::filesystem::path exePath = exePathBuf;
            rootDir = exePath.parent_path() / "plugins\cyber_engine_tweaks\mods" / MODNAME;

            // Quit if companion was not found
            if (!std::filesystem::exists(rootDir / "init.lua")) {
                break;
            }

            bool exeValid = false;
            int verInfoSz = GetFileVersionInfoSize(exePathBuf, nullptr);
            if (verInfoSz) {
                auto verInfo = std::make_unique<BYTE[]>(verInfoSz);
                if (GetFileVersionInfo(exePathBuf, 0, verInfoSz, verInfo.get())) {
                    struct {
                        WORD Language;
                        WORD CodePage;
                    } *pTranslations;
                    // Thanks WhySoSerious?, I have no idea what this block is doing but it works :D
                    UINT transBytes = 0;
                    if (VerQueryValueW(verInfo.get(), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&pTranslations), &transBytes)) {
                        UINT dummy;
                        TCHAR* productName = nullptr;
                        TCHAR subBlock[64];
                        for (UINT i = 0; i < (transBytes / sizeof(*pTranslations)); i++) {
                            swprintf(subBlock, L"\\StringFileInfo\\%04x%04x\\ProductName", pTranslations[i].Language, pTranslations[i].CodePage);
                            if (VerQueryValueW(verInfo.get(), subBlock, reinterpret_cast<void**>(&productName), &dummy)) {
                                if (wcscmp(productName, L"Cyberpunk 2077") == 0) {
                                    exeValid = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Check for correct exe name if product name check fails
            exeValid = exeValid || (exePath.filename() == L"Cyberpunk2077.exe");

            // Quit if not attaching to CP77
            if (!exeValid) {
                break;
            }

            // Create mutex for single instancing
            modInstanceMutex = CreateMutexW(NULL, TRUE, L"ImmersiveRoleplayFramework Instance");
            if (!modInstanceMutex) {
                break;
            }
            
            // Process information
            static bool processStarted { false };
            static STARTUPINFO processStartupInfo { };     
            static PROCESS_INFORMATION processInfo { };

            // Process startup
            processStartupInfo.cb = sizeof(processStartupInfo);
          processStarted = CreateProcessW( 
                (rootDir / "tools/ImmersiveRoleplayFramework.exe").native().c_str(),   // Executable path
                nullptr,                                                                // Command line
                nullptr,                                                                // Process handle not inheritable
                nullptr,                                                                // Thread handle not inheritable
                false,                                                                  // Set handle inheritance to FALSE
                0,                                                                      // No creation flags
                nullptr,                                                                // Use parent's environment block
                nullptr,                                                                // Use parent's starting directory 
                &processStartupInfo,                                                    // Pointer to STARTUPINFO structure
                &processInfo                                                            // Pointer to PROCESS_INFORMATION structure
            );

            // Close handles (we don't need them)
            if (processStarted) {
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
            }

            break;
        }

        case DLL_PROCESS_DETACH: {
            if (modInstanceMutex) {
                ReleaseMutex(modInstanceMutex);
                modInstanceMutex = nullptr;
            }
            break;
        }

        default: {
            break;
        }
    }

    return TRUE;
}
