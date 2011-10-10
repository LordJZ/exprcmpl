#ifndef _BYETBUFFER_H
#define _BYETBUFFER_H

#include "util.h"

class ByteBuffer
{
public:
    inline ByteBuffer(uint8* data, int length)
        : m_alloc(false), m_data(data), m_length(length), m_pos(0)
    {
    }

    inline ByteBuffer(int length)
        : m_alloc(true), m_data(new uint8[length]), m_length(length), m_pos(0)
    {
    }

    inline int pos()
    {
        return m_pos;
    }

    inline bool ensureCapacity(int len)
    {
        return m_length - m_pos >= len;
    }

    inline bool append_8(int byte)
    {
        if (!ensureCapacity(1))
            return false;

        m_data[m_pos] = uint8(byte);
        ++m_pos;
        return true;
    }

    inline bool append_16(int twobytes)
    {
        if (!ensureCapacity(2))
            return false;

        *(uint16*)&m_data[m_pos] = uint16(twobytes);
        m_pos += 2;
        return true;
    }

    inline bool append_32(uint32 val)
    {
        if (!ensureCapacity(4))
            return false;

        *(uint32*)&m_data[m_pos] = uint32(val);
        m_pos += 4;
        return true;
    }

    inline ~ByteBuffer()
    {
        if (m_alloc)
            delete[] m_data;
    }

private:
    bool m_alloc;
    uint8* m_data;
    int m_length;
    int m_pos;
};

#endif
