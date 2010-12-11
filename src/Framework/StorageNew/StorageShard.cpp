#include "StorageShard.h"

inline bool LessThan(StorageChunk* a, StorageChunk* b)
{
    return (a->GetChunkID() < b->GetChunkID());
}

uint64_t StorageShard::GetTableID()
{
    return tableID;
}

uint64_t StorageShard::GetShardID()
{
    return shardID;
}

ReadBuffer StorageShard::GetFirstKey()
{
    return ReadBuffer(firstKey);
}

ReadBuffer StorageShard::GetLastKey()
{
    return ReadBuffer(lastKey);
}

bool StorageShard::RangeContains(ReadBuffer& key)
{
    int         cf, cl;
    ReadBuffer  firstKey, lastKey;

    firstKey = GetFirstKey();
    lastKey = GetLastKey();

    cf = ReadBuffer::Cmp(firstKey, key);
    cl = ReadBuffer::Cmp(key, lastKey);

    if (firstKey.GetLength() == 0)
    {
        if (lastKey.GetLength() == 0)
            return true;
        else
            return (cl < 0);        // (key < lastKey);
    }
    else if (lastKey.GetLength() == 0)
    {
        return (cf <= 0);           // (firstKey <= key);
    }
    else
        return (cf <= 0 && cl < 0); // (firstKey <= key < lastKey);
}

StorageMemoChunk* StorageShard::GetMemoChunk()
{
    return (StorageMemoChunk*) chunks.Last();
}

void StorageShard::SetNewMemoChunk(StorageMemoChunk* memoChunk)
{
    chunks.Add(memoChunk);
}

StorageChunk** StorageShard::First()
{
    return chunks.First();
}

StorageChunk** StorageShard::Last()
{
    return chunks.Last();
}

StorageChunk** StorageShard::Next(StorageChunk** curr)
{
    return chunks.Next(curr);
}

StorageChunk** StorageShard::Prev(StorageChunk** curr)
{
    return chunks.Prev(curr);
}
