#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS  //或者直接定义再   属性- C/C++ -预处理器中
//解决windows/winsock2的宏定义冲突
//1.将winsock2放在前面
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include <vector>
//#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader {
	short datalength;//short 32767;
	short cmd;
};
//Datapackage
//继承或者直接将Dataheader写在内部
struct Login :public DataHeader{
	Login() {
		datalength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char username[32];
	char password[32];
};
struct LoginResult :public DataHeader {
	LoginResult() {
		datalength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};
struct Logout :public DataHeader {
	Logout() {
		datalength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char username[32];
};
struct LogoutResult :public DataHeader {
	LogoutResult() {
		datalength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;

	}
	int result;

};

struct  NewUserJoin:public DataHeader
{
	NewUserJoin() {
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
std::vector<SOCKET>g_clients;

int processor(SOCKET _cSock) {
	//缓冲区
	char szRecv[1024] = {};
	//5接收客户端数据
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("客户端<Socket = %d>退出,任务结束。\n",_cSock);
		return -1;
	}
	//if(nLen >= header->datalength) //现不判断


	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("收到客户端<Socket = %d>    请求命令[%d]  数据长度[%d]\n",_cSock, login->cmd, login->datalength);
		printf("username[%s] password[%s]\n", login->username, login->password);
		//忽略判断用户名密码是否正确的过程
		LoginResult ret;
		send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("收到客户端<Socket = %d> 请求命令[%d]  数据长度[%d]\n",  _cSock, logout->cmd, logout->datalength);
		printf("username[%s]\n", logout->username);
		//忽略判断用户名密码是否正确的过程
		LogoutResult ret;
		send(_cSock, (char *)&ret, sizeof(LogoutResult), 0);
	}
					 break;
	default: {
		DataHeader header = { 0,CMD_ERROR };
		/*		header.cmd = CMD_ERROR;
		header.datalength = 0;*/
		send(_cSock, (char *)&header, sizeof(header), 0);

	}
			 break;
	}
	//	6.处理请求

}


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
	//用Sokcet API建立建议TCP服务端
	//	1.建立一个socket
	SOCKET	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//	2.绑定接受客户端连接的端口bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);   //host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("错误，绑定网络端口失败...\n");
	}
	else {
		printf("绑定网络端口成功...\n");

	}
	//	3.监听网络端口listen
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("错误，监听网络端口失败...\n");
	}
	else {
		printf("监听网络端口成功...\n");

	}
	
	//char _recvBuf[128] = {};
	while (true) {

		//伯克利套接字 BDS 描述符
		fd_set fdRead;     //描述符(socket)集合
		fd_set fdWrite;
		fd_set fdExcept;
		FD_ZERO(&fdRead);   //清零集合
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		FD_SET(_sock, &fdRead);   //将描述符(socket)加入集合中
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExcept);
		for (int n = g_clients.size() - 1; n >= 0; n--) {
			//放在可读查询中看看有无新的数据需要接收
			FD_SET(g_clients[n], &fdRead);

		}
		
		//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
		//即是所有文件描述符最大值+1,再Windows中这个参数可以写0

		timeval t = {1,0};
				
		//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, NULL);
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

		//NULL阻塞的 ，若无客户端  则不会向下执行----
		//若还需主动向客户端通信，则定义TIMEOUT
		if (ret < 0) {
			printf("Select任务结束。\n");
			break;
		}

		//判断描述符(socket)是否在集合中
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			//	4.等待接受客户端连接 accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET	_cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("错误，接受到无效客户端SOCKET...\n");
			}
			else {
				for (int n = g_clients.size() - 1; n >= 0; n--) {
					//向其他客户端发送消息
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}

				g_clients.push_back(_cSock);

				printf("新客户端加入：socket =  %d, IP:%s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			}
			
		}
		for (int n = 0; n < fdRead.fd_count ; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
		
		//printf("空闲时间处理其他业务..\n");
		
	}
	for (int n = g_clients.size() - 1; n >= 0; n--) {
		//放在可读查询中看看有无新的数据需要接收
		closesocket(g_clients[n]);

	}

	//	8.关闭sokcet  close
	closesocket(_sock);
	///
	WSACleanup();
	printf("已退出，任务结束\n");
	getchar();

	return 0;
}