#pragma once
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

/*
Connect 		This function is used by the client to connect to an remote server
AcceptClient	This function is used by the server to accept clients
Listen			Start listening for incoming clients
Close			This will close the connection/server
recv			Receive the incoming packet, use ProcessMessage to process it first!
send			Send a message to the connected client, return: Bytes to read
*/

#define		BUFFERSIZE	1024

//is going to be hooked
typedef BOOL (__stdcall * WriteFile_t)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
typedef HANDLE (__stdcall * CreateFileA_t)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE (__stdcall * CreateFileW_t)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

class MemoryPipe
{
public:
	WriteFile_t WriteFile_o;
	CreateFileA_t CreateFileA_o;
	CreateFileW_t CreateFileW_o;

	MemoryPipe::MemoryPipe();
	MemoryPipe::MemoryPipe(char pipe[]);
	MemoryPipe::MemoryPipe(HANDLE hPipe);
	MemoryPipe::~MemoryPipe();

	bool Connect();
	BOOL Listen();
	BOOL AcceptClient();
	void Close();
	void Close(HANDLE handle);
	DWORD recv(char * szReturn);
	//bool send(LPTSTR buf);
	bool send(unsigned char buf[], const int Size);

	//public variables
	HANDLE hPipe;
	char pipe[100];
	bool connected;
	HANDLE hThread;
	HANDLE hHeap;
	TCHAR* pchRequest;
	TCHAR* pchReply;
	OVERLAPPED overlap; //recv and send at the same time
	
	//information about our client
	int pid;
	char processname[MAX_PATH];
	char path[MAX_PATH];
};