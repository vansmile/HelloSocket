#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS  //或者直接定义再   属性- C/C++ -预处理器中   或者关闭SDL检查
//解决windows/winsock2的宏定义冲突
//1.将winsock2放在前面
//2.#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};
struct DataHeader {
	short datalength;//short 32767;
	short cmd;
};
//Datapackage
//继承或者直接将Dataheader写在内部
struct Login :public DataHeader {
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
		else if(0 == strcmp(cmdBuf, "login")){
			//5.发送命令
			Login login;
			strcpy(login.username, "fanxiao");
			strcpy(login.password, "fanxiaomima");
			send(_sock,(const char *) &login, sizeof(login), 0);
			//接收服务器返回数据
			LoginResult retLogin = {};
			recv(_sock, (char *)&retLogin, sizeof(retLogin), 0);
			printf("LoginResult :%d\n", retLogin.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.username, "fanxiao");
			send(_sock, (const char *)&logout, sizeof(logout), 0);

			
			LoginResult retLogout = {};
			recv(_sock, (char *)&retLogout, sizeof(retLogout), 0);
			printf("LogoutResult :%d\n", retLogout.result);
		}
		else {
			printf("不支持的命令，请重新输入。\n");
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