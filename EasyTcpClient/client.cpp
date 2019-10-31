
#include "EasyTcpClient.hpp"
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

const int cCount = 10000;	//客户端数量
const int tCount = 4;  //线程数量
EasyTcpClient* client[cCount];

void sendThread(int id) {   //4个线程 1-4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;
	//栈内存
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

		//printf("空闲时间处理其他业务..\n");
	}
	for (int i = begin; i < end; i++) {
		client[i]->Close();
	}
}

int main() {
	

	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int i = 0; i < tCount; i++) {
		std::thread t1(sendThread,i+1);
		t1.detach();
	}

	while (g_bRun) {
		Sleep(100);
	}

	printf("已退出，任务结束\n");

	getchar();
	return 0;
}