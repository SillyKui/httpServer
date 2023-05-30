/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>


class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            //创建threadCount个子线程
            for(size_t i = 0; i < threadCount; i++) {
                
                std::thread([pool = pool_] {
                    std::unique_lock<std::mutex> locker(pool->mtx);  //创建一个锁，用来获取pool的锁
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);       //线程在此阻塞，等待条件变量的通知
                    }
                }).detach();//线程分离
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();       //通过条件变量唤醒所有的线程
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));        //将新任务加入到任务队列
        }
        pool_->cond.notify_one();       //通过条件变量去唤醒一个线程
    }

private:
    //结构体，池子
    struct Pool {
        std::mutex mtx;         //互斥锁
        std::condition_variable cond;   //条件变量
        bool isClosed;                  //线程池是否关闭
        std::queue<std::function<void()>> tasks;  //队列（保存主线程检测到的任务）
    };
    std::shared_ptr<Pool> pool_;        //创建池子的指针
};


#endif //THREADPOOL_H