package com.scalien.scaliendb;

public class ByteIterParams
{
    public byte[] prefix = new byte[0];
    public byte[] startKey = new byte[0];
    public byte[] endKey = new byte[0];
    
    public ByteIterParams prefix(byte[] prefix)
    {
        this.prefix = prefix;
        return this;
    }
    
    public ByteIterParams startKey(byte[] startKey)
    {
        this.startKey = startKey;
        return this;
    }
    
    public ByteIterParams endKey(byte[] endKey)
    {
        this.endKey = endKey;
        return this;
    }
}