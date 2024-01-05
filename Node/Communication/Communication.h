#ifndef COMMUNICATION_H
#define COMMUNICATION_H


class Communication {
public:
    virtual void begin(long frequency) = 0;
    virtual void send(String message) = 0;
    virtual String receive() = 0;
};

#endif
