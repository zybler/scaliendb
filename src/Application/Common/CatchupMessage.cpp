#include "CatchupMessage.h"

bool CatchupMessage::CatchupRequest(uint64_t nodeID)
{
}

bool CatchupMessage::BeginShard(uint64_t shardID)
{
}

bool CatchupMessage::KeyValue(ReadBuffer& key, ReadBuffer& value)
{
}

bool CatchupMessage::Commit(uint64_t paxosID)
{
}

bool CatchupMessage::Read(ReadBuffer& buffer)
{
}

bool CatchupMessage::Write(Buffer& buffer)
{
}