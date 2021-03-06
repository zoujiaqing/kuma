#include "HttpTest.h"
#include "TestLoop.h"

#include <string.h>

#ifdef KUMA_OS_WIN
#define strcasecmp _stricmp
#endif

HttpTest::HttpTest(TestLoop* loop, long conn_id)
: loop_(loop)
, http_(loop->getEventLoop())
, conn_id_(conn_id)
, is_options_(false)
{
    
}

KMError HttpTest::attachFd(SOCKET_FD fd, uint32_t ssl_flags)
{
    http_.setWriteCallback([this] (KMError err) { onSend(err); });
    http_.setErrorCallback([this] (KMError err) { onClose(err); });
    
    http_.setDataCallback([this] (uint8_t* data, size_t len) { onHttpData(data, len); });
    http_.setHeaderCompleteCallback([this] () { onHeaderComplete(); });
    http_.setRequestCompleteCallback([this] () { onRequestComplete(); });
    http_.setResponseCompleteCallback([this] () { onResponseComplete(); });
    http_.setSslFlags(ssl_flags);
    return http_.attachFd(fd);
}

KMError HttpTest::attachSocket(TcpSocket&& tcp, HttpParser&& parser)
{
    http_.setWriteCallback([this] (KMError err) { onSend(err); });
    http_.setErrorCallback([this] (KMError err) { onClose(err); });
    
    http_.setDataCallback([this] (uint8_t* data, size_t len) { onHttpData(data, len); });
    http_.setHeaderCompleteCallback([this] () { onHeaderComplete(); });
    http_.setRequestCompleteCallback([this] () { onRequestComplete(); });
    http_.setResponseCompleteCallback([this] () { onResponseComplete(); });
    
    return http_.attachSocket(std::move(tcp), std::move(parser));
}

int HttpTest::close()
{
    http_.close();
    return 0;
}

void HttpTest::onSend(KMError err)
{
    sendTestData();
}

void HttpTest::onClose(KMError err)
{
    printf("HttpTest::onClose, err=%d\n", err);
    loop_->removeObject(conn_id_);
}

void HttpTest::onHttpData(uint8_t* data, size_t len)
{
    printf("HttpTest::onHttpData, len=%zu\n", len);
}

void HttpTest::onHeaderComplete()
{
    printf("HttpTest::onHeaderComplete\n");
}

void HttpTest::onRequestComplete()
{
    printf("HttpTest::onRequestComplete\n");
    if (strcasecmp(http_.getMethod(), "OPTIONS") == 0) {
        http_.addHeader("Content-Length", (uint32_t)0);
        is_options_ = true;
    } else {
        http_.addHeader("Content-Length", (uint32_t)256*1024*1024);
        is_options_ = false;
    }
    http_.addHeader("Access-Control-Allow-Origin", "*");
    const char* hdr = http_.getHeaderValue("Access-Control-Request-Headers");
    if (hdr) {
        http_.addHeader("Access-Control-Allow-Headers", hdr);
    }
    hdr = http_.getHeaderValue("Access-Control-Request-Method");
    if (hdr) {
        http_.addHeader("Access-Control-Allow-Methods", hdr);
    }
    http_.sendResponse(200, "OK");
}

void HttpTest::onResponseComplete()
{
    printf("HttpTest::onResponseComplete\n");
    http_.reset();
}

void HttpTest::sendTestData()
{
    if (is_options_) {
        return;
    }
    uint8_t buf[128*1024];
    memset(buf, 'a', sizeof(buf));
    while (true) {
        int ret = http_.sendData(buf, sizeof(buf));
        if (ret < 0) {
            break;
        } else if (ret < sizeof(buf)) {
            break;
        }
    }
}
