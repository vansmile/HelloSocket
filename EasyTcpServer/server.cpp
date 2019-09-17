#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS  //����ֱ�Ӷ�����   ����- C/C++ -Ԥ��������
//���windows/winsock2�ĺ궨���ͻ
//1.��winsock2����ǰ��
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
//#pragma comment(lib,"ws2_32.lib")



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
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0) {
			printf("�ͻ����˳�,���������\n");
			break;
		}
		printf("���ܵ��ͻ�������[%s]\n", _recvBuf);
		//	6.��������
		if (0 == strcmp(_recvBuf, "getAge")) {
			char ageBuf[] = "24";
			//7.����
			send(_cSock, ageBuf, strlen(ageBuf) + 1, 0);    //strlen()+1 ���ͽ�β��
		}
		else if (0 == strcmp(_recvBuf, "getName")) {
			char nameBuf[] = "Fanxiao";
			send(_cSock, nameBuf, strlen(nameBuf) + 1, 0);    //strlen()+1 ���ͽ�β��
		}
		else {
			char msgBuf[] = "Hello,I'm Server.???What do you want to do?";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);    //strlen()+1 ���ͽ�β��
		}
		
		
	}


	//	8.�ر�sokcet  close
	closesocket(_sock);
	///
	WSACleanup();
	printf("���˳����������\n");
	getchar();

	return 0;
}