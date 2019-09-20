#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������   ���߹ر�SDL���
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#pragma comment(lib,"ws2_32.lib")

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
	//�ͻ���
	//	1.����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("���󣬽����׽���ʧ��...\n");
	}
	else {
		printf("�����׽��ֳɹ�...\n");
	}
	//	2.���ӷ�����connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("��������Sokcetʧ��\n");
	}
	else {
		printf("���ӳɹ�\n");
	}
	
	while (true) {
		//3.�û���������
	

		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
		//4.��������
		if (0 == strcmp(cmdBuf, "exit")) {
			printf("�յ��˳�����\n");

			break;
		}
		else if(0 == strcmp(cmdBuf, "login")){
			//5.��������
			Login login = {"fx","fx123456"};
			DataHeader dh = { sizeof(Login),CMD_LOGIN };
			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock,(const char *) &login, sizeof(login), 0);
			//���շ�������������
			DataHeader retHeader = {};
			LoginResult retLogin = {};
			recv(_sock, (char *)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char *)&retLogin, sizeof(retLogin), 0);
			printf("LoginResult :%d\n", retLogin.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = {"fx"};
			DataHeader dh = { sizeof(logout) ,CMD_LOGOUT};
			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock, (const char *)&logout, sizeof(logout), 0);

			DataHeader retHeader = {};
			LoginResult retLogout = {};
			recv(_sock, (char *)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char *)&retLogout, sizeof(retLogout), 0);
			printf("LogoutResult :%d\n", retLogout.result);
		}
		else {
			printf("��֧�ֵ�������������롣\n");
		}


	}
	

	

	//	4.�ر�socket close
	closesocket(_sock);

	///
	WSACleanup();
	printf("���˳����������\n");

	getchar();
	return 0;
}