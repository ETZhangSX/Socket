/* CountDownLatch.h
** ETZhangSX
**
*/

#pragma once
#include "Condition.h"
#include "MutexLock.h"

// 确保发给线程的任务开始后才返回

class CountDownLatch
{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();
	
private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};