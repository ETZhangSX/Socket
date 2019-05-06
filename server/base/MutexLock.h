/* MutexLock.h
** ETZhangSX
**
*/
#pragma once
#include <pthread.h>
#include <cstdio>

class MutexLock {
public:
	MutexLock() {
		pthread_mutex_init(&mutex, NULL);
	}

	~MutexLock() {
		pthread_mutex_lock(&mutex);
		pthread_mutex_destroy(&mutex);
	}
	
	void lock() {
		pthread_mutex_lock(&mutex);
	}

	void unlock() {
		pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_t *get() {
		return &mutex;
	}
private:
	pthread_mutex_t mutex;

	friend class Condition;
};

// 直接调用lock与unlock，无需创建新类
class MutexLockGuard {
	explicit MutexLockGuard(MutexLock &_mutex);
};