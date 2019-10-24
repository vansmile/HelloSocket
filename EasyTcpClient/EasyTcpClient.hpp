#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
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
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}
	//虚析构函数
	virtual ~EasyTcpClient() {
		Close();
	}
	//初始化socket
	void InitSocket() {
		//启动win sock 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		if (INVALID_SOCKET != _sock) {
			printf("<socket = %d>关闭连接...\n",_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock) {
			printf("错误，建立套接字失败...\n");
			}
			else {
			printf("建立套接字成功...\n");
		}
	}
	//连接服务器
	void Connect(const char *ip,unsigned short port) {
		//	2.连接服务器connect

		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.S_addr = inet_addr(ip);
#endif // _WIN32
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("<socket = %d>错误，连接Sokcet失败\n",_sock);
		}
		else {
			printf("<socket = %d>连接服务器<%s:%d>成功...\n",_sock,ip,port);
		}
	}

	//关闭socket
	void Close() {
		if (_sock != INVALID_SOCKET) {
			//关闭win sock 2.x环境
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif // _WIN32
			_sock = INVALID_SOCKET;
		}

	}
	int _nCount = 0;
	//查询网络消息
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);    //加入要查询的数据
			timeval t = { 0,0 };
			int ret = select(_sock, &fdReads, NULL, NULL, &t);
			//printf("select ret = %d count = %d\n", ret, _nCount++);
			if (ret < 0) {
				//printf("<socket = %d>Select任务结束1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket = %d>Select任务结束2\n", _sock);
					Close();

					return false;
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	//缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};

	//第二缓冲区  消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//消息缓冲区的数据尾部位置
	int _lastPos = 0;

	//接收数据   //处理粘包，拆分包
	int RecvData(SOCKET cSock) {
		//缓冲区
	
		//5接收数据
		int nLen = recv(cSock, _szRecv, RECV_BUFF_SIZE, 0);
		//printf("nLen = %d\n", nLen);
		if (nLen <= 0) {

			printf("<socket = %d>与服务器断开链接,任务结束。\n", cSock);
			return -1;
		}

		//将收取到的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf + _lastPos, _szRecv,nLen);
		///消息缓冲区的数据尾部位置后移
		_lastPos += nLen;
		//判断消息缓冲区的数据长度大于消息头Dataheader长度
		

		//解决少包与粘包问题
		while (_lastPos >= sizeof(DataHeader)) {
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//判断消息缓冲区的数据长度大于消息长度
			if (_lastPos >= header->datalength) {
				//消息缓冲区剩余未处理数据的长度
				int nSize =_lastPos - header->datalength;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->datalength, nSize);
				///消息缓冲区的数据尾部位置前移
				_lastPos = nSize;
			}
			else {
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}

		//
		//recv(_cSock, szRecv + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
		//
		return 0;
		//if(nLen >= header->datalength) //现不判断

	}

	//响应网络消息
	virtual void OnNetMsg(DataHeader* header) {
	
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginret = (LoginResult*)header;
			printf("<socket = %d>收到服务端消息：CMD_LOGIN_RESULT,数据结果[%d],数据长度【%d】\n", _sock,loginret->result, loginret->datalength);
		}
		break;
		case CMD_LOGOUT_RESULT: {		
			LogoutResult* logoutret = (LogoutResult*)header;
			printf("<socket = %d>收到服务端消息：CMD_LOGOUT_RESULT,数据结果[%d],数据长度【%d】\n", _sock, logoutret->result, logoutret->datalength);
		}
								break;
		case CMD_NEW_USER_JOIN: {
			NewUserJoin* userjoin = (NewUserJoin*)header;
			printf("<socket = %d>收到服务端消息：CMD_NEW_USER_JOIN,加入客户端socket[%d],数据长度【%d】\n", _sock, userjoin->sock, userjoin->datalength);
		}
								break;
		case CMD_ERROR: {
			printf("<socket = %d>收到服务端消息：CMD_ERROR,加入客户端socket[%d],数据长度【%d】\n", _sock, header->datalength);
		}
						break;
		default: {
			printf("<socket = %d>收到未定义消息加入客户端socket[%d],数据长度【%d】\n", _sock, header->datalength);

		}
				 break;
		}
	}
	//发送数据
	int SendData(DataHeader *header) {
		if (isRun() && header) {
			return send(_sock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}


private:

};


#endif // !_E
