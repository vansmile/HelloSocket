#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������   ���߹ر�SDL���
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#pragma comment(lib,"ws2_32.lib")

struct DataPackage {
	int age;
	char name[32];
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
		else {
			//5.��������
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		//	3.���շ�������Ϣrecv
		char recvBuf[256] = {};
		int nLen = recv(_sock, recvBuf, 256, 0);
		if (nLen > 0) {
			DataPackage* dp = (DataPackage *)recvBuf;
			printf("���ܵ����ݡ�%d��[%s]\n", dp->age,dp->name);
		}
		else {
			printf("��������ʧ��\n");

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