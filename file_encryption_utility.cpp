#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <fstream>
#include <vector>
#include <wincrypt.h>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

#define IDC_FILE_EDIT 101
#define IDC_KEY_EDIT 102
#define IDC_BROWSE_BUTTON 103
#define IDC_ENCRYPT_BUTTON 104
#define IDC_DECRYPT_BUTTON 105

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddControls(HWND);
void BrowseFile(HWND);
bool EncryptFileX(const std::wstring &, const std::wstring &);
bool DecryptFile(const std::wstring &, const std::wstring &);

std::string wstringToString(const std::wstring &wstr)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], bufferSize, NULL, NULL);
    return str;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"FileEncryptor";
    wc.lpfnWndProc = WndProc;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"FileEncryptor", L"File Encryption Tool", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 500, 200, NULL, NULL, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (wp)
        {
        case IDC_BROWSE_BUTTON:
            BrowseFile(hwnd);
            break;
        case IDC_ENCRYPT_BUTTON:
        {
            wchar_t filePath[256], key[256];
            GetDlgItemText(hwnd, IDC_FILE_EDIT, filePath, 256);
            GetDlgItemText(hwnd, IDC_KEY_EDIT, key, 256);
            if (EncryptFileX(filePath, key))
            {
                MessageBox(hwnd, L"File Encrypted Successfully", L"Success", MB_OK);
            }
            else
            {
                std::cout << GetLastError();
                MessageBox(hwnd, L"Encryption Failed", L"Error", MB_OK);
            }
            break;
        }
        case IDC_DECRYPT_BUTTON:
        {
            wchar_t filePath[256], key[256];
            GetDlgItemText(hwnd, IDC_FILE_EDIT, filePath, 256);
            GetDlgItemText(hwnd, IDC_KEY_EDIT, key, 256);
            if (DecryptFile(filePath, key))
            {
                std::cout << GetLastError();
                MessageBox(hwnd, L"File Decrypted Successfully", L"Success", MB_OK);
            }
            else
            {
                MessageBox(hwnd, L"Decryption Failed", L"Error", MB_OK);
            }
            break;
        }
        }
        break;
    case WM_CREATE:
        AddControls(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

void AddControls(HWND hwnd)
{
    CreateWindowW(L"static", L"File:", WS_VISIBLE | WS_CHILD, 20, 20, 50, 20, hwnd, NULL, NULL, NULL);
    CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 70, 20, 300, 20, hwnd, (HMENU)IDC_FILE_EDIT, NULL, NULL);
    CreateWindowW(L"button", L"Browse", WS_VISIBLE | WS_CHILD, 380, 20, 80, 20, hwnd, (HMENU)IDC_BROWSE_BUTTON, NULL, NULL);

    CreateWindowW(L"static", L"Key:", WS_VISIBLE | WS_CHILD, 20, 60, 50, 20, hwnd, NULL, NULL, NULL);
    CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 70, 60, 300, 20, hwnd, (HMENU)IDC_KEY_EDIT, NULL, NULL);

    CreateWindowW(L"button", L"Encrypt", WS_VISIBLE | WS_CHILD, 100, 100, 100, 30, hwnd, (HMENU)IDC_ENCRYPT_BUTTON, NULL, NULL);
    CreateWindowW(L"button", L"Decrypt", WS_VISIBLE | WS_CHILD, 220, 100, 100, 30, hwnd, (HMENU)IDC_DECRYPT_BUTTON, NULL, NULL);
}

void BrowseFile(HWND hwnd)
{
    OPENFILENAME ofn;
    wchar_t file_name[100] = {0};
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = sizeof(file_name) / sizeof(wchar_t);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;

    GetOpenFileName(&ofn);
    SetDlgItemText(hwnd, IDC_FILE_EDIT, ofn.lpstrFile);
}

bool EncryptDecryptFile(const std::wstring &filePath, const std::wstring &key, bool encrypt)
{
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    HCRYPTHASH hHash;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        return false;
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
        CryptReleaseContext(hProv, 0);
        return false;
    }

    std::string keyStr = wstringToString(key);
    if (!CryptHashData(hHash, (BYTE *)keyStr.c_str(), keyStr.length(), 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    CryptDestroyHash(hHash);

    HANDLE hSourceFile = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSourceFile == INVALID_HANDLE_VALUE)
    {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    std::wstring outFilepath = encrypt ? (filePath + L".enc") : (filePath + L".dec");
    HANDLE hDestFile = CreateFile(outFilepath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDestFile == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSourceFile);
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    const DWORD bufferSize = 4096;
    BYTE buffer[bufferSize];
    BOOL result;
    DWORD bytesRead, bytesWritten;
    while (ReadFile(hSourceFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0)
    {
        if (encrypt)
        {
            result = CryptEncrypt(hKey, 0, bytesRead < bufferSize, 0, buffer, &bytesRead, bufferSize);
        }
        else
        {
            result = CryptDecrypt(hKey, 0, bytesRead < bufferSize, 0, buffer, &bytesRead);
        }

        if (!result)
        {
            CloseHandle(hSourceFile);
            CloseHandle(hDestFile);
            CryptDestroyKey(hKey);
            CryptReleaseContext(hProv, 0);
            return false;
        }

        if (!WriteFile(hDestFile, buffer, bytesRead, &bytesWritten, NULL))
        {
            CloseHandle(hSourceFile);
            CloseHandle(hDestFile);
            CryptDestroyKey(hKey);
            CryptReleaseContext(hProv, 0);
            return false;
        }
    }

    CloseHandle(hSourceFile);
    CloseHandle(hDestFile);
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
    return true;
}

bool EncryptFileX(const std::wstring &filePath, const std::wstring &key)
{
    return EncryptDecryptFile(filePath, key, true);
}

bool DecryptFile(const std::wstring &filePath, const std::wstring &key)
{
    return EncryptDecryptFile(filePath, key, false);
}
