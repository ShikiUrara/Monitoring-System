#ifndef COMMUNICATION_H
#define COMMUNICATION_H


class Communication {
public:
    virtual void begin(long frequency) = 0;
    virtual void sendNode(String message) = 0;
    virtual String receiveNode() = 0;
};

// class Communication {
// public:
//     virtual void init() = 0;
//     virtual void send(const char* message) = 0;
//     virtual const char* receive() = 0;
// };

#endif
