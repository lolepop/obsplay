#include "native.h"

#ifdef LINUX
// linux implementation

#include <X11/Xlib.h>

std::unordered_set<std::string> Native::getAllWindows()
{
    auto windows = std::unordered_set<std::string>();
    auto display = XOpenDisplay(0);

    Window root, parent, *children;
    unsigned int nChildren;
    if (XQueryTree(display, DefaultRootWindow(display), &root, &parent, &children, &nChildren))
    {
        // XWindowAttributes windowAttributes;
        char* name;
        for (unsigned int i = 0; i < nChildren; i++)
        {
            auto child = children[i];
            // wtf window is in the foreground and it somehow isnt viewable
            // if (XGetWindowAttributes(display, child, &windowAttributes) && windowAttributes.map_state == IsViewable)
            // {
                if (XFetchName(display, child, &name))
                {
                    windows.insert(name);
                }
            // }
        }
        
        XFree(name);
    }

    XFree(children);
    XFree(display);

    return windows;
}

bool Native::isWindowOpen(std::vector<std::string> filter)
{
    auto windows = getAllWindows();
    for (auto i = filter.begin(); i != filter.end(); i++)
    {
        if (windows.find(*i) != windows.end())
            return true;
    }
    return false;
}
#else
// windows implementation

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <locale>
#include <codecvt>
#include <Windows.h>
#include <vector>
#include <tuple>
#include <Psapi.h>

std::unordered_set<DWORD> getPidList()
{
    std::unordered_set<DWORD> pids;
    DWORD procPids[1024], cbNeeded;

    if (EnumProcesses(procPids, sizeof(procPids), &cbNeeded))
    {
        for (int i = 0; i < cbNeeded / sizeof(DWORD); i++)
            pids.insert(procPids[i]);
    }

    return pids;
}

std::wstring getProcessName(std::unordered_set<DWORD>* pidList, DWORD pid)
{
    WCHAR baseName[MAX_PATH];

    if (pidList->find(pid) != pidList->end())
    {
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        if (handle != NULL)
        {
            HMODULE hModule;
            DWORD cbNeeded;

            if (EnumProcessModules(handle, &hModule, sizeof(hModule), &cbNeeded))
            {
                GetModuleBaseNameW(handle, hModule, baseName, sizeof(baseName) / sizeof(WCHAR));
                return baseName;
            }
        }
    }

    return L"";
}

std::unordered_set<std::string> Native::getAllWindows()
{
    std::unordered_set<std::string> windows;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    auto pids = getPidList();

    auto param = std::tuple(&windows, &pids, &converter);
    EnumWindows([](HWND hwnd, LPARAM p) {
        auto [windows, pids, converter] = *reinterpret_cast<decltype(param)*>(p);

        //WCHAR windowTitle[1024];
        //GetWindowTextW(hwnd, windowTitle, 1024);

        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        auto name = getProcessName(pids, pid);

        if (name.length() > 0 && IsWindowVisible(hwnd))
            windows->insert(converter->to_bytes(name));

        return TRUE;
    }, reinterpret_cast<LPARAM>(&param));

    return windows;
}

bool Native::isWindowOpen(std::vector<std::string> filter)
{
    std::unordered_set<std::wstring> f;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    for (auto i : filter)
        f.insert(converter.from_bytes(i));

    auto pids = getPidList();
    bool found = false;

    auto param = std::tuple(&f, &pids, &found);
    EnumWindows([](HWND hwnd, LPARAM p) {
        auto [filter, pids, found] = *reinterpret_cast<decltype(param)*>(p);

        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        auto name = getProcessName(pids, pid);

        if (name.length() > 0 && filter->find(name) != filter->end() && IsWindowVisible(hwnd))
        {
            //std::wcout << name << "\n";
            *found = true;
            return FALSE;
        }

        return TRUE;
    }, reinterpret_cast<LPARAM>(&param));

    return found;
}
#endif
