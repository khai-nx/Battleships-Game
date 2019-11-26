#pragma once
#include <winsock2.h>
void SendFile(SOCKET socket, const char* filePath);
void ReceiveFile(SOCKET socket, const char* filePath);
void PrintFile(const char* filePath);
void SendCord(SOCKET socket, int x, int y);
void ReceiveCord(SOCKET socket, int& x, int& y);