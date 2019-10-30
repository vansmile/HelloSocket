
#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
//预编译处理，保证该代码只被包含一次

#ifdef _WIN32
#define FD_SETSIZE  4024
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
#include "CELLTimeStamp.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUFF_SIZE
class ClientSocket {
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}
	SOCKET sockfd() {
		return _sockfd;
	}
	char *msgBuf() {
		return _szMsgBuf;
	}
	int getLastPos() {
		return _lastPos;
	}

	void setLastPos(int pos) {
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;    //socket fd_set   文件描述符  file desc set
    //第二缓冲区  消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区的数据尾部位置
	int _lastPos;
};

//new 堆内存
//其他  栈
//栈空间小  1M
//所以选择ClientSocket*
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*>_clients;
	CELLTimeStamp _tTime;
	int _recvCount;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
		_recvCount = 0;
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
		SOCKET	cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif
		if (INVALID_SOCKET == cSock) {
			printf("Socket =<%d>错误，接受到无效客户端SOCKET...\n",_sock);
		}
		else {
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("Socket =<%d>新客户端<%d>加入：socket =  %d, IP:%s\n", _sock,_clients.size(),(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}
	//关闭Socket
	void Close() {
		if (_sock == INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//放在可读查询中看看有无新的数据需要接收
				closesocket(_clients[n]->sockfd());
				delete _clients[n];

			}
			//	8.关闭sokcet  close
			closesocket(_sock);
			///
			WSACleanup();
#else
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//放在可读查询中看看有无新的数据需要接收
				close(_clients[n]->sockfd);
				delete _clients[n];

		}
			//	8.关闭sokcet  close
			close(_sock);
#endif
			_clients.clear();
		}
	}

	int _nCount = 0;
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
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//放在可读查询中看看有无新的数据需要接收
				FD_SET(_clients[n]->sockfd(), &fdRead);

			}
			

			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
			//即是所有文件描述符最大值+1,再Windows中这个参数可以写0

			timeval t = { 1,0 };

			//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, NULL);
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);
			//printf("select ret = %d count = %d\n", ret, _nCount++);


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
				//如果有连接  先不处理数据

				return true;
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}

			/*for (int n = 0; n < fdRead.fd_count; n++) {
				if (-1 == RecvData(fdRead.fd_array[n])) {
					auto iter = find(_clients.begin(), _clients.end(), fdRead.fd_array[n]);
					if (iter != _clients.end()) {
						_clients.erase(iter);
					}
				}

			}*/
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
	char _szRecv[RECV_BUFF_SIZE] = {};

	int RecvData(ClientSocket* pClient) {
		//缓冲区
		//5接收客户端数据
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		/*printf("nLen = %d\n", nLen);*/
		//DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("客户端<Socket = %d>退出,任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取到的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() +pClient->getLastPos(), _szRecv, nLen);

		///消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的数据长度大于消息头Dataheader长度


		//解决少包与粘包问题
		//
		while (pClient->getLastPos() >= sizeof(DataHeader)) {
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->datalength) {
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->datalength;
				//处理网络消息

				//数据量过大，消息队列
				//多线程    异步发送
				OnNetMsg(pClient->sockfd(),header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
				///消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			}
			else {
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}


		
		//OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header) {
		_recvCount++;
		auto t1 = _tTime.getElaspsedSecond();
		if (t1 >= 1.0) {
			printf("time<%lf>,<socket = %d> ,<clients = %d> ,_recvCount<%d>\n", t1,_sock,_clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.update();

		}
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("收到客户端<socket = %d>请求：CMD_LOGIN,数据长度【%d】username[%s] password[%s]\n", cSock, login->datalength,login->username, login->password);
			//忽略判断用户名密码是否正确的过程
			/*LoginResult ret;
			SendData(cSock, &ret);*/
			
		}
		break;
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)header;
			//printf("收到客户端<socket = %d>请求：CMD_LOGOUT,数据长度【%d】username[%s] \n", cSock, logout->datalength, logout->username);
			//忽略判断用户名密码是否正确的过程
			/*LogoutResult ret;
			SendData(cSock, &ret);*/
		}
						 break;
		default: {
			printf("收到客户端<socket = %d>未定义请求\n", cSock);
			//DataHeader header;
			///*		header.cmd = CMD_ERROR;
			//header.datalength = 0;*/
			//SendData(cSock, &header);

		}
				 break;
		}
		//	6.处理请求
	}

	//发送指定Socket数据
	int SendData(SOCKET cSock, DataHeader *header) {
		if (isRun() && header) {
			return send(cSock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}

	//群发消息
	void SendDataToAll(DataHeader *header) {
		for (int n = _clients.size() - 1; n >= 0; n--) {
			//向其他客户端发送消息
			SendData(_clients[n]->sockfd(), header);
		}
	}

private:

};

#endif