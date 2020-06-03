//
// Created by liampilot on 31/05/2020.
//

#ifndef QUEUEEXPERIMENTS_TCPQUEUE_H
#define QUEUEEXPERIMENTS_TCPQUEUE_H


#include "QueryBuffer.h"

class TCPQueue : public QueryBuffer {
public:
    long put(char* values, long bytes, long latencyMark) override;

    void free() override;

    void free(long offset) override;

    ByteBuffer& getBuffer() override;

    size_t getBufferCapacity(int id) override;

    long getLong(size_t index) override;

    void setLong(size_t index, long value) override;

    void appendBytesTo(int startPos, int endPos, ByteBuffer& outputBuffer) override;

    void appendBytesTo(int startPos, int endPos, char* output) override;
};


#endif //QUEUEEXPERIMENTS_TCPQUEUE_H
