#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include <vector>
//#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader {
	short datalength;//short 32767;
	short cmd;
};
//Datapackage
//�̳л���ֱ�ӽ�Dataheaderд���ڲ�
struct Login :public DataHeader{
	Login() {
		datalength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char username[32];
	char password[32];
};
struct LoginResult :public DataHeader {
	LoginResult() {
		datalength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};
struct Logout :public DataHeader {
	Logout() {
		datalength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char username[32];
};
struct LogoutResult :public DataHeader {
	LogoutResult() {
		datalength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;

	}
	int result;

};

struct  NewUserJoin:public DataHeader
{
	NewUserJoin() {
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
std::vector<SOCKET>g_clients;

int processor(SOCKET _cSock) {
	//������
	char szRecv[1024] = {};
	//5���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("�ͻ���<Socket = %d>�˳�,���������\n",_cSock);
		return -1;
	}
	//if(nLen >= header->datalength) //�ֲ��ж�


	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ��ͻ���<Socket = %d>    ��������[%d]  ���ݳ���[%d]\n",_cSock, login->cmd, login->datalength);
		printf("username[%s] password[%s]\n", login->username, login->password);
		//�����ж��û��������Ƿ���ȷ�Ĺ���
		LoginResult ret;
		send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("�յ��ͻ���<Socket = %d> ��������[%d]  ���ݳ���[%d]\n",  _cSock, logout->cmd, logout->datalength);
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


int main() {
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//����
	//LNK2019	�޷��������ⲿ���� __imp__WSAStartup@8���÷����ں��� _main �б�����	
	//����ȱ�ٶ�̬���ӿ�  
	//����1.-----#pragma comment(lib,"ws2_32.lib")  ���ܿ�ƽ̨
	//����2.����-������-����-����������

	///
	//��Sokcet API��������TCP�����
	//	1.����һ��socket
	SOCKET	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//	2.�󶨽��ܿͻ������ӵĶ˿�bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);   //host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("���󣬰�����˿�ʧ��...\n");
	}
	else {
		printf("������˿ڳɹ�...\n");

	}
	//	3.��������˿�listen
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("���󣬼�������˿�ʧ��...\n");
	}
	else {
		printf("��������˿ڳɹ�...\n");

	}
	
	//char _recvBuf[128] = {};
	while (true) {

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

		timeval t = {1,0};
				
		//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, NULL);
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

		//NULL������ �����޿ͻ���  �򲻻�����ִ��----
		//������������ͻ���ͨ�ţ�����TIMEOUT
		if (ret < 0) {
			printf("Select���������\n");
			break;
		}

		//�ж�������(socket)�Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			//	4.�ȴ����ܿͻ������� accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET	_cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("���󣬽��ܵ���Ч�ͻ���SOCKET...\n");
			}
			else {
				for (int n = g_clients.size() - 1; n >= 0; n--) {
					//�������ͻ��˷�����Ϣ
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}

				g_clients.push_back(_cSock);

				printf("�¿ͻ��˼��룺socket =  %d, IP:%s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			}
			
		}
		for (int n = 0; n < fdRead.fd_count ; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
		
		//printf("����ʱ�䴦������ҵ��..\n");
		
	}
	for (int n = g_clients.size() - 1; n >= 0; n--) {
		//���ڿɶ���ѯ�п��������µ�������Ҫ����
		closesocket(g_clients[n]);

	}

	//	8.�ر�sokcet  close
	closesocket(_sock);
	///
	WSACleanup();
	printf("���˳����������\n");
	getchar();

	return 0;
}