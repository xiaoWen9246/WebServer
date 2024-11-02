
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
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {                //   lambda expression
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);
                    }
                }).detach();   // 线程在后台独立运行
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;  // 移动构造函数
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {  // 检查指针 pool_ 是否指向有效对象
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {     // 通用引用:既可以匹配左值引用又可以匹配右值引用 (1)形如T&& (2)T的类型需要推导
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));    // 完美转发，保留task的类型信息，也保留它的值类别（即左值或右值）
        }
        pool_->cond.notify_one();      // wake up a waiting thread
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;  // 存储任何返回 void 且无参数的可调用对象
    };
    std::shared_ptr<Pool> pool_;   // Pool类型的智能指针
};


#endif //THREADPOOL_H