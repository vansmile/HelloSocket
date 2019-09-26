#include "EasyTcpServer.hpp"

int main() {
	EasyTcpServer server;
	server.Bind(nullptr, 4567);
	server.Listen(5);
	//char _recvBuf[128] = {};
	while (server.isRun()) {
		server.OnRun();
	}
	server.Close();
	printf("已退出，任务结束\n");
	getchar();

	return 0;
}