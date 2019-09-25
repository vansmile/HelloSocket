
#include "EasyTcpClient.hpp"
#include <thread>

void cmdThread(EasyTcpClient* client) {

	while (true) {
		char cmdBuf[256] = {};

		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			
			client->Close();
			printf("�˳�CmdThread�߳�..\n");
			return;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.username, "fanxiao");
			strcpy(login.password, "nihaoa");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.username, "fanxiao");
			client->SendData(&logout);
		}
	}
	
}


int main() {

	EasyTcpClient client;
	//client.initSocket();
	client.Connect("127.0.0.1", 4567);


	//����UI�߳�
	std::thread t1(cmdThread,&client);
	t1.detach();
;
	while (client.isRun()) {
		client.OnRun();

		//printf("����ʱ�䴦������ҵ��..\n");
		

	}
	
	client.Close();
	printf("���˳����������\n");

	getchar();
	return 0;
}