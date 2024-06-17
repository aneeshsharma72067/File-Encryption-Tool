# File Encryption Tool Using Win32 API

This is a GUI based Tool using C++ and WinAPI to encrypt files

This makes use of the Windows native API (`windows.h`) and Windows CryptoAPI (`wincrypt.h`)

## Why I am using Win API ?

I chose to use the Windows API (WinAPI) for this encryption tool to gain a deeper understanding of Windows' underlying architecture. By working directly with WinAPI, I am able to engage with the native interfaces and mechanisms of the operating system, bypassing the abstractions and simplifications provided by higher-level frameworks. This approach allows me to learn about the core functionalities and low-level operations that drive Windows, thus enriching my knowledge and skills in system programming and Windows internals.

## Cryptography Implementation with Windows CryptoAPI

For the cryptographic functions in this tool, I am employing the Windows Cryptography API (CryptoAPI), which is accessible via the wincrypt.h header. The CryptoAPI provides a set of functions that allow for secure encryption and decryption of data, leveraging the robust and well-tested security infrastructure that is built into the Windows operating system.

## How to Use ??

- Execute the program by running `run.bat`
- Select location of file to be encrypted
- Enter a secret key
- Click Encrypt
- To Decrypt a file, select the file location and enter the key used to encrypt the file and click Decrypt
