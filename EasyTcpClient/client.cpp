#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS  //或者直接定义再   属性- C/C++ -预处理器中   或者关闭SDL检查
//解决windows/winsock2的宏定义冲突
//1.将winsock2放在前面
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
	//启动windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//报错
	//LNK2019	无法解析的外部符号 __imp__WSAStartup@8，该符号在函数 _main 中被引用	
	//代表缺少动态链接库  
	//方法1.-----#pragma comment(lib,"ws2_32.lib")  不能跨平台
	//方法2.属性-链接器-输入-附加依赖项

	///
	//客户端
	//	1.建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("错误，建立套接字失败...\n");
	}
	else {
		printf("建立套接字成功...\n");
	}
	//	2.连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("错误，连接Sokcet失败\n");
	}
	else {
		printf("连接成功\n");
	}
	
	while (true) {
		//3.用户输入命令
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
		//4.处理请求
		if (0 == strcmp(cmdBuf, "exit")) {
			printf("收到退出命令\n");

			break;
		}
		else {
			//5.发送命令
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		//	3.接收服务器信息recv
		char recvBuf[256] = {};
		int nLen = recv(_sock, recvBuf, 256, 0);
		if (nLen > 0) {
			DataPackage* dp = (DataPackage *)recvBuf;
			printf("接受到数据【%d】[%s]\n", dp->age,dp->name);
		}
		else {
			printf("接受数据失败\n");

		}
	}
	

	

	//	4.关闭socket close
	closesocket(_sock);

	///
	WSACleanup();
	printf("已退出，任务结束\n");

	getchar();
	return 0;
}