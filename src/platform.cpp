#include "platform.h"
#include <iostream>
#ifdef WIN32
#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#endif

using namespace konstructs;

int Platform::init_winsock() {
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }

    return 0;
#else
    return 0;
#endif
}