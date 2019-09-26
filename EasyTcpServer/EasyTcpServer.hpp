
#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
//预编译处理，保证该代码只被包含一次


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR			(-1)
#endif // _WIN32

#include <stdio.h>
#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET>g_clients;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//初始化socket
	void InitSocket() {
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

#endif // _WIN32
		//用Sokcet API建立建议TCP服务端
		//	1.建立一个socket
		if (INVALID_SOCKET != _sock) {
			printf("<socket = %d>关闭连接...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("错误，建立套接字Socket =<%d>失败...\n",_sock);
		}
		else {
			printf("建立套接字Socket =<%d>成功...\n",_sock);
		}

		
		
	}
	//绑定ip和端口
	int Bind(const char *ip,unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		// 2.绑定接受客户端连接的端口bind
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);   //host to net unsigned short
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);// ;
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
		}
#else
		if (ip) {
			_sin.sin_addr.S_addr = inet_addr(ip);// ;
	}
		else {
			_sin.sin_addr.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
		}
#endif // _WIN32
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("错误，绑定网络端口<%d>失败...\n",port);
		}
		else {
			printf("绑定网络端口<%d>成功...\n",port);
		}
		return ret;
	}

	//监听端口
	int Listen(int n) {
		//	3.监听网络端口listen
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket =<%d>错误，监听网络端口失败...\n",_sock);
		}
		else {
			printf("Socket =<%d>监听网络端口成功...\n",_sock);

		}
		return ret;
	}

	//接收客户端连接
	SOCKET Accept() {
		//	4.等待接受客户端连接 accept
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET	_cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif
		if (INVALID_SOCKET == _cSock) {
			printf("Socket =<%d>错误，接受到无效客户端SOCKET...\n",_sock);
		}
		else {
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);

			printf("Socket =<%d>新客户端加入：socket =  %d, IP:%s\n", _sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	//关闭Socket
	void Close() {
		if (_sock == INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = g_clients.size() - 1; n >= 0; n--) {
				//放在可读查询中看看有无新的数据需要接收
				closesocket(g_clients[n]);

			}
			//	8.关闭sokcet  close
			closesocket(_sock);
			///
			WSACleanup();
#else
			for (int n = g_clients.size() - 1; n >= 0; n--) {
				//放在可读查询中看看有无新的数据需要接收
				close(g_clients[n]);

		}
			//	8.关闭sokcet  close
			close(_sock);
#endif
		}
	}


	//处理网络消息
	bool OnRun() {
		if (isRun()) {
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

			timeval t = { 1,0 };

			//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, NULL);
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

			//NULL阻塞的 ，若无客户端  则不会向下执行----
			//若还需主动向客户端通信，则定义TIMEOUT
			if (ret < 0) {
				printf("Select任务结束。\n");
				Close();
				return false;
			}

			//判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = 0; n < fdRead.fd_count; n++) {
				if (-1 == RecvData(fdRead.fd_array[n])) {
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end()) {
						g_clients.erase(iter);
					}
				}

			}
			return true;
			//printf("空闲时间处理其他业务..\n");
		}
		return false;
		
	}


	//是否工作中
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//接收数据
	int RecvData(SOCKET _cSock) {
		//缓冲区
		char szRecv[1024] = {};
		//5接收客户端数据
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("客户端<Socket = %d>退出,任务结束。\n", _cSock);
			return -1;
		}
		//if(nLen >= header->datalength) //现不判断
		recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);

		
		OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("收到客户端<Socket = %d>    请求命令[%d]  数据长度[%d]\n", _cSock, header->cmd, header->datalength);
			printf("username[%s] password[%s]\n", login->username, login->password);
			//忽略判断用户名密码是否正确的过程
			LoginResult ret;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)header;
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

	//发送指定Socket数据
	int SendData(SOCKET _cSock, DataHeader *header) {
		if (isRun() && header) {
			return send(_cSock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}

	//群发消息
	void SendDataToAll(DataHeader *header) {
		for (int n = g_clients.size() - 1; n >= 0; n--) {
			//向其他客户端发送消息
			SendData(g_clients[n], header);
		}
	}

private:

};

#endif