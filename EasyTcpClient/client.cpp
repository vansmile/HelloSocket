
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

const int cCount = 10000;	//�ͻ�������
const int tCount = 4;  //�߳�����
EasyTcpClient* client[cCount];

void sendThread(int id) {   //4���߳� 1-4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;
	//ջ�ڴ�
	//EasyTcpClient client;
	//client.initSocket();
	for (int i = begin; i < end; i++) {
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1", 4567);
		printf("Connect = %d\n", i);
	}
	/*client.Connect("127.0.0.1", 4567);*/
	//client.Connect("118.24.240.141", 4567);

	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);

	Login login;
	strcpy(login.username, "fanxiao");
	strcpy(login.password, "fanxiaomima");
	while (g_bRun) {
		//
		//client.SendData(&login);
		for (int i = begin; i < end; i++) {

			client[i]->SendData(&login);
			//client[i]->OnRun();
		}

		//printf("����ʱ�䴦������ҵ��..\n");
	}
	for (int i = begin; i < end; i++) {
		client[i]->Close();
	}
}

int main() {
	

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	//���������߳�
	for (int i = 0; i < tCount; i++) {
		std::thread t1(sendThread,i+1);
		t1.detach();
	}

	while (g_bRun) {
		Sleep(100);
	}

	printf("���˳����������\n");

	getchar();
	return 0;
}