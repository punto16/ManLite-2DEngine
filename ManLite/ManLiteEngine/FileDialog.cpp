#include "FileDialog.h"

#include <Windows.h>
#include <commdlg.h>


std::string FileDialog::OpenFile(const char* filter, const char* initialDir)
{
    OPENFILENAMEA ofn;
    CHAR sizeFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = sizeFile;
    ofn.nMaxFile = sizeof(sizeFile);
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE)
        return ofn.lpstrFile;

    return "";
}

std::string FileDialog::SaveFile(const char* filter, const char* initialDir)
{
    OPENFILENAMEA ofn;
    CHAR sizeFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = sizeFile;
    ofn.nMaxFile = sizeof(sizeFile);
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn) == TRUE)
        return ofn.lpstrFile;

    return "";
}