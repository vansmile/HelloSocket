#include "EasyTcpServer.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread() {

	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			printf("退出CmdThread线程..\n");
			g_bRun = false;
			return;
		}
		else {
			printf("不支持的命令。\n");
		}
	}

}

int main() {
	EasyTcpServer server;
	server.Bind(nullptr, 4567);
	server.Listen(5);
	//char _recvBuf[128] = {};

	server.Start();
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun) {
		server.OnRun();
	}
	server.Close();
	printf("已退出，任务结束\n");
	getchar();

	return 0;
}