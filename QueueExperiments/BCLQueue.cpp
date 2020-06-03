//
// Created by liampilot on 31/05/2020.
//

#include "BCLQueue.h"

long BCLQueue::put(char* values, long bytes, long latencyMark) {
    return 0;
}

void BCLQueue::free() {

}

void BCLQueue::free(long offset) {

}

ByteBuffer& BCLQueue::getBuffer() {
    return <#initializer#>;
}

size_t BCLQueue::getBufferCapacity(int id) {
    return 0;
}

long BCLQueue::getLong(size_t index) {
    return 0;
}

void BCLQueue::setLong(size_t index, long value) {

}

void BCLQueue::appendBytesTo(int startPos, int endPos, ByteBuffer& outputBuffer) {

}

void BCLQueue::appendBytesTo(int startPos, int endPos, char* output) {

}
