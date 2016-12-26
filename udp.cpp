// udp
// udp revice port [/c|/ca]
// udp send ipaddr:port msg

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int usage(int status) {
	if (!status) {
		printf("udp sockert tool for windows. Include sever and client.\n\n"
			"udp receive port [/c|/ca cmd] , udp send ipaddr:port msg\n\n"
			"  receive    run as a server to receive msg as text, cmd, or cmd argument\n"
			"    port    port to bind\n"
			"    [/c|/ca cmd]    msg as command, cmd argument (text if not specified)\n"
			"  send    run as a client to receive msg\n"
			"    ipaddr:port    server ip address and port\n"
			"    msg    msg to send\n\n"
			"By qinst64 <qinst64@gmail.com> https://github.com/qinst64/\n\n");
	}
	else {
		printf("Error usage.\n"
			"\tPlease type upd /? for more infomation.\n\n");
	}
	return status;
}

int main(int argc, char* argv[]) {
	// parse mode and from argv
	int error = 0;//syntax error
	int mode = -1;//0-server, 1-client
	int msgType = -1;//msg as: 0-text, 1-cmd, 2-cmd argument,
	if (argc < 2) {
		return usage(-1);
	}

	if (argc == 2 && !strcmp(argv[1], "/?") || !strcmp(argv[1], "-h")) {
		return usage(0);
	}

	if (!strcmp(argv[1], "receive")) {
		mode = 0;
		if (argc == 3) {
			msgType = 0;
		}
		else if (argc == 4 && !strcmp(argv[3], "/c")) {
			msgType = 1;
		}
		else if (argc == 5 && !strcmp(argv[3], "/ca")) {
			msgType = 5;
		}
		else {
			error = 1;
		}
	}
	else if (!strcmp(argv[1], "send")) {
		mode = 1;
		if (argc == 4) {

		}
		else {
			error = 1;
		}
	}
	else {
		error = 1;
	}

	if (error)return usage(-1);

	//parse ipaddr and port from argv
	char* port = NULL;
	unsigned long ipaddr = INADDR_ANY;
	if (mode) {
		ipaddr = inet_addr(strtok_s(argv[2], ":", &port));
	}
	else {
		port = argv[2];
	}


	//init socket
	WSADATA wsaData;
	WORD socketVersion = MAKEWORD(2, 2);
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		printf("socket error occured");
		return -1;
	}
	SOCKET msocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in msocktaddr;
	msocktaddr.sin_family = AF_INET;
	msocktaddr.sin_port = htons(atoi(port));
	msocktaddr.sin_addr.S_un.S_addr = ipaddr;

	// send or receive
	if (mode) {// client send msg
		sendto(msocket, argv[3], strlen(argv[3]), 0, (sockaddr *)&msocktaddr, sizeof(msocktaddr));

	}
	else {//server receive

		//bind port
		if (bind(msocket, (sockaddr *)&msocktaddr, sizeof(msocktaddr)) == SOCKET_ERROR) {
			printf("socket bind error !");
			closesocket(msocket);
			return -1;
		}

		//disp server IP
		printf("Server IP Address (port %s):\r\n", port);
		system("@echo off & for /f \"tokens=2 delims=:\" %i in ('ipconfig ^| find /i \"IPv4\"') do (echo %i) ");
		printf("----------------\r\n");

		//init
		time_t now;
		struct tm tmTmp;
		char buffer[10];
		sockaddr_in msocktaddr_remote;
		int maddrlen_remote = sizeof(msocktaddr_remote);
		int num = 0;
		char recvdata[255];
		int ret;
		string cmd, arg;//for /ca

		while (1)
		{
			ret = recvfrom(msocket, recvdata, 255, 0, (sockaddr *)&msocktaddr_remote, &maddrlen_remote);
			if (ret > 0)
			{
				time(&now);
				localtime_s(&tmTmp, &now);
				strftime(buffer, 10, "%H:%M:%S", &tmTmp);

				num++;
				recvdata[ret] = 0x00;
				printf("[%03d] %s\t%s\n", num, buffer, recvdata);

				//
				switch (msgType) {
				case 1:
					system(recvdata);
					break;
				case 2:
					cmd = argv[1];
					arg = recvdata;
					cmd = cmd + " " + arg;
					system(cmd.c_str());
					break;
				}
				//send back
				char * sendData = "Received\n";
				sendto(msocket, sendData, strlen(sendData), 0, (sockaddr *)&msocktaddr_remote, maddrlen_remote);

			}

		}
		closesocket(msocket);
		WSACleanup();
		return 0;
	}
}