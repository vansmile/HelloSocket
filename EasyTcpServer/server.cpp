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
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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
		

		//������
		char szRecv[1024] = {};

		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("�ͻ����˳�,���������\n");
			break;
		}
		//if(nLen >= header->datalength) //�ֲ��ж�


		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			recv(_cSock,szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("�յ�����[%d]  ���ݳ���[%d]\n", login->cmd, login->datalength);
			printf("username[%s] password[%s]\n", login->username, login->password);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
			break;
		case CMD_LOGOUT: {
			recv(_cSock, szRecv + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
			Logout* logout = (Logout*)szRecv;
			printf("�յ�����[%d]  ���ݳ���[%d]\n", logout->cmd, logout->datalength);
			printf("username[%s]\n", logout->username);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LogoutResult ret;
			send(_cSock, (char *)&ret, sizeof(LogoutResult), 0);
		}
			break;
		default: {
			DataHeader header = {0,CMD_ERROR};
	/*		header.cmd = CMD_ERROR;
			header.datalength = 0;*/
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