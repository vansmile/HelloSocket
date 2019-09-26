
#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
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
#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET>g_clients;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//��ʼ��socket
	void InitSocket() {
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

#endif // _WIN32
		//��Sokcet API��������TCP�����
		//	1.����һ��socket
		if (INVALID_SOCKET != _sock) {
			printf("<socket = %d>�ر�����...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("���󣬽����׽���Socket =<%d>ʧ��...\n",_sock);
		}
		else {
			printf("�����׽���Socket =<%d>�ɹ�...\n",_sock);
		}

		
		
	}
	//��ip�Ͷ˿�
	int Bind(const char *ip,unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		// 2.�󶨽��ܿͻ������ӵĶ˿�bind
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);   //host to net unsigned short
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);// ;
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
		}
#else
		if (ip) {
			_sin.sin_addr.S_addr = inet_addr(ip);// ;
	}
		else {
			_sin.sin_addr.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
		}
#endif // _WIN32
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("���󣬰�����˿�<%d>ʧ��...\n",port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}

	//�����˿�
	int Listen(int n) {
		//	3.��������˿�listen
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket =<%d>���󣬼�������˿�ʧ��...\n",_sock);
		}
		else {
			printf("Socket =<%d>��������˿ڳɹ�...\n",_sock);

		}
		return ret;
	}

	//���տͻ�������
	SOCKET Accept() {
		//	4.�ȴ����ܿͻ������� accept
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET	_cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif
		if (INVALID_SOCKET == _cSock) {
			printf("Socket =<%d>���󣬽��ܵ���Ч�ͻ���SOCKET...\n",_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);

			printf("Socket =<%d>�¿ͻ��˼��룺socket =  %d, IP:%s\n", _sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	//�ر�Socket
	void Close() {
		if (_sock == INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = g_clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				closesocket(g_clients[n]);

			}
			//	8.�ر�sokcet  close
			closesocket(_sock);
			///
			WSACleanup();
#else
			for (int n = g_clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				close(g_clients[n]);

		}
			//	8.�ر�sokcet  close
			close(_sock);
#endif
		}
	}


	//����������Ϣ
	bool OnRun() {
		if (isRun()) {
			//�������׽��� BDS ������
			fd_set fdRead;     //������(socket)����
			fd_set fdWrite;
			fd_set fdExcept;
			FD_ZERO(&fdRead);   //���㼯��
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			FD_SET(_sock, &fdRead);   //��������(socket)���뼯����
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExcept);
			for (int n = g_clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				FD_SET(g_clients[n], &fdRead);

			}

			//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1,��Windows�������������д0

			timeval t = { 1,0 };

			//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, NULL);
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

			//NULL������ �����޿ͻ���  �򲻻�����ִ��----
			//������������ͻ���ͨ�ţ�����TIMEOUT
			if (ret < 0) {
				printf("Select���������\n");
				Close();
				return false;
			}

			//�ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = 0; n < fdRead.fd_count; n++) {
				if (-1 == RecvData(fdRead.fd_array[n])) {
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end()) {
						g_clients.erase(iter);
					}
				}

			}
			return true;
			//printf("����ʱ�䴦������ҵ��..\n");
		}
		return false;
		
	}


	//�Ƿ�����
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//��������
	int RecvData(SOCKET _cSock) {
		//������
		char szRecv[1024] = {};
		//5���տͻ�������
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("�ͻ���<Socket = %d>�˳�,���������\n", _cSock);
			return -1;
		}
		//if(nLen >= header->datalength) //�ֲ��ж�
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);

		
		OnNetMsg(_cSock, header);
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("�յ��ͻ���<Socket = %d>    ��������[%d]  ���ݳ���[%d]\n", _cSock, header->cmd, header->datalength);
			printf("username[%s] password[%s]\n", login->username, login->password);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)header;
			printf("username[%s]\n", logout->username);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LogoutResult ret;
			send(_cSock, (char *)&ret, sizeof(LogoutResult), 0);
		}
						 break;
		default: {
			DataHeader header = { 0,CMD_ERROR };
			/*		header.cmd = CMD_ERROR;
			header.datalength = 0;*/
			send(_cSock, (char *)&header, sizeof(header), 0);

		}
				 break;
		}
		//	6.��������
	}

	//����ָ��Socket����
	int SendData(SOCKET _cSock, DataHeader *header) {
		if (isRun() && header) {
			return send(_cSock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}

	//Ⱥ����Ϣ
	void SendDataToAll(DataHeader *header) {
		for (int n = g_clients.size() - 1; n >= 0; n--) {
			//�������ͻ��˷�����Ϣ
			SendData(g_clients[n], header);
		}
	}

private:

};

#endif