#include<thread>
#include<mutex>
#include<list>
#include<queue>
#include<functional>
class ThreadPool
{
	public:
		ThreadPool(const unsigned &corePoolSize,const unsigned maxPoolSize,const unsigned &keepAliveTime,const unsigned &maxQueueSize,const std::function<void(const std::function<void(void)>&)> &handler);
		~ThreadPool();
		bool execute(const std::function<void(void)> &work);
	private:
		const unsigned corePoolSize;
		const unsigned maxPoolSize;
		const unsigned keepAliveTime;
		const unsigned maxQueueSize;
		unsigned workCount;
		std::mutex workCountLock;
		const std::function<void(const std::function<void(void)>&)> handler;
		std::queue<std::function<void(void)> > workQueue;
		std::mutex workQueueLock;
		std::list<std::thread*> threadList;
		std::mutex threadListLock;
		bool shutdown;
};
