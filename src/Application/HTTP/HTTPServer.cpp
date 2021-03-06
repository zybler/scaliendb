#include "HTTPServer.h"
#include "System/IO/IOProcessor.h"

#define CONN_BACKLOG    10

void HTTPServer::Init(int port)
{
    if (!TCPServer<HTTPServer, HTTPConnection>::Init(port, true, CONN_BACKLOG))
        STOP_FAIL(1, "Cannot initialize HTTP server at port %d (Already running?)", port);
    handlers = NULL;
}

void HTTPServer::Shutdown()
{
    Close();
}

void HTTPServer::InitConn(HTTPConnection* conn)
{
    conn->Init(this);
}

void HTTPServer::RegisterHandler(HTTPHandler* handler)
{
    handler->nextHTTPHandler = handlers;
    handlers = handler;
}

bool HTTPServer::HandleRequest(HTTPConnection* conn, HTTPRequest& request)
{
    HTTPHandler*    handler;
    bool            ret;
    
    // call each handler until one handles the request
    ret = false;
    for (handler = handlers; handler && !ret; handler = handler->nextHTTPHandler)
        ret = handler->HandleRequest(conn, request);
    
    return ret;
}

