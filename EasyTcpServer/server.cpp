#include "EasyTcpServer.hpp"
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
	EasyTcpServer server;
	server.Bind(nullptr, 4567);
	server.Listen(5);
	//char _recvBuf[128] = {};


	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun) {
		server.OnRun();
	}
	server.Close();
	printf("���˳����������\n");
	getchar();

	return 0;
}