#pragma once

// Network api based on system

// Win32
#define _WIN32_SOCKETS
#include <WinSock2.h>
typedef int socklen_t;		// Only win64 defines this?
#pragma comment( lib, "Ws2_32.lib" )

#define Net_SetLastError WSASetLastError
#define Net_GetLastError WSAGetLastError