 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}
/* 返回缓冲区内可读字节数 */
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
/*  返回缓冲区内可写字节数 */
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}
/* 返回可以用作预存储的字节数，即 readPos_，这通常用于回收空间或支持数据头插入  */
size_t Buffer::PrependableBytes() const {
    return readPos_;
}
/*  返回当前读位置的指针，用于直接访问读位置的内容。  */
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}
/*  移动 readPos_ 向前 len 个字节   */
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}
/* 移动 readPos_ 到指定的 end 指针位置  */
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}
/* 清空缓冲区内容，重置读写位置   */
void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}
/* 将缓冲区的所有数据作为字符串返回，然后清空缓冲区 */
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

/*返回当前写位置的指针*/
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}
/*返回当前写位置的指针*/
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}
/*将 writePos_ 向前移动 len 字节，标记写入了 len 字节的数据*/
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}
/*  确保缓冲区有足够的可写空间来容纳 len 字节   */
void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);   // 如果当前缓冲区的剩余可写空间不足，则调用 MakeSpace_(len) 扩展空间
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 
    通过设置 iov[0] 和 iov[1]，readv 会将数据按顺序填充到 iov 数组中指定的两个缓冲区，从而实现分散读
    */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);  
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {   // buffer_可写空间可以容纳读取的数据
        writePos_ += len;
    }
    else {       //   buffer_可写空间容纳不了读取的数据, 写进临时缓冲区buff内
        writePos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}
/*  将 Buffer 中的内容写入指定文件描述符 fd，并返回写入的字节数 */
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}
//   可写起始指针
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}
//   可读起始指针
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}


void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {  // 缓冲区的当前可写空间和预留空间不足以满足所需的 len 字节时
        buffer_.resize(writePos_ + len + 1);  //  对buffer_扩容
    } 
    else {  //  空余空间够写len字节
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());  // 将 readPos_ 到 writePos_ 之间的数据前移到缓冲区的起始位置
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}