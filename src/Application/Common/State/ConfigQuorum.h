#ifndef CONFIGQUORUM_H
#define CONFIGQUORUM_H

#include "System/Common.h"
#include "System/Containers/List.h"
#include "System/Containers/ArrayList.h"

// TODO: create ConfigConsts.h and define there
#define CONFIG_MAX_NODES                7

#define CONFIG_QUORUM_PRODUCTION        'P'
#define CONFIG_QUORUM_TEST              'T'

/*
===============================================================================================

 ConfigQuorum

===============================================================================================
*/

class ConfigQuorum
{
public:
    typedef ArrayList<uint64_t, CONFIG_MAX_NODES> NodeList;

    ConfigQuorum();
    ConfigQuorum(const ConfigQuorum& other);
    
    ConfigQuorum&       operator=(const ConfigQuorum& other);

    uint64_t            quorumID;
    NodeList            activeNodes;
    NodeList            inactiveNodes;
    List<uint64_t>      shards;
    
    // ========================================================================================
    //
    // Not replicated, only stored by the MASTER in-memory
    bool                hasPrimary;
    uint64_t            primaryID;
    //
    // ========================================================================================

    ConfigQuorum*       prev;
    ConfigQuorum*       next;
};


inline ConfigQuorum::ConfigQuorum()
{
    prev = next = this; hasPrimary = false;
}

inline ConfigQuorum::ConfigQuorum(const ConfigQuorum& other)
{
    *this = other;
}

inline ConfigQuorum& ConfigQuorum::operator=(const ConfigQuorum& other)
{
    uint64_t*   sit;
    
    quorumID = other.quorumID;
    activeNodes = other.activeNodes;
    inactiveNodes = other.inactiveNodes;
    
    for (sit = other.shards.First(); sit != NULL; sit = other.shards.Next(sit))
        shards.Append(*sit);
    
    hasPrimary = other.hasPrimary;
    primaryID = other.primaryID;
    
    prev = next = this;
    
    return *this;
}

#endif