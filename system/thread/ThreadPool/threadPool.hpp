#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unistd.h>
#include "lockGuard.hpp"
#include "thread.hpp"

const int g_thread_num = 3;


// 消费者模型
template<class T>
class ThreadPool
{
public:
    pthread_mutex_t* getMutex()
    {
        return &_lock;
    }

    bool isEmpty()
    {
        return _task_queue.empty();
    }

    void waitCond()
    {
        pthread_cond_wait(&_cond, )
    }

    T getTask()
    {
        T t = _task_queue.front();
        _task_queue.pop();
        return t;
    }

public:
    ThreadPool(int thread_num = g_thread_num)
        :_num(thread_num)
    {
        pthread_mutex_init(&_lock, nullptr);
        pthread_cond_init(&_cond, nullptr);
        
        for (int i = 1; i <= _num; i++)
        {
            // 创建线程
            _threads.push_back(new Thread(i, routine, this));
        }
    }

    void run()
    {
        for (auto &iter : _threads)
        {
            iter->start();
            std::cout << iter->name() << " 启动成功" << std::endl;
        }
    }

    // 线程池本质也是一个生产消费模型
    // 执行任务 (消费)
    static void* routine(void* args)   // static 不存在this指针，并且只能使用类内的静态方法
    {
        ThreadData* td = (ThreadData*)args;
        ThreadPool<T>* tp = (ThreadPool<T>*)td->_args;
        
        while (true)
        {
            T task;
            {
                lockGuard lockhguard(tp->getMutex());  // 创建对象自动加锁
                while (tp->isEmpty())
                {
                    tp->waitCond();  // 队列中没有任务就等待
                }
                // 读取任务
                task = tp->getTask();  // 任务队列是共享的->把任务从共享，拿到私有空间
            }
            // 处理任务
            task();
        }
    }

    // 插入任务 (生产)
    void pushTask(const T& task) 
    {
        lockGuard lockhguard(&_lock);  // 创建对象自动加锁
        _task_queue.push(task);
        pthread_cond_signal(&_cond);   // 唤醒线程逐个执行
    }

    // test func
    void joins()
    {
        for (auto &iter : _threads)
        {
            iter->join();  // 等待
        }
    }

    void show()
    {
        std::cout << "可以传入this,让使用静态方法的线程，访问线程池内的方法"  << std::endl;
    }
    ~ThreadPool()
    {
        for (auto &iter : _threads)
        {
            //iter->join();  // 主线程先回收资源
            delete iter;   // 再释放
        }

        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_cond);
    }
    
private:
    std::vector<Thread*> _threads;   // 储存线程
    int _num;                       // 线程数
    std::queue<T> _task_queue;     // 环形队列(任务池)
    pthread_mutex_t _lock;         // 锁 
    pthread_cond_t _cond;           // 条件变量
};