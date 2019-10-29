#include <iostream>
#include <thread>
#include <mutex>//��
#include <atomic> //ԭ��
#include "CELLTimeStamp.hpp"
using namespace std;
//ԭ�Ӳ���   ԭ��  ����
//�������������ʱ����С������λ
mutex m;
const int tCount = 4;
atomic_int sum = 0;
//��������  ʹ��ԭ�Ӳ���
//����    ��

void workFun(int index) {	
	for (int i = 0; i <20000000; i++) {
		//�Խ���---�������� unlock
		//�����lock unlock����ȫ
		//���迼������

		//lock_guard<mutex> lg(m);   
		//m.lock(); 
		//�ٽ�����-��ʼ
		sum++;
		//�ٽ�����-����
		//m.unlock();
	}
	//�̰߳�ȫ  �̲߳���ȫ
	//cout << "hello other thread <<<<" << index << endl;
		
}//��ռʽ

int main() {
	thread t[tCount];
	for (int n = 0; n < tCount; n++) {
		t[n] = thread(workFun, n);	
	}

	CELLTimeStamp tTime;
	for (int n = 0; n < tCount; n++) {
		t[n].join();
	}


	//t.detach();
	//t.join();
	cout <<tTime.getElapsedTimeInMilliSec()<<",sum = "<< sum<<endl;
	sum = 0;
	tTime.update();
	for (int i = 0; i < 80000000; i++)
	{	
		sum++;
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",sum = " << sum << endl;
	cout << "hello ,main thread " << endl;
	//getchar();
	return 0;
}