#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
//#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
struct DataHeader {
	short datalength;//short 32767;
	short cmd;
};
//Datapackage
struct Login {
	char username[32];
	char password[32];
};
struct LoginResult {
	int result;

};
struct Logout {
	char username[32];
};
struct LogoutResult {
	int result;

};

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
	//	4.�ȴ����ܿͻ������� accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET	_cSock = INVALID_SOCKET;
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("���󣬽��ܵ���Ч�ͻ���SOCKET...\n");
	}
	
	printf("�¿ͻ��˼��룺socket =  %d, IP:%s\n", (int)_cSock,inet_ntoa(clientAddr.sin_addr));
	char _recvBuf[128] = {};
	while (true) {
		//5���տͻ�������
		DataHeader header = {};

		int nLen = recv(_cSock, (char *)&header, sizeof(header), 0);
		if (nLen <= 0) {
			printf("�ͻ����˳�,���������\n");
			break;
		}
		printf("�յ�����[%d]  ���ݳ���[%d]\n", header.cmd,header.datalength);
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(_cSock, (char *)&login, sizeof(Login), 0);
			printf("username[%s] password[%s]", login.username, login.password);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginResult ret = {0};
			send(_cSock, (char *)&header, sizeof(DataHeader), 0);
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
			break;
		case CMD_LOGOUT: {
			Logout logout = {};
			recv(_cSock, (char *)&logout, sizeof(Logout), 0);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LogoutResult ret = { 1};
			send(_cSock, (char *)&header, sizeof(DataHeader), 0);
			send(_cSock, (char *)&ret, sizeof(LogoutResult), 0);
		}
			break;
		default: {
			header.cmd = CMD_ERROR;
			header.datalength = 0;
			send(_cSock, (char *)&header, sizeof(header), 0);

		}
			break;
		}
		//	6.��������
		
		
		
	}


	//	8.�ر�sokcet  close
	closesocket(_sock);
	///
	WSACleanup();
	printf("���˳����������\n");
	getchar();

	return 0;
}