#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <winternl.h>
#include "MemoryPipe.h"
#include <detours.h>
#include <vector>
#include <Psapi.h>
#include <CorHdr.h>
#include "corinfo.h"
#include "corjit.h"
#include "DisasMSIL.h"
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "detours.lib")

using namespace std;
#define MAX_INSTR      100
#define IMPORT_TABLE_OFFSET 1
HINSTANCE hInstance;

enum API
{
	API_BitBlt = 0,
	API_MessageBoxA = 1,
	API_MessageBoxW = 2,
	API_SetWindowsHookEx = 3,
	API_NtWriteFile = 4,
	API_WriteProcessMemory = 5,
	DotNet_AssemblyLoad = 6,
	API_CreateFile = 7,
	API_CreateProcess = 8,
    DotNet_MethodCalls = 9,
};

enum API_Setting
{
	API_RememberAllow = 0,
	API_RememberBlock = 1,
	API_RememberUnknown = 2
};

void RemoveFilenameFromPath(char *pszPath)
{ 
	size_t len = strlen(pszPath);
	while(len && *(pszPath+len) != '\\') len--;
	if(len) *(pszPath+len+1) = '\0';
}

//FUNCTIONS FOR .NET
extern "C"
{
	PROCESS_INFORMATION pi;
	__declspec(dllexport) DWORD __stdcall StartProcess(char *szTargetFileExe)
	{
		STARTUPINFOA si;
		char TargetDirectory[MAX_PATH];
		strcpy(TargetDirectory, szTargetFileExe);
		RemoveFilenameFromPath(TargetDirectory);

		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
		si.wShowWindow = SW_SHOW;
		si.dwX = GetSystemMetrics(SM_CXSCREEN);
		si.dwY = GetSystemMetrics(SM_CYSCREEN);

		if(!CreateProcessA(szTargetFileExe, NULL, NULL, NULL, 0, CREATE_SUSPENDED, NULL, TargetDirectory, &si, &pi))
			return 0;
		return pi.dwProcessId;
	}

	__declspec(dllexport) DWORD __stdcall InjectFromDisk(char *szTargetFileDll)
	{
		LPVOID LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");
		LPVOID RemoteString = (LPVOID)VirtualAllocEx(pi.hProcess, NULL, strlen(szTargetFileDll), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(pi.hProcess, (LPVOID)RemoteString, szTargetFileDll, strlen(szTargetFileDll), NULL);
		HANDLE RemoteThread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
		if (WaitForSingleObject(RemoteThread, 90000) != WAIT_OBJECT_0)
		{
			VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
			CloseHandle(pi.hProcess);
			return 0;
		}
		if (RemoteThread == 0)
		{
			VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
			CloseHandle(pi.hProcess);
			return 0;
		}
		ResumeThread(pi.hThread);
		
		// Cleaning up
		VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return pi.dwProcessId;
	}

	__declspec(dllexport) bool __stdcall RuntimeInject(char *dllName, DWORD ProcessID)
	{
		HANDLE Proc;
		char buf[50]={0};
		LPVOID RemoteString, LoadLibAddy;
		Proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, ProcessID);
 
		if(!Proc)
			return false;
 
		LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
		RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(dllName), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(Proc, (LPVOID)RemoteString, dllName, strlen(dllName), NULL);
		CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
		CloseHandle(Proc);
		return true;
	}

	__declspec(dllexport) DWORD __stdcall CreateAndInjectFromDisk(char *szTargetFileExe, char *szTargetFileDll)
	{
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		char TargetDirectory[MAX_PATH];
		strcpy(TargetDirectory, szTargetFileExe);
		RemoveFilenameFromPath(TargetDirectory);

		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
		si.wShowWindow = SW_SHOW;
		si.dwX = GetSystemMetrics(SM_CXSCREEN);
		si.dwY = GetSystemMetrics(SM_CYSCREEN);

		if(!CreateProcessA(szTargetFileExe, NULL, NULL, NULL, 0, CREATE_SUSPENDED, NULL, TargetDirectory, &si, &pi))
			return 0;
	 
		LPVOID LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");
		LPVOID RemoteString = (LPVOID)VirtualAllocEx(pi.hProcess, NULL, strlen(szTargetFileDll), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(pi.hProcess, (LPVOID)RemoteString, szTargetFileDll, strlen(szTargetFileDll), NULL);
		HANDLE RemoteThread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
		if (WaitForSingleObject(RemoteThread, 90000) != WAIT_OBJECT_0)
		{
			VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
			CloseHandle(pi.hProcess);
			return 0;
		}
		if (RemoteThread == 0)
		{
			VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
			CloseHandle(pi.hProcess);
			return 0;
		}
		ResumeThread(pi.hThread);
		
		// Cleaning up
		VirtualFreeEx(pi.hProcess, RemoteString, 0, MEM_RELEASE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return pi.dwProcessId;
	}
}

typedef int (__stdcall * BitBlt_t)(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, unsigned long dwRop);
typedef int (__stdcall * MessageBoxA_t)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef int (__stdcall * MessageBoxW_t)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef HHOOK (__stdcall * SetWindowsHookEx_t)(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
typedef BOOL (__stdcall * WriteProcessMemory_t)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten);
//typedef NTSTATUS (__stdcall * NtWriteFile_t)(HANDLE FileHandle, HANDLE Event OPTIONAL, PIO_APC_ROUTINE ApcRoutine OPTIONAL, PVOID ApcContext OPTIONAL, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset OPTIONAL, PULONG Key OPTIONAL);
typedef BOOL (__stdcall * CreateProcessA_t)(LPCSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef BOOL (__stdcall * CreateProcessW_t)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

MessageBoxA_t MessageBoxA_o;
MessageBoxW_t MessageBoxW_o;
BitBlt_t BitBlt_o;
SetWindowsHookEx_t SetWindowsHookExA_o;
SetWindowsHookEx_t SetWindowsHookExW_o;
//NtWriteFile_t NtWriteFile_o;
WriteProcessMemory_t WriteProcessMemory_o;
CreateProcessA_t CreateProcessA_o;
CreateProcessW_t CreateProcessW_o;


PBYTE WriteFile_OrgAddress;
PBYTE CreateFileA_OrgAddress;
PBYTE CreateFileW_OrgAddress;

//API Settings
API_Setting Remember_MessageBoxA = API_RememberUnknown;
API_Setting Remember_MessageBoxW = API_RememberUnknown;
API_Setting Remember_BitBlt = API_RememberUnknown;
API_Setting Remember_SetWindowsHookExA = API_RememberUnknown;
API_Setting Remember_SetWindowsHookExW = API_RememberUnknown;
API_Setting Remember_WriteFile = API_RememberUnknown;
API_Setting Remember_WriteProcessMemory = API_RememberUnknown;
API_Setting Remember_CreateFileA = API_RememberUnknown;
API_Setting Remember_CreateFileW = API_RememberUnknown;
API_Setting Remember_CreateProcessA = API_RememberUnknown;
API_Setting Remember_CreateProcessW = API_RememberUnknown;
API_Setting Remember_JIT_AssemblyLoad = API_RememberUnknown;
API_Setting Remember_JIT_GraphicsCopyFromScreen = API_RememberUnknown;


char * PipeName;

class BitConverter
{
public:
	static int GetInt16(unsigned char * value, const int index)
	{
		short val = 0;
		memcpy_s(&val, 2, value+index, 2);
		return val;
	}
	static int GetInt32(unsigned char * value, const int index)
	{
		int val = 0;
		memcpy_s(&val, 4, value+index, 4);
		return val;
	}
	static int GetInt64(unsigned char * value, const int index)
	{
		long val = 0;
		memcpy_s(&val, 8, value+index, 8);
		return val;
	}

	static void GetBytes(short value, unsigned char * dest)
	{
		memcpy_s(dest, 2, &value, 2);
	}
	static void GetBytes(int value, unsigned char * dest)
	{
		memcpy_s(dest, 4, &value, 4);
	}
	static void GetBytes(long value, unsigned char * dest)
	{
		memcpy_s(dest, 8, &value, 8);
	}
};
typedef class 
{
public:
	DWORD index;
	int _size;
	unsigned char * _payload;
	char * LastString;
	DWORD LastStringSize;

	void Initialize(unsigned char * payload, const int size)
	{
		_payload = new unsigned char[size];
		memcpy_s(_payload, size, payload, size);
		index = 0;
		_size = size;
	}

	unsigned char ReadByte()
	{
		unsigned char ret = _payload[index];
		index++;
		return ret;
	}

	short ReadInt16()
	{
		short ret = BitConverter::GetInt16(_payload, index);
		index += 2;
		return ret;
	}

	int ReadInt32()
	{
		int ret = BitConverter::GetInt32(_payload, index);
		index += 4;
		return ret;
	}

	long ReadInt64()
	{
		long ret = BitConverter::GetInt64(_payload, index);
		index += 8;
		return ret;
	}
	
	void ReadString()
	{
		vector<unsigned char> stringy;
		int length = 0;
		for(DWORD i = index; i < _size; i += 2)
		{
			if(_payload[i] == 0 && _payload[i+1] == 0)
				break;

			int value = BitConverter::GetInt16(_payload, i);
			stringy.push_back((unsigned char)value);
			length++;
		}

		//Copy the string directly to the LastString
		if(length > 0)
		{
			LastString = (char*)malloc(length+1);
			memcpy_s(LastString, length, &stringy[0], length);
			LastString[length] = '\0';
			LastStringSize = length;
		}
		else
		{
			LastString = new char[0];
			LastStringSize = 0;
		}

		stringy.clear();
		index += (stringy.size() * 2) + 2; //length*2 + end bytes
	}
} PayloadReader;
typedef class
{
public:
	vector<unsigned char> payload;
	void WriteByte(unsigned char value)
	{
		//if(value == 255)
		//	MessageBoxA_o(NULL, "VALUE IS 255 BIATCH", "", NULL);

		payload.push_back(value);
	}

	void WriteBytes(unsigned char value[], const int size)
	{
		for(int i = 0; i < size; i++)
			WriteByte(value[i]);
	}

	void Clear()
	{
		payload.clear();
	}

	void WriteShort(short value)
	{
		unsigned char tmp[2];
		BitConverter::GetBytes(value, tmp);
		WriteBytes(tmp, 2);
	}
	void WriteInteger(int value)
	{
		unsigned char tmp[4];
		BitConverter::GetBytes(value, tmp);
		WriteBytes(tmp, 4);
	}
	void WriteLong(long value)
	{
		unsigned char tmp[8];
		BitConverter::GetBytes(value, tmp);
		WriteBytes(tmp, 8);
	}
	void WriteDouble(double value)
	{
		unsigned char tmp[8];
		BitConverter::GetBytes((long)value, tmp);
		WriteBytes(tmp, 8);
	}
	void WriteFloat(float value)
	{
		unsigned char tmp[4];
		BitConverter::GetBytes((int)value, tmp);
		WriteBytes(tmp, 4);
	}

	void WriteString(char value[])
	{
		for(WORD i = 0; i < strlen(value); i++)
		{
			unsigned char * tmp = new unsigned char[2];
			BitConverter::GetBytes((short)value[i], tmp);
			WriteByte(tmp[0]);
			WriteByte(tmp[1]);
			delete tmp;
		}
		WriteByte(0);
		WriteByte(0);
	}

	void WriteString(WCHAR value[])
	{
		int length = wcslen(value)+1;
		char * tmp = new char[length];
		wcstombs(tmp, value, wcslen(value)+1);
		WriteString(tmp);
		delete tmp;
	}

	void ToByteArray(unsigned char * dest)
	{
		memcpy_s(dest, payload.size(), &payload[0], payload.size());
	}
} PayloadWriter;

MemoryPipe * client;
bool ProcessingAPI = false;


bool SendPacket(PayloadWriter * writer)
{
	if(client != NULL)
	{
		unsigned char * tmp = new unsigned char[writer->payload.size()];
		writer->ToByteArray(tmp);

		bool ret = client->send(tmp, writer->payload.size());
		delete tmp;
		delete writer;
		return ret;
	}
	return false;
}

bool IsApiAllowed(API_Setting * setting)
{
	if(*setting == API_RememberAllow)
		return true;
	else if(*setting == API_RememberBlock)
		return false;

	char * result = new char[BUFFERSIZE];
	int read = client->recv(result);
	
	if(read > 0)
	{
		if(result[0] == 1)
		{
			if(result[1] == 1) //remember
				*setting = API_RememberAllow;

			SetLastError(0);
			ProcessingAPI = false;
			return true;
		}
		else
		{
			if(result[1] == 1) //remember
				*setting = API_RememberBlock;

			SetLastError(ERROR_ACCESS_DENIED);
			ProcessingAPI = false;
			return false;
		}
	}
	ProcessingAPI = false;
	return true;
}

int __stdcall MessageBoxA_Hooked(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	if(Remember_MessageBoxA == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_MessageBoxA);
		pw->WriteString((char*)lpText);
		pw->WriteString((char*)lpCaption);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_MessageBoxA))
		return MessageBoxA_o(hWnd, lpText, lpCaption, uType);
	return false;
}

int __stdcall MessageBoxW_Hooked(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	if(Remember_MessageBoxW == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_MessageBoxW);
		pw->WriteString((WCHAR*)lpText);
		pw->WriteString((WCHAR*)lpCaption);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_MessageBoxW))
		return MessageBoxW_o(hWnd, lpText, lpCaption, uType);
	return false;
}

int __stdcall BitBlt_Hooked(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, unsigned long dwRop)
{
	if(dwRop == SRCCOPY)
	{
		if(Remember_BitBlt == API_RememberUnknown)
		{
			while(ProcessingAPI)
				Sleep(10);
			ProcessingAPI = true;

			PayloadWriter * pw = new PayloadWriter();
			pw->WriteByte((unsigned char)API_BitBlt);
			SendPacket(pw);
		}
		if(IsApiAllowed(&Remember_BitBlt))
			return BitBlt_o(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
		return false;
	}
	return BitBlt_o(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
}

HHOOK __stdcall SetWindowsHookExA_Hooked(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
	if(Remember_SetWindowsHookExA == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_SetWindowsHookEx);
		pw->WriteInteger(idHook);
		pw->WriteInteger((int)hMod);
		pw->WriteInteger((int)dwThreadId);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_SetWindowsHookExA))
		return SetWindowsHookExA_o(idHook, lpfn, hMod, dwThreadId);
	return 0;
}

HHOOK __stdcall SetWindowsHookExW_Hooked(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
	if(Remember_SetWindowsHookExW == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_SetWindowsHookEx);
		pw->WriteInteger(idHook);
		pw->WriteInteger((int)hMod);
		pw->WriteInteger((int)dwThreadId);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_SetWindowsHookExW))
		return SetWindowsHookExW_o(idHook, lpfn, hMod, dwThreadId);
	return 0;
}

//need to hook WriteFile first
/*NTSTATUS __stdcall NtWriteFile_Hooked(HANDLE FileHandle, HANDLE Event OPTIONAL, PIO_APC_ROUTINE ApcRoutine OPTIONAL, PVOID ApcContext OPTIONAL, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset OPTIONAL, PULONG Key OPTIONAL)
{
	Beep(500, 500);
	while(ProcessingAPI)
		Sleep(10);
	ProcessingAPI = true;

	WCHAR * FileName = new WCHAR[MAX_PATH+1];
	if(Buffer != NULL && GetFileNameFromHandle(FileHandle, FileName) && client != NULL && Length > 0)
	{
		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_NtWriteFile);
		pw->WriteString(FileName);
		pw->WriteInteger(Length);
		pw->WriteBytes((unsigned char*)Buffer, Length);
		SendPacket(pw);

		if(IsApiAllowed())
			return NtWriteFile_o(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
		return 0x5;
	}
	return NtWriteFile_o(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
}*/

BOOL __stdcall WriteFile_Hooked(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	if(lpBuffer != NULL && nNumberOfBytesToWrite > 0)
	{
		if(Remember_WriteFile == API_RememberUnknown)
		{
			while(ProcessingAPI)
				Sleep(10);
			ProcessingAPI = true;

			PayloadWriter * pw = new PayloadWriter();
			pw->WriteByte((unsigned char)API_NtWriteFile);
			pw->WriteInteger((int)hFile);
			pw->WriteInteger(nNumberOfBytesToWrite);
			pw->WriteBytes((unsigned char*)lpBuffer, nNumberOfBytesToWrite);
			SendPacket(pw);
		}

		if(IsApiAllowed(&Remember_WriteFile))
			return client->WriteFile_o(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		return 0;
	}
	ProcessingAPI = false;
	return client->WriteFile_o(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL __stdcall WriteProcessMemory_Hooked(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten)
{
	if(lpBuffer != NULL && nSize > 0)
	{
		if(Remember_WriteProcessMemory == API_RememberUnknown)
		{
			while(ProcessingAPI)
				Sleep(10);
			ProcessingAPI = true;

			PayloadWriter * pw = new PayloadWriter();
			pw->WriteByte((unsigned char)API_WriteProcessMemory);
			pw->WriteInteger((int)hProcess);
			pw->WriteInteger(nSize);
			pw->WriteBytes((unsigned char*)lpBuffer, nSize);
			SendPacket(pw);
		}
		if(IsApiAllowed(&Remember_WriteProcessMemory))
			return WriteProcessMemory_o(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
		return 0;
	}
	ProcessingAPI = false;
	return WriteProcessMemory_o(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

HANDLE __stdcall CreateFileA_Hooked(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE FileHandle = client->CreateFileA_o(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if(FileHandle > 0)
	{
		if(Remember_CreateFileA == API_RememberUnknown)
		{
			while(ProcessingAPI)
				Sleep(10);
			ProcessingAPI = true;

			PayloadWriter * pw = new PayloadWriter();
			pw->WriteByte((unsigned char)API_CreateFile);
			pw->WriteString((char*)lpFileName);
			pw->WriteInteger((int)dwDesiredAccess);
			pw->WriteInteger((int)FileHandle);
			SendPacket(pw);
		}

		if(IsApiAllowed(&Remember_CreateFileA))
			return FileHandle;
		
		CloseHandle(FileHandle);
	}
	return INVALID_HANDLE_VALUE;
}

HANDLE __stdcall CreateFileW_Hooked(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE FileHandle = client->CreateFileW_o(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if(FileHandle > 0)
	{
		if(Remember_CreateFileW == API_RememberUnknown)
		{
			while(ProcessingAPI)
				Sleep(10);
			ProcessingAPI = true;

			PayloadWriter * pw = new PayloadWriter();
			pw->WriteByte((unsigned char)API_CreateFile);
			pw->WriteString((WCHAR*)lpFileName);
			pw->WriteInteger((int)dwDesiredAccess);
			pw->WriteInteger((int)FileHandle);
			SendPacket(pw);
		}

		if(IsApiAllowed(&Remember_CreateFileW))
			return FileHandle;
		CloseHandle(FileHandle);
	}
	return INVALID_HANDLE_VALUE;
}

BOOL __stdcall CreateProcessA_Hooked(LPCSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	if(Remember_CreateProcessA == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_CreateProcess);
		pw->WriteString((WCHAR*)lpApplicationName);
		pw->WriteString(lpCommandLine);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_CreateProcessA))
		return CreateProcessA_o(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	return 0;
}

BOOL __stdcall CreateProcessW_Hooked(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	if(Remember_CreateProcessW == API_RememberUnknown)
	{
		while(ProcessingAPI)
			Sleep(10);
		ProcessingAPI = true;

		PayloadWriter * pw = new PayloadWriter();
		pw->WriteByte((unsigned char)API_CreateProcess);
		pw->WriteString((WCHAR*)lpApplicationName);
		pw->WriteString(lpCommandLine);
		SendPacket(pw);
	}

	if(IsApiAllowed(&Remember_CreateProcessW))
		return CreateProcessW_o(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	return 0;
}

typedef bool (__stdcall * Beep_t)(DWORD freq, DWORD dur);
Beep_t Beep_o;

void HookJIT();

bool connected;
VOID WINAPI InitThread(LPVOID args)
{
	while(true)
	{
		if(client->Connect())
		{
			connected = true;
			while(client->connected)
				Sleep(100);
			ProcessingAPI = false;
		}
		connected = false;
		Sleep(1000);
	}
}

#pragma region JIT Hook
BOOL bHooked = FALSE;
ULONG_PTR *(__stdcall *p_getJit)();
typedef int (__stdcall *compileMethod_def)(ULONG_PTR classthis, ICorJitInfo *comp, CORINFO_METHOD_INFO *info, unsigned flags, BYTE **nativeEntry, ULONG  *nativeSizeOfCode);

struct JIT
{
	compileMethod_def compileMethod;
};

compileMethod_def compileMethod;
int __stdcall my_compileMethod(ULONG_PTR classthis, ICorJitInfo *comp, CORINFO_METHOD_INFO *info, unsigned flags, BYTE **nativeEntry, ULONG  *nativeSizeOfCode);
bool DisplayMethodAndCalls(ICorJitInfo *comp, CORINFO_METHOD_INFO *info);

void HookJIT()
{
	if (bHooked) return;

	int tries = 0;

	while(true)
	{
		LoadLibraryA("mscoree.dll");
		HMODULE hJitMod = LoadLibraryA("C:\\Windows\\Microsoft.NET\\Framework\\v2.0.50727\\mscorjit.dll"); //LoadLibraryA("mscorjit.dll");

		if (!hJitMod)
			continue;

		p_getJit = (ULONG_PTR *(__stdcall *)()) GetProcAddress(hJitMod, "getJit");
		if (p_getJit)
		{
			JIT *pJit = (JIT *) *((ULONG_PTR *) p_getJit());
			if (pJit)
			{
				DWORD OldProtect;
				VirtualProtect(pJit, sizeof (ULONG_PTR), PAGE_READWRITE, &OldProtect);
				compileMethod =  pJit->compileMethod;
				pJit->compileMethod = &my_compileMethod;
				VirtualProtect(pJit, sizeof (ULONG_PTR), OldProtect, &OldProtect);
				bHooked = TRUE;
			}
		}
		break;
		Sleep(1);
		tries++;

		if(tries >= 10000)
			return;
	}
}

int __stdcall my_compileMethod(ULONG_PTR classthis, ICorJitInfo *comp, CORINFO_METHOD_INFO *info, unsigned flags, BYTE **nativeEntry, ULONG  *nativeSizeOfCode)
{
// in case somebody hooks us (x86 only)
#ifdef _M_IX86
	__asm 
	{
		nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
	}
#endif
	int nRet = compileMethod(classthis, comp, info, flags, nativeEntry, nativeSizeOfCode);

	if(!DisplayMethodAndCalls(comp, info))
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return -1; //don't execute
	}
	return nRet;
}

bool DisplayMethodAndCalls(ICorJitInfo *comp, CORINFO_METHOD_INFO *info)
{
	const char *szMethodName = NULL;
	const char *szClassName = NULL;

	szMethodName = comp->getMethodName(info->ftn, &szClassName);
	ILOPCODE_STRUCT ilopar[MAX_INSTR];
	DISASMSIL_OFFSET CodeBase = 0;
	BYTE *pCur = info->ILCode;
	UINT nSize = info->ILCodeSize;
	UINT nDisasmedInstr;

	bool show = false;
	while (DisasMSIL(pCur, nSize, CodeBase, ilopar, MAX_INSTR, &nDisasmedInstr))
	{
		for (UINT x = 0; x < nDisasmedInstr; x++)
		{
			if (info->ILCode[ilopar[x].Offset] == ILOPCODE_CALL)
			{
				DWORD dwToken = *((DWORD *) &info->ILCode[ilopar[x].Offset + 1]);
				CORINFO_METHOD_HANDLE hCallHandle = comp->findMethod(info->scope, dwToken, info->ftn);
				szMethodName = comp->getMethodName(hCallHandle, &szClassName);

				if(strcmp(szClassName, "Assembly") == 0 && strcmp(szMethodName, "Load") == 0)
				{
					while(ProcessingAPI)
						Sleep(10);
					ProcessingAPI = true;

					PayloadWriter * pw = new PayloadWriter();
					pw->WriteByte((unsigned char)DotNet_AssemblyLoad);
					SendPacket(pw);

					if(!IsApiAllowed(&Remember_JIT_AssemblyLoad))
						return false;
				}

				PayloadWriter * writer = new PayloadWriter();
				writer->WriteByte((unsigned char)DotNet_MethodCalls);
				writer->WriteString((char*)szClassName);
				writer->WriteString((char*)szMethodName);
				SendPacket(writer);
			}
		}

		DISASMSIL_OFFSET next = ilopar[nDisasmedInstr - 1].Offset - CodeBase;
		next += ilopar[nDisasmedInstr - 1].Size;
		pCur += next;
		nSize -= next;
		CodeBase += next;
	}
	return true;
}
#pragma endregion

BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	HANDLE BitBltAddress		= GetProcAddress(GetModuleHandle(L"Gdi32.dll"), "BitBlt");
	HANDLE MessageBoxAAddress	= GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	HANDLE MessageBoxWAddress	= GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxW");
	HANDLE SetWindowsHookExAAddress	= GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowsHookExA");
	HANDLE SetWindowsHookExWAddress	= GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowsHookExW");
	HANDLE NtWriteFileAddress	= GetProcAddress(GetModuleHandle(L"Ntdll.dll"), "NtWriteFile");
	HANDLE WriteFileAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "WriteFile");
	HANDLE WriteProcessMemoryAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "WriteProcessMemory");
	HANDLE CreateFileAAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "CreateFileA");
	HANDLE CreateFileWAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "CreateFileW");
	HANDLE BeepAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "Beep");
	HANDLE CreateProcessAAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "CreateProcessA");
	HANDLE CreateProcessWAddress	= GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "CreateProcessW");
	Beep_o = (Beep_t)BeepAddress;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			hInstance = hinstDLL;
			PipeName = new char[255];
			sprintf_s(PipeName, 255, "RemoteProcessAccessTunnel%i\0", GetCurrentProcessId());

			HANDLE inject = GetPropA(GetDesktopWindow(), PipeName);
			if(inject == (HANDLE)1)
			{
				connected = false;
				//IAThooking(hinstDLL, "Beep", (PBYTE)newFunc);
				client = new MemoryPipe(PipeName);
				DWORD dwThreadId;
				if(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitThread, 0, 0, &dwThreadId) == 0)
					MessageBoxA(0, "Unable to create thread", "", MB_OK);

				Sleep(2000); //give the pipe sometime to connect, use mutex instead...

				//IAThooking(hInstance, "GetModuleHandleW", (PBYTE)newFunc); //test

				//hook the APIs
				DetourRemove((PBYTE)BitBltAddress, (PBYTE)BitBlt_Hooked);
				DetourRemove((PBYTE)MessageBoxAAddress, (PBYTE)MessageBoxA_Hooked);
				DetourRemove((PBYTE)MessageBoxWAddress, (PBYTE)MessageBoxW_Hooked);
				DetourRemove((PBYTE)SetWindowsHookExAAddress, (PBYTE)SetWindowsHookExA_Hooked);
				DetourRemove((PBYTE)SetWindowsHookExWAddress, (PBYTE)SetWindowsHookExW_Hooked);
				//DetourRemove((PBYTE)NtWriteFileAddress, (PBYTE)NtWriteFile_Hooked);
				DetourRemove((PBYTE)WriteFileAddress, (PBYTE)WriteFile_Hooked);
				DetourRemove((PBYTE)WriteProcessMemoryAddress, (PBYTE)WriteProcessMemory_Hooked);
				DetourRemove((PBYTE)CreateFileAAddress, (PBYTE)CreateFileA_Hooked);
				DetourRemove((PBYTE)CreateFileWAddress, (PBYTE)CreateFileW_Hooked);
				DetourRemove((PBYTE)CreateProcessAAddress, (PBYTE)CreateProcessA_Hooked);
				DetourRemove((PBYTE)CreateProcessWAddress, (PBYTE)CreateProcessW_Hooked);

				BitBlt_o = (BitBlt_t)DetourFunction((PBYTE)BitBltAddress, (PBYTE)BitBlt_Hooked);
				MessageBoxA_o = (MessageBoxA_t)DetourFunction((PBYTE)MessageBoxAAddress, (PBYTE)MessageBoxA_Hooked);
				MessageBoxW_o = (MessageBoxW_t)DetourFunction((PBYTE)MessageBoxWAddress, (PBYTE)MessageBoxW_Hooked);
				SetWindowsHookExA_o = (SetWindowsHookEx_t)DetourFunction((PBYTE)SetWindowsHookExAAddress, (PBYTE)SetWindowsHookExA_Hooked);
				SetWindowsHookExW_o = (SetWindowsHookEx_t)DetourFunction((PBYTE)SetWindowsHookExWAddress, (PBYTE)SetWindowsHookExW_Hooked);
				client->WriteFile_o = (WriteFile_t)DetourFunction((PBYTE)WriteFileAddress, (PBYTE)WriteFile_Hooked);
				//NtWriteFile_o = (NtWriteFile_t)DetourFunction((PBYTE)NtWriteFileAddress, (PBYTE)NtWriteFile_Hooked);
				WriteProcessMemory_o = (WriteProcessMemory_t)DetourFunction((PBYTE)WriteProcessMemoryAddress, (PBYTE)WriteProcessMemory_Hooked);
				client->CreateFileA_o = (CreateFileA_t)DetourFunction((PBYTE)CreateFileAAddress, (PBYTE)CreateFileA_Hooked);
				client->CreateFileW_o = (CreateFileW_t)DetourFunction((PBYTE)CreateFileWAddress, (PBYTE)CreateFileW_Hooked);
				CreateProcessA_o = (CreateProcessA_t)DetourFunction((PBYTE)CreateProcessAAddress, (PBYTE)CreateProcessA_Hooked);
				CreateProcessW_o = (CreateProcessW_t)DetourFunction((PBYTE)CreateProcessWAddress, (PBYTE)CreateProcessW_Hooked);

				HookJIT();
			}
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			if(MessageBoxAAddress > 0) //lazymode
			{
				DetourRemove((PBYTE)BitBltAddress, (PBYTE)BitBlt_Hooked);
				DetourRemove((PBYTE)MessageBoxAAddress, (PBYTE)MessageBoxA_Hooked);
				DetourRemove((PBYTE)MessageBoxWAddress, (PBYTE)MessageBoxW_Hooked);
				DetourRemove((PBYTE)SetWindowsHookExAAddress, (PBYTE)SetWindowsHookExA_Hooked);
				DetourRemove((PBYTE)SetWindowsHookExWAddress, (PBYTE)SetWindowsHookExW_Hooked);
				//DetourRemove((PBYTE)NtWriteFileAddress, (PBYTE)NtWriteFile_Hooked);
				DetourRemove((PBYTE)WriteFileAddress, (PBYTE)WriteFile_Hooked);
				DetourRemove((PBYTE)WriteProcessMemoryAddress, (PBYTE)WriteProcessMemory_Hooked);
				DetourRemove((PBYTE)CreateFileAAddress, (PBYTE)CreateFileA_Hooked);
				DetourRemove((PBYTE)CreateFileWAddress, (PBYTE)CreateFileW_Hooked);
				DetourRemove((PBYTE)CreateProcessAAddress, (PBYTE)CreateProcessA_Hooked);
				DetourRemove((PBYTE)CreateProcessWAddress, (PBYTE)CreateProcessW_Hooked);
			}
			break;
		}
	}
	return TRUE;
}