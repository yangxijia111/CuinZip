#pragma once

#include <Windows.h>

#include <cwchar>
#include <string>
#include <vector>

namespace winrt::NanaZip::Modern::implementation
{
    // The Home window passes a NUL-separated file list to the file manager.
    // Validate the whole list before writing it: dropping the tail silently
    // would create an archive that is missing some of the user's files.
    inline bool TryWriteStartPaths(
        std::vector<std::wstring> const& paths,
        wchar_t* buffer,
        std::size_t capacity,
        INT32& count)
    {
        count = 0;
        if (!buffer || capacity == 0)
        {
            return paths.empty();
        }

        buffer[0] = L'\0';
        std::size_t required = 1;
        for (std::wstring const& path : paths)
        {
            if (path.empty() || path.find(L'\0') != std::wstring::npos
                || path.size() >= capacity - required)
            {
                return false;
            }
            required += path.size() + 1;
        }

        wchar_t* cursor = buffer;
        for (std::wstring const& path : paths)
        {
            std::wmemcpy(cursor, path.c_str(), path.size() + 1);
            cursor += path.size() + 1;
            ++count;
        }
        *cursor = L'\0';
        return true;
    }
}
