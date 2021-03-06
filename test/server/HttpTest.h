#ifndef __HttpTest_H__
#define __HttpTest_H__

#include "kmapi.h"
#include "TestLoop.h"

#include <map>

using namespace kuma;

class HttpTest : public LoopObject
{
public:
    HttpTest(TestLoop* loop, long conn_id);

    KMError attachFd(SOCKET_FD fd, uint32_t ssl_flags);
    KMError attachSocket(TcpSocket&& tcp, HttpParser&& parser);
    int close();
    
    void onSend(KMError err);
    void onClose(KMError err);
    
    void onHttpData(uint8_t*, size_t);
    void onHeaderComplete();
    void onRequestComplete();
    void onResponseComplete();
    
private:
    void cleanup();
    void sendTestData();
    
private:
    TestLoop*      loop_;
    HttpResponse    http_;
    long            conn_id_;
    bool            is_options_;
};

#endif
