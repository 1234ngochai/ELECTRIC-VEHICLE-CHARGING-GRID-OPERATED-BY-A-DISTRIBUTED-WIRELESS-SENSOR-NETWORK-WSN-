#include "ev_buffer.h"

// Initializes the buffer, setting its front and rear pointers to default positions.
void setupEVBuffer(EVBuffer* buffer) {
    buffer->front = -1;
    buffer->rear = -1;
}

// Determines whether the provided buffer has reached its capacity.
// Returns 1 if the buffer is full, otherwise 0.
int bufferIsFull(EVBuffer* buffer) {
    return (buffer->rear + 1) % BUFFER_SIZE == buffer->front;
}

// Checks if the provided buffer is empty, meaning no data is present.
// Returns 1 if the buffer is empty, otherwise 0.
int bufferIsEmpty(EVBuffer* buffer) {
    return buffer->front == -1;
}

// Inserts the provided port data into the buffer. If the buffer is already full,
// the oldest value will be removed to make room for the new data.
void addDataToBuffer(EVBuffer* buffer, PortData pdata) {
    if (bufferIsFull(buffer)) {
        buffer->front = (buffer->front + 1) % BUFFER_SIZE;
    }
    if (buffer->front == -1) {
        buffer->front = 0;
    }
    buffer->rear = (buffer->rear + 1) % BUFFER_SIZE;
    buffer->data[buffer->rear] = pdata;
}
