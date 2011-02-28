#ifndef SDBPRESULT_H
#define SDBPRESULT_H

#include "System/Containers/InTreeMap.h"
#include "Application/Common/ClientResponse.h"
#include "SDBPClientRequest.h"

namespace SDBPClient
{

/*
===============================================================================================

 SDBPClient::Result

===============================================================================================
*/

class Result
{
public:
    ~Result();
    
    void            Close();
    
    void            Begin();
    void            Next();
    bool            IsEnd();

    bool            IsFinished();

    void            SetBatchLimit(uint64_t limit);
    
    bool            AppendRequest(Request* req);
    bool            AppendRequestResponse(ClientResponse* resp);
    void            RemoveRequest(Request* req);

    int             GetCommandStatus();
    int             GetTransportStatus();
    void            SetTransportStatus(int status);
    void            SetConnectivityStatus(int status);
    int             GetTimeoutStatus();
    void            SetTimeoutStatus(int status);

    int             GetKey(ReadBuffer& key);
    int             GetValue(ReadBuffer& value);
    int             GetNumber(uint64_t& number);
    
    int             GetDatabaseID(uint64_t& databaseID);
    int             GetTableID(uint64_t& tableID);

    unsigned        GetRequestCount();
    Request*        GetRequestCursor();

private:
    typedef InTreeMap<Request> RequestMap;
    
    RequestMap      requests;
    int             transportStatus;
    int             timeoutStatus;
    int             connectivityStatus;
    unsigned        numCompleted;
    Request*        requestCursor;
    ClientResponse** responseCursor;
    unsigned        responsePos;
    uint64_t        batchLimit;
    uint64_t        batchSize;
};

};  // namespace

#endif
