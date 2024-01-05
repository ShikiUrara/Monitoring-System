#ifndef PROTOCOL_H
#define PROTOCOL_H


class Protocol {
public:
    virtual void sendDataCenter(const char* link, const char* message)= 0;
    virtual String receiveDataCenter(const char* link)= 0;
    virtual bool connectDataCenter()= 0;
    //virtual void disconnect();
    
};

#endif
