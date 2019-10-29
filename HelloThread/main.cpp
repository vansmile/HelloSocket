#include <iostream>
#include <thread>
#include <mutex>//锁
#include <atomic> //原子
#include "CELLTimeStamp.hpp"
using namespace std;
//原子操作   原子  分子
//计算机处理命令时的最小操作单位
mutex m;
const int tCount = 4;
atomic_int sum = 0;
//基础数据  使用原子操作
//其他    锁

void workFun(int index) {	
	for (int i = 0; i <20000000; i++) {
		//自解锁---出作用域 unlock
		//相比于lock unlock更安全
		//无需考虑死锁

		//lock_guard<mutex> lg(m);   
		//m.lock(); 
		//临界区域-开始
		sum++;
		//临界区域-结束
		//m.unlock();
	}
	//线程安全  线程不安全
	//cout << "hello other thread <<<<" << index << endl;
		
}//抢占式

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