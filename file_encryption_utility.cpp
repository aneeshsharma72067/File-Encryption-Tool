#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <commdlg.h>
#include <fstream>
#include <vector>
#include <wincrypt.h>

#define BUTTON_ID 1

// Function Prototype's
LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
std::string ConvertWideStringToNarrowString(const std::wstring &widestring);
bool EncryptData(const std::vector<BYTE> &data, std::vector<BYTE> &encryptedData);
void ReadFileToVector(const std::string &filename, std::vector<BYTE> &buffer);
void WriteVectorToFile(const std::string &filename, std::vector<BYTE> &buffer);
void showOpenFileDialog(HWND hwnd);

// WinAPI Initialization
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t *CLASS_NAME = L"File Encrypt Window Class";
    const wchar_t *WINDOW_NAME = L"File Encryption Utility";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc))
    {
        return -1;
    }

    HWND hWnd = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, hInstance, NULL);
    if (!hWnd)
    {
        return -1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        HWND createButton = CreateWindowEx(0, L"Button", L"Select File", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 150, 150, 100, 40, hwnd, (HMENU)BUTTON_ID, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        if (!createButton)
        {
            std::cout << "Error in creating Button :" << GetLastError();
            return -1;
        }
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == BUTTON_ID)
        {
            // MessageBox(hwnd, L"Button Clicked", L"Notification", MB_ICONINFORMATION);
            showOpenFileDialog(hwnd);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void showOpenFileDialog(HWND hwnd)
{
    OPENFILENAME ofn;
    wchar_t szFile[260] = {0};

    // // Initialise OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"ALL\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        std::string fileName = "";
        std::wstring filePath = ofn.lpstrFile;

        int length = wcslen(ofn.lpstrFile);

        for (int i = length - 1; i >= 0 && ofn.lpstrFile[i] != '\\'; i--)
        {
            fileName = (char)ofn.lpstrFile[i] + fileName;
        }
        std::cout << "\n"
                  << "Selected File: " << fileName << std::endl;
        std::wcout
            << L"File Location : " << ofn.lpstrFile << std::endl;

        std::string narrowFilePath(filePath.begin(), filePath.end());

        std::vector<BYTE> data;
        ReadFileToVector(narrowFilePath, data);

        // Encrypt the data
        std::vector<BYTE> encryptedData;
        if (EncryptData(data, encryptedData))
        {
            std::string encryptedFilePath = narrowFilePath + ".enc";
            WriteVectorToFile(encryptedFilePath, encryptedData);

            std::wcout << L"Selected File: " << filePath << std::endl;
            std::cout << "Encrypted File: " << encryptedFilePath << std::endl;
        }
        else
        {
            std::cerr << "Encryption Failed. " << std::endl;
        }
    }
    else
    {
        DWORD error = CommDlgExtendedError();
        if (error != 0)
        {
            std::wcerr << L"Failed to open file Dailog. Error Code: " << error << std::endl;
        }
    }
}

std::string ConvertWideStringToNarrowString(const std::wstring &widestring)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
    std::vector<char> buffer(bufferSize);
    WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, buffer.data(), bufferSize, NULL, NULL);
    return std::string(buffer.data());
}

bool EncryptData(const std::vector<BYTE> &data, std::vector<BYTE> &encryptedData)
{
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTHASH hHash = 0;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        std::cerr << "CryptAcquiredContext Failed." << std::endl;
        return false;
    }

    if (!CryptCreateHash(hProv, CALG_SHA, 0, 0, &hHash))
    {
        std::cerr << "CryptCreateHash failed. " << std::endl;
        CryptReleaseContext(hProv, 0);
        return false;
    }

    // Using PASSWORD to hash data

    const BYTE *password = reinterpret_cast<const BYTE *>("password");
    DWORD passwordLen = strlen("password");

    // if(!CryptHashData(hHash, password, passwordLen,0)){}

    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey))
    {
        std::cerr << "CryptDeriveKey failed. " << std::endl;
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    encryptedData = data;
    DWORD dataLen = encryptedData.size();

    if (!CryptEncrypt(hKey, NULL, TRUE, 0, encryptedData.data(), &dataLen, encryptedData.size()))
    {
        std::cerr << "CryptEncrypt failed. " << std::endl;
        CryptDestroyKey(hKey);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
}

void ReadFileToVector(const std::string &filename, std::vector<BYTE> &buffer)
{
    std::ifstream file(filename, std::ios::binary);
    buffer = std::vector<BYTE>(std::istreambuf_iterator<char>(file), {});
}

void WriteVectorToFile(const std::string &filename, std::vector<BYTE> &buffer)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}