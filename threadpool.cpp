#include"threadpool.h"
#include<time.h>
ThreadPool::ThreadPool(const unsigned &corePoolSize,const unsigned maxPoolSize,const unsigned &keepAliveTime,const unsigned &maxQueueSize,const std::function<void(const std::function<void(void)>&)> &handler):
	corePoolSize(corePoolSize),
	maxPoolSize(maxPoolSize),
	keepAliveTime(keepAliveTime),
	maxQueueSize(maxQueueSize),
	workCount(0),
	handler(handler),
	shutdown(false)
{
}
ThreadPool::~ThreadPool()
{
	shutdown=true;
	for(std::list<std::thread*>::iterator i=threadList.begin();i!=threadList.end();i++)
	{
		(*i)->join();
		delete *i;
		*i=nullptr;
	}
}
bool ThreadPool::execute(const std::function<void(void)> &work)
{
	workCountLock.lock();
	if(workCount<corePoolSize)
	{
		threadListLock.lock();
		workCount++;
		threadList.push_back(new std::thread([](ThreadPool *const &threadPool,const std::function<void(void)> &firstWork)->void
		{
			firstWork();
			while(!threadPool->shutdown)
			{
				std::function<void(void)> work;
				bool flag=false;
				threadPool->workQueueLock.lock();
				if(!threadPool->workQueue.empty())
				{
					work=threadPool->workQueue.front();
					threadPool->workQueue.pop();
					flag=true;
				}
				threadPool->workQueueLock.unlock();
				if(flag)
					work();
			}
			threadPool->workCountLock.lock();
			threadPool->workCount--;
			threadPool->workCountLock.unlock();
		},this,work));
		threadListLock.unlock();
	}
	else if(workQueue.size()<maxQueueSize)
	{
		workQueueLock.lock();
		workQueue.push(work);
		workQueueLock.unlock();
	}
	else if(workCount<maxPoolSize)
	{
		threadListLock.lock();
		workCount++;
		threadList.push_back(new std::thread([](ThreadPool *const &threadPool,const std::function<void(void)> &firstWork)->void
		{
			int lastWorkTime;
			firstWork();
			lastWorkTime=clock();
			while((unsigned)(clock()-lastWorkTime)<=threadPool->keepAliveTime&&!threadPool->shutdown)
			{
				std::function<void(void)> work;
				bool flag=false;
				threadPool->workQueueLock.lock();
				if(!threadPool->workQueue.empty())
				{
					work=threadPool->workQueue.front();
					threadPool->workQueue.pop();
					flag=true;
				}
				threadPool->workQueueLock.unlock();
				if(flag)
				{
					work();
					lastWorkTime=clock();
				}
			}
			threadPool->workCountLock.lock();
			threadPool->workCount--;
			threadPool->workCountLock.unlock();
		},this,work));
		threadListLock.unlock();
	}
	else
	{
		handler(work);
		workCountLock.unlock();
		return 0;
	}
	workCountLock.unlock();
	return 1;
}
