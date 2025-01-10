#include <windows.h>
#include <iostream>
#include <string>

std::string GetAuditionInstallPath()
{
    HKEY hKey;
    const std::string paths[] = {
        "SOFTWARE\\Adobe\\Audition",
        "SOFTWARE\\WOW6432Node\\Adobe\\Audition"
    };
    const std::string valueName = "InstallPath";
    char installPath[512];
    DWORD bufferSize = sizeof(installPath);
    LONG lResult;

    for (const auto& subKey : paths)
    {
        // 打开注册表项
        lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey.c_str(), 0, KEY_READ, &hKey);

        if (lResult == ERROR_SUCCESS)
        {
            // 读取注册表值
            lResult = RegQueryValueExA(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)installPath, &bufferSize);

            if (lResult == ERROR_SUCCESS)
            {
                // 成功读取值
                RegCloseKey(hKey);
                return std::string(installPath);
            }
            else
            {
                std::cerr << "Error reading registry value: " << lResult << std::endl;
            }
            RegCloseKey(hKey);
        }
        else
        {
            std::cerr << "Error opening registry key: " << lResult << std::endl;
        }
    }

    return "";
}

int main()
{
    std::string installPath = GetAuditionInstallPath();
    if (!installPath.empty())
    {
        std::cout << "Adobe Audition is installed at: " << installPath << std::endl;
    }
    else
    {
        std::cout << "Adobe Audition installation path not found. Adobe Audition not found. Please install it." << std::endl;
    }

    return 0;
}
