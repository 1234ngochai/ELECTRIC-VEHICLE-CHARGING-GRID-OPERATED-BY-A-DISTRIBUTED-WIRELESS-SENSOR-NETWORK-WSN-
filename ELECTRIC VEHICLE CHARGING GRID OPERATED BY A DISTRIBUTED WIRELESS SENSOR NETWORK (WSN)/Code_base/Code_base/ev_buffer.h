#ifndef EV_BUFFER_H
#define EV_BUFFER_H

#define BUFFER_SIZE 10  

typedef struct {
    int year, month, day, hour, minute, second;
    int available_ports;
} PortData;

typedef struct {
    PortData data[BUFFER_SIZE];
    int front;
    int rear;
} EVBuffer;



void setupEVBuffer(EVBuffer* buffer);


int bufferIsFull(EVBuffer* buffer);


int bufferIsEmpty(EVBuffer* buffer);


void addDataToBuffer(EVBuffer* buffer, PortData pdata);

#endif // EV_BUFFER_H
