#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������   ���߹ر�SDL���
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<thread>
#pragma comment(lib,"ws2_32.lib")

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
struct Login :public DataHeader {
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

struct  NewUserJoin :public DataHeader
{
	NewUserJoin() {
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
bool g_bRun = true;
int processor(SOCKET _cSock) {
	//������
	char szRecv[1024] = {};
	//5���շ���������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		
		printf("��������Ͽ�����,���������\n");
		return -1;
	}
	//if(nLen >= header->datalength) //�ֲ��ж�


	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		LoginResult* loginret = (LoginResult*)szRecv;
		printf("�յ��������Ϣ��CMD_LOGIN_RESULT,���ݽ��[%d],���ݳ��ȡ�%d��\n",loginret->result,loginret->datalength);
	}
	break;
	case CMD_LOGOUT_RESULT :{
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		LogoutResult* logoutret = (LogoutResult*)szRecv;
		printf("�յ��������Ϣ��CMD_LOGOUT_RESULT,���ݽ��[%d],���ݳ��ȡ�%d��\n", logoutret->result, logoutret->datalength);
	}
	break;
	case CMD_NEW_USER_JOIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		NewUserJoin* userjoin = (NewUserJoin*)szRecv;
		printf("�յ��������Ϣ��CMD_NEW_USER_JOIN,����ͻ���socket[%d],���ݳ��ȡ�%d��\n", userjoin->sock, userjoin->datalength);
	}
	default: {
	}
			 break;
	}
	//	6.��������

}


void cmdThread(SOCKET _sock) {

	while (true) {
		char cmdBuf[256] = {};

		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			printf("�˳�CmdThread�߳�..\n");
			return;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.username, "fanxiao");
			strcpy(login.password, "nihaoa");
			send(_sock, (const char *)&login, sizeof(login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.username, "fanxiao");
			send(_sock, (const char *)&logout, sizeof(logout), 0);
		}
	}
	
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
	
	std::thread t1(cmdThread,_sock);
	t1.detach();

	while (g_bRun) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);    //����Ҫ��ѯ������
		timeval t = {1,0 };
		int ret = select(_sock, &fdReads, NULL, NULL, &t);
		if (ret < 0) {
			printf("Select�������1\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock)) {
				printf("Select�������2\n");
				break;
			}
			
		}

	
		//printf("����ʱ�䴦������ҵ��..\n");
		

	}
	

	

	//	4.�ر�socket close
	closesocket(_sock);

	///
	WSACleanup();
	printf("���˳����������\n");

	getchar();
	return 0;
}