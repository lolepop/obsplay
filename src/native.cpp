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

#include <locale>
#include <codecvt>

inline std::string wstrToUtf8(std::wstring str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(str);
}

inline std::wstring utf8ToWstr(std::string str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(str);
}

std::unordered_set<std::string> Native::getAllWindows()
{

}

bool Native::isWindowOpen(std::vector<std::string> filter)
{

}
#endif
