//
// Created by liampilot on 31/05/2020.
//

#include "TCPQueue.h"

long TCPQueue::put(char* values, long bytes, long latencyMark) {
    return 0;
}

void TCPQueue::free() {

}

void TCPQueue::free(long offset) {

}

ByteBuffer& TCPQueue::getBuffer() {
    return <#initializer#>;
}

size_t TCPQueue::getBufferCapacity(int id) {
    return 0;
}

long TCPQueue::getLong(size_t index) {
    return 0;
}

void TCPQueue::setLong(size_t index, long value) {

}

void TCPQueue::appendBytesTo(int startPos, int endPos, ByteBuffer& outputBuffer) {

}

void TCPQueue::appendBytesTo(int startPos, int endPos, char* output) {

}
