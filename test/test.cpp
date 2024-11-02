
#include "../code/log/log.h"
#include "../code/pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    '''
    Single thread  
    '''
    int cnt = 0, level = 0;
    Log::Instance()->init(level, "./testlog1", ".log", 0);   // Synchronous
    for(level = 3; level >= 0; level--) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"PID:[%04d] %s 111111111 %d ============= ", gettid(), "Test", cnt++);  // when level <= i then output a log. 1W + 2w + 3w + 4w = 10w
            }
        }
    }
    cnt = 0;
    Log::Instance()->init(level, "./testlog2", ".log", 5000);   // asynchronous
    for(level = 0; level < 4; level++) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"PID:[%04d] %s 222222222 %d ============= ", gettid(), "Test", cnt++); // 4w + 3W + 2W + 1W = 10W
            }
        }
    }
}

void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 10000; j++ ){
        LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

void TestThreadPool() {
    '''
    multi thread
    '''
    Log::Instance()->init(0, "./testThreadpool", ".log", 5000);
    ThreadPool threadpool(6);
    for(int i = 0; i < 18; i++) {
        threadpool.AddTask(std::bind(ThreadLogTask, i % 4, i * 10000));  
    }
    getchar();
}

int main() {
    TestLog();     
    TestThreadPool();
}