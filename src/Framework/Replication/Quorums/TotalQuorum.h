#ifndef TOTALQUORUM_H
#define TOTALQUORUM_H

#include "Quorum.h"

/*
===============================================================================================

 TotalQuorum

 Everynode has to agree for there to be consensus.

===============================================================================================
*/

class TotalQuorum : public Quorum
{
public:
    TotalQuorum();
    
    void                AddNode(uint64_t nodeID);
    void                ClearNodes();
    bool                IsMember(uint64_t nodeID) const;
    unsigned            GetNumNodes() const;
    const uint64_t*     GetNodes() const;
    QuorumVote*         NewVote() const;    

private:
    uint64_t            nodes[9];
    unsigned            numNodes;
    unsigned            numAccepted;
    unsigned            numRejected;
};

/*
===============================================================================================

 TotalQuorumVote

===============================================================================================
*/

class TotalQuorumVote : public QuorumVote
{
public:
    TotalQuorumVote(TotalQuorum* quorum);
    
    void                RegisterAccepted(uint64_t nodeID);
    void                RegisterRejected(uint64_t nodeID);
    void                Reset();

    bool                IsRejected() const;
    bool                IsAccepted() const;
    bool                IsComplete() const;

private:
    TotalQuorum*        quorum;
    unsigned            numAccepted;
    unsigned            numRejected;
};


#endif
