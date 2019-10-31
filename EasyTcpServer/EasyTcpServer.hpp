
#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
//Ԥ���봦����֤�ô���ֻ������һ��

#ifdef _WIN32
#define FD_SETSIZE  2506
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
#include <thread>
#include <mutex>
#include <atomic>
#include "MessageHeader.hpp"
#include "CELLTimeStamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif // !RECV_BUFF_SIZE

#define _CellServer_THREAD_COUNT 4 





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
	SOCKET _sockfd;    //socket fd_set   �ļ�������  file desc set
    //�ڶ�������  ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��Ϣ������������β��λ��
	int _lastPos;
};

//new ���ڴ�
//����  ջ
//ջ�ռ�С  1M
//����ѡ��ClientSocket*

class InetEvent
{
public:
	//�ͻ����뿪�¼�
	virtual void OnLeave(ClientSocket* pClient) = 0;  //���麯��
	//virtual void OnNetMsg(ClientSocket* pClient) = 0;
private:

};
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET) {
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
		_pNetEvent = nullptr;
	}
	~CellServer() {
		Close();
		_sock = INVALID_SOCKET;
	}
	//�ر�Socket
	void Close() {
		if (_sock == INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			//	8.�ر�sokcet  close
			closesocket(_sock);
			///
			WSACleanup();
#else
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				close(_clients[n]->sockfd);
				delete _clients[n];

			}
			//	8.�ر�sokcet  close
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//����������Ϣ
	bool OnRun() {
		while (isRun()) {
			if (_clientsBuff.size() > 0)
			{//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}


			//���û����Ҫ����Ŀͻ��˾�����
			if (_clients.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
				
			//�������׽��� BDS ������
			fd_set fdRead;     //������(socket)����
			FD_ZERO(&fdRead);   //���㼯��
	
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				FD_SET(_clients[n]->sockfd(), &fdRead);

			}
			//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1,��Windows�������������д0
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, nullptr,nullptr, &t);



			//NULL������ �����޿ͻ���  �򲻻�����ִ��----
			//������������ͻ���ͨ�ţ�����TIMEOUT
			if (ret < 0) {
				printf("Select���������\n");
				Close();
				return false;
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							if(_pNetEvent)
								_pNetEvent->OnLeave(_clients[n]);
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}
	//�Ƿ�����
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}


	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) {
		_recvCount++;
		/*auto t1 = _tTime.getElaspsedSecond();
		if (t1 >= 1.0) {
			printf("time<%lf>,<socket = %d> ,<clients = %d> ,_recvCount<%d>\n", t1, _sock, _clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();

		}*/
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("�յ��ͻ���<socket = %d>����CMD_LOGIN,���ݳ��ȡ�%d��username[%s] password[%s]\n", cSock, login->datalength,login->username, login->password);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			/*LoginResult ret;
			SendData(cSock, &ret);*/

		}
		break;
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<socket = %d>����CMD_LOGOUT,���ݳ��ȡ�%d��username[%s] \n", cSock, logout->datalength, logout->username);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			/*LogoutResult ret;
			SendData(cSock, &ret);*/
		}
						 break;
		default: {
			printf("�յ��ͻ���<socket = %d>δ��������\n", cSock);
			//DataHeader header;
			///*		header.cmd = CMD_ERROR;
			//header.datalength = 0;*/
			//SendData(cSock, &header);

		}
				 break;
		}
		//	6.��������
	}

	//��������
	char _szRecv[RECV_BUFF_SIZE] = {};
	int RecvData(ClientSocket* pClient) {
		//������
		//5���տͻ�������
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		/*printf("nLen = %d\n", nLen);*/
		//DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("�ͻ���<Socket = %d>�˳�,���������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);

		///��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataheader����


		//����ٰ���ճ������
		//
		while (pClient->getLastPos() >= sizeof(DataHeader)) {
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= header->datalength) {
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - header->datalength;
				//����������Ϣ

				//������������Ϣ����
				//���߳�    �첽����
				OnNetMsg(pClient->sockfd(), header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
				///��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else {
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}



		//OnNetMsg(_cSock, header);
		return 0;
	}

	void addClient(ClientSocket* pClient) {
		/*_mutex.lock();*/
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start() {
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	//�ṩ�ͻ��������ķ���
	size_t getClientCount() {
		//���еĿͻ������ͻ������Ŀͻ�����
		return _clients.size() + _clientsBuff.size();
	}


	void setEventObj(InetEvent* pEvent) {
		_pNetEvent = pEvent;
	}

private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::vector<ClientSocket*>_clients;
	//����ͻ�����
	std::vector<ClientSocket*>_clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	InetEvent* _pNetEvent;
public:
	std::atomic_int _recvCount;
};


class EasyTcpServer:public InetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*>_clients;
	std::vector<CellServer*>_cellServers;
	CELLTimeStamp _tTime;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//��ʼ��socket
	void InitSocket() {
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

#endif // _WIN32
		//��Sokcet API��������TCP�����
		//	1.����һ��socket
		if (INVALID_SOCKET != _sock) {
			printf("<socket = %d>�ر�����...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("���󣬽����׽���Socket =<%d>ʧ��...\n",_sock);
		}
		else {
			printf("�����׽���Socket =<%d>�ɹ�...\n",_sock);
		}

		
		
	}
	//��ip�Ͷ˿�
	int Bind(const char *ip,unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		// 2.�󶨽��ܿͻ������ӵĶ˿�bind
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
			printf("���󣬰�����˿�<%d>ʧ��...\n",port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}

	//�����˿�
	int Listen(int n) {
		//	3.��������˿�listen
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket =<%d>���󣬼�������˿�ʧ��...\n",_sock);
		}
		else {
			printf("Socket =<%d>��������˿ڳɹ�...\n",_sock);

		}
		return ret;
	}

	//���������
	void Start() {
		for (int n = 0; n < _CellServer_THREAD_COUNT; n++) {
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	}


	//���տͻ�������
	SOCKET Accept() {
		//	4.�ȴ����ܿͻ������� accept
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET	cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif
		if (INVALID_SOCKET == cSock) {
			printf("Socket =<%d>���󣬽��ܵ���Ч�ͻ���SOCKET...\n",_sock);
		}
		else {
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			addClientToCellServer(new ClientSocket(cSock));
			//printf("Socket =<%d>�¿ͻ���<%d>���룺socket =  %d, IP:%s\n", _sock,_clients.size(),(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}
	void addClientToCellServer(ClientSocket* pClient) {
		_clients.push_back(pClient);
		//���ҿͻ��������ٵ�CellServer
		auto  pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) {
			if (pMinServer->getClientCount() > pCellServer->getClientCount()) {
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);

	}

	//�ر�Socket
	void Close() {
		if (_sock == INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			//	8.�ر�sokcet  close
			closesocket(_sock);
			///
			WSACleanup();
#else
			for (int n = _clients.size() - 1; n >= 0; n--) {
				//���ڿɶ���ѯ�п��������µ�������Ҫ����
				close(_clients[n]->sockfd);
				delete _clients[n];

		}
			//	8.�ر�sokcet  close
			close(_sock);
#endif
			_clients.clear();
		}
	}



	//int _nCount = 0;
	//����������Ϣ
	bool OnRun() {
		if (isRun()) {
			time4msg();
			//�������׽��� BDS ������
			fd_set fdRead;     //������(socket)����
			//fd_set fdWrite;
			//fd_set fdExcept;
			FD_ZERO(&fdRead);   //���㼯��
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExcept);

			FD_SET(_sock, &fdRead);   //��������(socket)���뼯����
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExcept);

			//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1,��Windows�������������д0

			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			//NULL������ �����޿ͻ���  �򲻻�����ִ��----
			//������������ͻ���ͨ�ţ�����TIMEOUT
			if (ret < 0) {
				printf("Select���������\n");
				Close();
				return false;
			}

			//�ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
				//��������

				return true;
			}
			return true;
		}
		return false;

	}


	


	//��Ӧ������Ϣ
	void time4msg() {
		auto t1 = _tTime.getElaspsedSecond();
		if (t1 >= 1.0) {
			int recvCount = 0;
			for (auto ser : _cellServers) {
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}

			printf("thread<%d>,time<%lf>,<socket = %d> ,<clients = %d> ,recvCount<%d>\n",_cellServers.size(), t1, _sock, _clients.size(), (int)(recvCount / t1));
			_tTime.update();

		}
	}



	

	//�Ƿ�����
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	//����ָ��Socket����
	int SendData(SOCKET cSock, DataHeader *header) {
		if (isRun() && header) {
			return send(cSock, (const char *)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}

	//Ⱥ����Ϣ
	void SendDataToAll(DataHeader *header) {
		for (int n = _clients.size() - 1; n >= 0; n--) {
			//�������ͻ��˷�����Ϣ
			SendData(_clients[n]->sockfd(), header);
		}
	}

	virtual void OnLeave(ClientSocket* pClient) {
		//���ͻ����뿪ʱ����clients�в��Ҳ��Ƴ��ÿͻ���
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			if (_clients[n] == pClient) {
				auto iter = _clients.begin() + n;
				if (iter != _clients.end()) {
					_clients.erase(iter);
				}
			}
		}
	}

private:

};

#endif