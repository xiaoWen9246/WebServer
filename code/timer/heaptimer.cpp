 
#include "heaptimer.h"
/*
void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}  */

void HeapTimer::siftup_(size_t i) {    //  从下标i向上调整
    assert(i < heap_.size());   
    while (i > 0) {
        size_t j = (i - 1) / 2;
        if (heap_[j] < heap_[i]) {
            break;
        }
        SwapNode_(i, j);   // 如果父节点大于等于子节点,交换
        i = j;
    }
}

void HeapTimer::SwapNode_(size_t i, size_t j) {   // 交换i和j节点
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;   // 更新节点的下标索引
    ref_[heap_[j].id] = j;
} 


/*从指定的索引 index 开始，将节点向下调整到正确位置，使堆满足最小堆性质*/
bool HeapTimer::siftdown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;  // 如果右子节点存在且小于左子节点，j 设置为右子节点的索引
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if(ref_.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i);
    } 
    else {
        /* 已有结点：调整堆 */
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) {
            siftup_(i);
        }
    }
}

void HeapTimer::doWork(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

void HeapTimer::del_(size_t index) {
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int timeout) {
    /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    siftdown_(ref_[id], heap_.size());
}

void HeapTimer::tick() {
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}
/* 返回堆中最早到期的节点距离当前时间的剩余时间*/
int HeapTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}