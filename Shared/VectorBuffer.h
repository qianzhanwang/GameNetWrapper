#ifndef VECTOR_BUFFER_H
#define VECTOR_BUFFER_H

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>
#include <functional>


/// chg by muduo::Buffer
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
/// todo:后续使用jemalloc替代vector做内存管理
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class VectorBuffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    VectorBuffer()
        : buffer_(kCheapPrepend + kInitialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend)
    {
        assert(readable_size() == 0);
        assert(writeable_size() == kInitialSize);
        assert(prependable_size() == kCheapPrepend);
    }

	VectorBuffer(size_t ini_size)
		: buffer_(kCheapPrepend + ini_size),
		readerIndex_(kCheapPrepend),
		writerIndex_(kCheapPrepend)
	{
		assert(ini_size > 0);
		assert(readable_size() == 0);
		assert(writeable_size() == ini_size);
		assert(prependable_size() == kCheapPrepend);
	}

    VectorBuffer(VectorBuffer&& tmp)
    {
        buffer_.swap(tmp.buffer_);
        std::swap(readerIndex_, tmp.readerIndex_);
        std::swap(writerIndex_, tmp.writerIndex_);
    }

    size_t readable_size() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writeable_size() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependable_size() const
    {
        return readerIndex_;
    }

    const char* data_readable() const
    {
        return begin() + readerIndex_;
    }

    char* data_writeable()
    {
        return begin() + writerIndex_;
    }

    void pop_front(size_t len)
    {
        assert(len <= readable_size());
        if (len < readable_size())
        {
            readerIndex_ += len;
        }
        else
        {
            clear();
        }
    }

    void pop_front_to(const char* end)
    {
        assert(data_readable() <= end);
        assert(end <= data_writeable());
        pop_front(end - data_readable());
    }

    void clear()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string to_string()
    {
        return to_string(readable_size());;
    }

    std::string to_string(size_t len)
    {
        assert(len <= readable_size());
        std::string result(data_readable(), len);
        pop_front(len);
        return result;
    }

    void append(const char* data, size_t len)
    {
        check_wirteable_size(len);
        ::memcpy(data_writeable(), data, len);
        writerIndex_ += len;
    }

    void append_outside(size_t len, std::function<void(char*, size_t)> cb_copy)
    {
        if (cb_copy)
        {
            check_wirteable_size(len);
            cb_copy(data_writeable(), len);
            writerIndex_ += len;
        }
    }

    void un_append(size_t len)
    {
        assert(len <= readable_size());
        writerIndex_ -= len;
    }

    void check_wirteable_size(size_t len)
    {
        if (writeable_size() < len)
        {
            alloc_buf(len);
        }
        assert(writeable_size() >= len);
    }

    void buf_shrink_to_fit()
    {
        buffer_.shrink_to_fit();
    }

    size_t buf_capacity() const
    {
        return buffer_.capacity();
    }

private:

    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void alloc_buf(size_t len)
    {
        if (writeable_size() + prependable_size() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readable_size();
//             std::copy(begin() + readerIndex_,
//                 begin() + writerIndex_,
//                 begin() + kCheapPrepend);

            strcpy_s(begin() + kCheapPrepend, readable, data_readable());
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readable_size());
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

#endif  // MUDUO_NET_BUFFER_H
