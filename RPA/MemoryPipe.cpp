#include "MemoryPipe.h"

MemoryPipe::MemoryPipe(char pipe[])
{
	strcpy_s(this->pipe, "\\\\.\\pipe\\");
	strcat_s(this->pipe, pipe);
	MemoryPipe::MemoryPipe();
}

MemoryPipe::MemoryPipe(HANDLE hPipe)
{
	this->hPipe = hPipe;
	MemoryPipe::MemoryPipe();
}
MemoryPipe::MemoryPipe()
{
	connected = true;
	hHeap = GetProcessHeap();
	pchReply = (wchar_t *)HeapAlloc(hHeap, 0, BUFFERSIZE*sizeof(wchar_t));
	pchRequest = (wchar_t *)HeapAlloc(hHeap, 0, BUFFERSIZE*sizeof(wchar_t));
}
MemoryPipe::~MemoryPipe()
{
	delete pchReply;
	delete pchRequest;
}

bool MemoryPipe::Connect()
{
	hPipe = CreateFileA_o(pipe, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

BOOL MemoryPipe::Listen()
{
	hPipe = CreateNamedPipeA(this->pipe,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		BUFFERSIZE,
		BUFFERSIZE,
		0,
		NULL);
	return (hPipe ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)); 
}

BOOL MemoryPipe::AcceptClient()
{
	return ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
}

void MemoryPipe::Close()
{
	//	FlushFileBuffers(hPipe); //it's not really needed and it could block futher code
	DisconnectNamedPipe(hPipe);
	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	//CloseHandle(hPipe);
	//CloseHandle(hReadPipe);
	//CloseHandle(hWritePipe);
	//CloseHandle(hThread);
	//CloseHandle(hHeap);
}
void MemoryPipe::Close(HANDLE handle)
{
	FlushFileBuffers(handle);
	DisconnectNamedPipe(handle);
	CloseHandle(handle);
}

DWORD MemoryPipe::recv(char * szReturn)
{
	DWORD cbBytesRead = 0;
	BOOL fSuccess = ReadFile(hPipe, pchRequest, 4, &cbBytesRead, NULL);

	if(fSuccess)
	{
		int BytesToRead = *(int*)pchRequest;
		fSuccess = ReadFile(hPipe, pchRequest, BytesToRead, &cbBytesRead, NULL);
	}

	size_t origsize = wcslen(pchRequest) + 1;
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, szReturn, origsize, pchRequest, _TRUNCATE);
	return cbBytesRead;
}
bool MemoryPipe::send(unsigned char buf[], const int Size)
{
	if(hPipe == INVALID_HANDLE_VALUE)
		return false;

	DWORD written = 0;
	FlushFileBuffers(hPipe);

	unsigned char * sizeBytes = new unsigned char[4];
	memcpy_s(sizeBytes, 4, &Size, 4);
	WriteFile_o(hPipe, sizeBytes, 4, &written, NULL);
	delete sizeBytes;
	return WriteFile_o(hPipe, buf, Size, &written, NULL) ? true : false;
}