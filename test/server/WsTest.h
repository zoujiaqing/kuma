#ifndef __WsTest_H__
#define __WsTest_H__

#include "kmapi.h"
#include "TestLoop.h"

#include <map>

using namespace kuma;

class WsTest : public LoopObject
{
public:
    WsTest(TestLoop* loop, long conn_id);

    KMError attachFd(SOCKET_FD fd, uint32_t ssl_flags);
    KMError attachSocket(TcpSocket&& tcp, HttpParser&& parser);
    int close();
    
    void onSend(KMError err);
    void onClose(KMError err);
    
    void onData(uint8_t*, size_t);
    
private:
    void cleanup();
    void sendTestData();
    
private:
    TestLoop*       loop_;
    WebSocket       ws_;
    long            conn_id_;
};

#endif
