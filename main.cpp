#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

DWORD PID;

HANDLE GetHandle(const char* process) {
	HANDLE hPID = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 ProcEntry;
	ProcEntry.dwSize = sizeof(ProcEntry);
	do {
		if (!strcmp(ProcEntry.szExeFile, process)) {
			PID = ProcEntry.th32ProcessID;
			CloseHandle(hPID);
			return OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
		}
	} while (Process32Next(hPID, &ProcEntry));
	return 0;
}

DWORD GetModule(const char* module) {
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);
	while (Module32Next(hModule, &mEntry)) {
		if (!strcmp(mEntry.szModule, module)) {
			CloseHandle(hModule);
			return (DWORD)mEntry.modBaseAddr;
		}
	}
	return 0;
}

//grab fresh offsets/netvars here
//https://github.com/frk1/hazedumper/blob/master/csgo.hpp
#define dwEntityList 0x4D04A94
#define m_flDetectedByEnemySensorTime 0x3960
#define m_bSpotted 0x93D

int main() {
	HANDLE Vidya = GetHandle("csgo.exe");
	DWORD ClientDLL = GetModule("client_panorama.dll");
	while (true) {
		for (int i = 0; i <= 64; i++) {
			DWORD EntityList;
			const float glowtime = 134217722; //max
			const bool radarspotted = true;
			ReadProcessMemory(Vidya, (LPVOID)(ClientDLL + dwEntityList + i * 0x10), &EntityList, sizeof(EntityList), NULL);
			WriteProcessMemory(Vidya, (LPVOID)(EntityList + m_flDetectedByEnemySensorTime), &glowtime, sizeof(glowtime), NULL);
			WriteProcessMemory(Vidya, (LPVOID)(EntityList + m_bSpotted), &radarspotted, sizeof(radarspotted), NULL);
		}
		Sleep(100);
	}
}
