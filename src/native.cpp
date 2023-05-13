#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

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
        for (unsigned int i = 0; i < nChildren; i++)
        {
            char* name;
            auto child = children[i];
            // wtf window is in the foreground and it somehow isnt viewable
            // if (XGetWindowAttributes(display, child, &windowAttributes) && windowAttributes.map_state == IsViewable)
            // {
                if (XFetchName(display, child, &name))
                {
                    windows.insert(name);
                }
            // }
            free(name);
        }
        
    }

    XFree(children);
    XCloseDisplay(display);

    return windows;
}

bool Native::isWindowOpen(const std::vector<std::string>& filter)
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

#include <locale>
#include <codecvt>
#include <Windows.h>
#include <vector>
#include <tuple>
#include <Psapi.h>
#include <tlhelp32.h>

#ifdef WINDOWS_AC_COMPAT
// list all processes instead of processes of windows
// cannot obtain handles of some windows due to mnay anticheats stripping openprocess handles without kernel access or weird usermode hooks

class ProcessList
{
    HANDLE handle;

public:
    class Iterator
    {
        int pos = 0;
        HANDLE* hProcessSnap;
        PROCESSENTRY32W pe32;

    public:
        using iterator_category = std::input_iterator_tag;

        Iterator(HANDLE* parent) : hProcessSnap(parent), pe32(PROCESSENTRY32W())
        {
            pe32.dwSize = sizeof(PROCESSENTRY32W);
            next(true);
        }

        Iterator() : pos(-1) { }

        void next(bool first = false)
        {
            pos = Process32NextW(*hProcessSnap, &pe32) || first ? pos + 1 : -1;
        }

        PROCESSENTRY32W* operator*() { return &pe32; }
        PROCESSENTRY32W operator->() { return ***this; }
        Iterator& operator++() { next(); return *this; }
        Iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.pos == b.pos; }
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.pos != b.pos; }

    };

    Iterator begin() { return Iterator(&handle); }
    Iterator end() { return Iterator(); }

    ProcessList()
    {
        handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (handle == INVALID_HANDLE_VALUE)
            throw 0;
    }

    ~ProcessList()
    {
        CloseHandle(handle);
    }

};

std::unordered_set<std::string> Native::getAllWindows()
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::unordered_set<std::string> o;
    ProcessList p;
    for (auto i : p)
        o.insert(converter.to_bytes(i->szExeFile));
    return o;
}

bool Native::isWindowOpen(const std::vector<std::string>& filter)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::unordered_set<std::wstring> f;
    for (auto i : filter)
        f.insert(converter.from_bytes(i));

    ProcessList p;
    for (auto i : p)
        if (f.find(i->szExeFile) != f.end())
            return true;
    return false;
}
#else
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

bool Native::isWindowOpen(const std::vector<std::string>& filter)
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

#endif
