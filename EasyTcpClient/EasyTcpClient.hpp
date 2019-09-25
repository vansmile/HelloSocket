#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
//Ԥ���봦����֤�ô���ֻ������һ��
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR			(-1)
#endif // _WIN32

#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}
	//����������
	virtual ~EasyTcpClient() {
		Close();
	}
	//��ʼ��socket
	void initSocket() {
		//����win sock 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		if (INVALID_SOCKET != _sock) {
			printf("<socket = %d>�ر�����...\n",_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock) {
			printf("���󣬽����׽���ʧ��...\n");
			}
			else {
			printf("�����׽��ֳɹ�...\n");
		}
	}
	//���ӷ�����
	void Connect(const char *ip,unsigned short port) {
		//	2.���ӷ�����connect

		if (INVALID_SOCKET == _sock) {
			initSocket();
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.S_addr = inet_addr(ip);
#endif // _WIN32
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("<socket = %d>��������Sokcetʧ��\n",_sock);
		}
		else {
			printf("<socket = %d>���ӷ�����<%s:%d>�ɹ�...\n",_sock,ip,port);
		}
	}

	//�ر�socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
			//�ر�win sock 2.x����
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif // _WIN32
			_sock = INVALID_SOCKET;
		}

	}
	
	//��ѯ������Ϣ
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);    //����Ҫ��ѯ������
			timeval t = { 1,0 };
			int ret = select(_sock, &fdReads, NULL, NULL, &t);
			if (ret < 0) {
				printf("<socket = %d>Select�������1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket = %d>Select�������2\n", _sock);
					Close();

					return false;
				}

			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	
	//��������   //����ճ������ְ�
	int RecvData(SOCKET _cSock) {
		//������
		char szRecv[1024] = {};
		//5���շ���������
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {

			printf("<socket = %d>��������Ͽ�����,���������\n",_sock);
			return -1;
		}

		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		OnNetMsg(header);

		//if(nLen >= header->datalength) //�ֲ��ж�

	}

	//��Ӧ������Ϣ
	void OnNetMsg(DataHeader* header) {
	
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginret = (LoginResult*)header;
			printf("<socket = %d>�յ��������Ϣ��CMD_LOGIN_RESULT,���ݽ��[%d],���ݳ��ȡ�%d��\n", _sock,loginret->result, loginret->datalength);
		}
		break;
		case CMD_LOGOUT_RESULT: {		
			LogoutResult* logoutret = (LogoutResult*)header;
			printf("<socket = %d>�յ��������Ϣ��CMD_LOGOUT_RESULT,���ݽ��[%d],���ݳ��ȡ�%d��\n", _sock, logoutret->result, logoutret->datalength);
		}
								break;
		case CMD_NEW_USER_JOIN: {
			NewUserJoin* userjoin = (NewUserJoin*)header;
			printf("<socket = %d>�յ��������Ϣ��CMD_NEW_USER_JOIN,����ͻ���socket[%d],���ݳ��ȡ�%d��\n", _sock, userjoin->sock, userjoin->datalength);
		}
		default: {
		}
				 break;
		}
	}
	//��������
	int SendData(DataHeader *header) {
		if (isRun() && header) {
			return send(_sock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}


private:

};


#endif // !_E
