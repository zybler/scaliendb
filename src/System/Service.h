#ifndef SERVICE_H
#define SERVICE_H

// These are the same codes what Windows uses
#define SERVICE_STATUS_RUNNING          0x00000004
#define SERVICE_STATUS_STOP_PENDING     0x00000003

class Service
{
public:
    static bool     Main(int argc, char** argv, void (*func)());
    static void     SetStatus(unsigned status);
};

#endif