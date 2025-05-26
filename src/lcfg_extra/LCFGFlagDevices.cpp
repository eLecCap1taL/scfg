#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
void markLCFGHasRun()
{
#ifdef _WIN32
    HKEY hKey;
    const wchar_t* subkey = L"Software\\LCFG";
    const wchar_t* valueName = L"LCFGHasRun";
    DWORD value = 1;

    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        subkey,
        0,
        NULL,
        0,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL);

    if (result == ERROR_SUCCESS)
    {
        result = RegSetValueExW(
            hKey,
            valueName,
            0,
            REG_DWORD,
            reinterpret_cast<const BYTE*>(&value),
            sizeof(value));

        if (result != ERROR_SUCCESS)
        {
            std::cerr << "Failed to set registry value LCFGHasRun" << std::endl;
        }
        else
        {
            std::cerr << "Registry value LCFGHasRun = 1 written successfully" << std::endl;
        }

        RegCloseKey(hKey);
    }
    else
    {
        std::cerr << "Failed to open/create registry key for LCFG" << std::endl;
    }
#else
    std::cerr << "markLCFGHasRun is only implemented for Windows platforms" << std::endl;
#endif
}
