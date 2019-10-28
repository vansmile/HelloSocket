
#include "EasyTcpClient.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread() {

	while (true) {
		char cmdBuf[256] = {};

		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			printf("�˳�CmdThread�߳�..\n");
			g_bRun = false;
			return;
		}
		else {
			printf("��֧�ֵ����\n");
		}
		
	}
	
}


int main() {
	const int cCount = 1000;
	EasyTcpClient* client[cCount];
	//ջ�ڴ�
	//EasyTcpClient client;
	//client.initSocket();
	for (int i = 0; i < cCount; i++) {
		client[i] = new EasyTcpClient();
		if (!g_bRun) {
			return 0;
		}
		client[i]->Connect("127.0.0.1", 4567);
	}
	/*client.Connect("127.0.0.1", 4567);*/
	//client.Connect("118.24.240.141", 4567);



	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	Login login;
	strcpy(login.username, "fanxiao");
	strcpy(login.password, "fanxiaomima");
	while (g_bRun) {
		//
		//client.SendData(&login);
		for (int i = 0; i < cCount; i++) {
			
			client[i]->SendData(&login);
			client[i]->OnRun();
		}

		//printf("����ʱ�䴦������ҵ��..\n");
	}
	for (int i = 0; i < cCount; i++) {
		client[i]->Close();
		delete client[i];
	}
	printf("���˳����������\n");

	getchar();
	return 0;
}