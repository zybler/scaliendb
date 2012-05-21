#include "ReplicatedLog.h"
#include "Framework/Replication/ReplicationConfig.h"
#include "System/Events/EventLoop.h"

//#define RLOG_DEBUG_MESSAGES 1

static Buffer dummy;

ReplicatedLog::ReplicatedLog()
{
    canaryTimer.SetCallable(MFUNC(ReplicatedLog, OnCanaryTimeout));
    canaryTimer.SetDelay(CANARY_TIMEOUT);
}

void ReplicatedLog::Init(QuorumContext* context_)
{
    Log_Trace();
    
    context = context_;

    paxosID = 0;
    waitingOnAppend = false;
    
    proposer.Init(context);
    acceptor.Init(context);

    lastRequestChosenTime = 0;
    lastLearnChosenTime = 0;

    EventLoop::Add(&canaryTimer);
    
    dummy.Write("dummy");
}

void ReplicatedLog::Shutdown()
{
    EventLoop::Remove(&canaryTimer);
    proposer.RemoveTimers();
}

uint64_t ReplicatedLog::GetLastLearnChosenTime()
{
    return lastLearnChosenTime;
}

void ReplicatedLog::Stop()
{
    proposer.Stop();
}

void ReplicatedLog::Continue()
{
    if (context->IsLeaseKnown() && context->GetHighestPaxosID() > GetPaxosID())
        RequestChosen(context->GetLeaseOwner());
}

bool ReplicatedLog::IsMultiPaxosEnabled()
{
    return proposer.state.multi;
}

bool ReplicatedLog::IsAppending()
{
    return context->IsLeaseOwner() && proposer.state.numProposals > 0;
}

void ReplicatedLog::TryAppendDummy()
{
    Log_Trace();
    
    proposer.SetUseTimeouts(true);

    if (proposer.IsActive())
        return;
    
    Append(dummy);
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("Appending DUMMY!");
#endif
}

void ReplicatedLog::TryAppendNextValue()
{
    Log_Trace();
    
    if (!context->IsLeaseOwner() || proposer.IsActive() || !proposer.state.multi)
        return;
    
    Buffer& value = context->GetNextValue();
    if (value.GetLength() == 0)
        return;
    
    proposer.SetUseTimeouts(context->UseProposeTimeouts());
    Append(value);
}

void ReplicatedLog::TryCatchup()
{
    if (context->IsLeaseKnown() && context->GetHighestPaxosID() > GetPaxosID())
        RequestChosen(context->GetLeaseOwner());
}

void ReplicatedLog::Restart()
{
    context->OnStartProposing();

    if (proposer.IsActive())
        proposer.Restart();
}

void ReplicatedLog::SetPaxosID(uint64_t paxosID_)
{
    paxosID = paxosID_;
}

uint64_t ReplicatedLog::GetPaxosID()
{
    return paxosID;
}

void ReplicatedLog::NewPaxosRound()
{
    paxosID++;
    proposer.RemoveTimers();
    proposer.state.OnNewPaxosRound();
    acceptor.state.OnNewPaxosRound();
    lastRequestChosenTime = 0;
}

void ReplicatedLog::RegisterPaxosID(uint64_t paxosID, uint64_t nodeID)
{
    Log_Trace();
    
    if (paxosID > GetPaxosID())
    {
        //  I am lagging and need to catch-up
        RequestChosen(nodeID);
    }
}

void ReplicatedLog::OnMessage(PaxosMessage& imsg)
{
    Log_Trace();
    bool processed;

    processed = false;
    if (imsg.type == PAXOS_PREPARE_REQUEST)
        processed = OnPrepareRequest(imsg);
    else if (imsg.IsPrepareResponse())
        processed = OnPrepareResponse(imsg);
    else if (imsg.type == PAXOS_PROPOSE_REQUEST)
        processed = OnProposeRequest(imsg);
    else if (imsg.IsProposeResponse())
        processed = OnProposeResponse(imsg);
    else if (imsg.IsLearn())
        processed = OnLearnChosen(imsg);
    else if (imsg.type == PAXOS_REQUEST_CHOSEN)
        processed = OnRequestChosen(imsg);
    else if (imsg.type == PAXOS_START_CATCHUP)
        processed = OnStartCatchup(imsg);
    else
        ASSERT_FAIL();

    if (processed)
        context->OnMessageProcessed();
}

void ReplicatedLog::OnCatchupStarted()
{
    acceptor.OnCatchupStarted();
}

void ReplicatedLog::OnCatchupComplete(uint64_t paxosID_)
{
    paxosID = paxosID_;
    
    acceptor.OnCatchupComplete(); // commits
    
    NewPaxosRound();
}

void ReplicatedLog::OnLearnLease()
{
    Log_Trace("context->IsLeaseOwner()   = %s", (context->IsLeaseOwner() ? "true" : "false"));
    Log_Trace("!proposer.IsActive()  = %s", (!proposer.IsActive() ? "true" : "false"));
    Log_Trace("!proposer.state.multi = %s", (!proposer.state.multi ? "true" : "false"));
    if (context->IsLeaseOwner() && !proposer.IsActive() && !proposer.state.multi)
    {
        Log_Trace("Appending dummy to enable MultiPaxos");
        TryAppendDummy();
    }
}

void ReplicatedLog::OnLeaseTimeout()
{
    proposer.Stop();
}

void ReplicatedLog::OnAppendComplete()
{
    waitingOnAppend = false;

    NewPaxosRound(); // increments paxosID, clears proposer, acceptor
    
    if (context->IsLeaseKnown() && paxosID <= context->GetHighestPaxosID())
        RequestChosen(context->GetLeaseOwner());

    if (paxosID < context->GetHighestPaxosID())
        context->GetDatabase()->Commit();

    if (!context->UseCommitChaining())
    {
        acceptor.WriteState();
        context->GetDatabase()->Commit();
    }

    context->OnMessageProcessed();

    TryAppendNextValue();
}

void ReplicatedLog::WriteState()
{
    acceptor.WriteState();
}

uint64_t ReplicatedLog::GetMemoryUsage()
{
    return sizeof(ReplicatedLog) - 
        sizeof(PaxosAcceptor) + acceptor.GetMemoryUsage() -
        sizeof(PaxosProposer) + proposer.GetMemoryUsage();
}

void ReplicatedLog::Append(Buffer& value)
{
    Log_Trace();
        
    if (proposer.IsActive())
        return;
    
    context->OnStartProposing();
    proposer.Propose(value);
    
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("Proposing for paxosID = %U", GetPaxosID());
#endif
}

bool ReplicatedLog::OnPrepareRequest(PaxosMessage& imsg)
{
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnPrepareRequest");
#endif

    bool processed = acceptor.OnPrepareRequest(imsg);

    OnRequest(imsg);

    return processed;
}

bool ReplicatedLog::OnPrepareResponse(PaxosMessage& imsg)
{
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnPrepareResponse");
#endif

    Log_Trace();
    
    if (imsg.paxosID == paxosID)
        proposer.OnPrepareResponse(imsg);
    
    return true;
}

bool ReplicatedLog::OnProposeRequest(PaxosMessage& imsg)
{
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnProposeRequest");
#endif

    Log_Trace();
    
    bool processed = acceptor.OnProposeRequest(imsg);
    
    OnRequest(imsg);

    return processed;
}

bool ReplicatedLog::OnProposeResponse(PaxosMessage& imsg)
{
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnProposeResponse");
#endif

    Log_Trace();

    if (imsg.paxosID == paxosID)
        proposer.OnProposeResponse(imsg);

    return true;
}

bool ReplicatedLog::OnLearnChosen(PaxosMessage& imsg)
{
    uint64_t        runID;

#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("OnLearnChosen begin");
#endif

    if (context->GetDatabase()->IsCommitting())
    {
#ifdef RLOG_DEBUG_MESSAGES
        Log_Debug("Database is commiting, dropping Paxos message");
#endif
        return true;
    }

    if (waitingOnAppend)
    {
#ifdef RLOG_DEBUG_MESSAGES
        Log_Debug("Waiting OnAppend, dropping Paxos message");
#endif
        return true;
    }

    if (imsg.nodeID != MY_NODEID && proposer.state.multi)
    {
        Log_Debug("Received learn message from %U, but I'm in multi paxos mode", imsg.nodeID);
        return true;
    }

    if (imsg.nodeID != MY_NODEID && context->IsLeaseOwner())
    {
        Log_Debug("Received learn message from %U, but I'm the lease owner", imsg.nodeID);
        return true;
    }

    Log_Trace();

    if (imsg.paxosID > paxosID)
    {
        RequestChosen(imsg.nodeID); //  I am lagging and need to catch-up
        return true;
    }
    else if (imsg.paxosID < paxosID)
        return true;
    
    if (imsg.type == PAXOS_LEARN_VALUE)
    {
        runID = 0;
        // in the PAXOS_LEARN_VALUE case (and only in this case) runID is 0
        // for legacy reasons the PAXOS_LEARN_VALUE message also includes a runID,
        // which is always set to 0
        acceptor.state.accepted = true;
        acceptor.state.acceptedValue.Write(imsg.value);
        acceptor.WriteState();
    }
    else if (imsg.type == PAXOS_LEARN_PROPOSAL && acceptor.state.accepted &&
     acceptor.state.acceptedProposalID == imsg.proposalID)
    {
        runID = acceptor.state.acceptedRunID;
    }
    else
    {
        RequestChosen(imsg.nodeID);
        return true;
    }
    
    lastLearnChosenTime = EventLoop::Now();
    ProcessLearnChosen(imsg.nodeID, runID);

#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("OnLearnChosen end");
#endif

    return false;
}

bool ReplicatedLog::OnRequestChosen(PaxosMessage& imsg)
{
    Buffer          value;
    PaxosMessage    omsg;
    
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnRequestChosen, imsg.paxosID = %U, mine = %U",
     imsg.paxosID, GetPaxosID());
#endif

    if (imsg.paxosID >= GetPaxosID())
        return true;
    
    // the node is lagging and needs to catch-up

    if (context->AlwaysUseDatabaseCatchup() && imsg.paxosID < GetPaxosID())
    {
        omsg.StartCatchup(paxosID, MY_NODEID);
    }
    else
    {
        context->GetDatabase()->GetAcceptedValue(imsg.paxosID, value);
        if (value.GetLength() > 0)
        {
            Log_Trace("Sending paxosID %d to node %d", imsg.paxosID, imsg.nodeID);
            omsg.LearnValue(imsg.paxosID, MY_NODEID, 0, value);
        }
        else
        {
            Log_Trace("Node requested a paxosID I no longer have");
            omsg.StartCatchup(paxosID, MY_NODEID);
        }
    }
    context->GetTransport()->SendMessage(imsg.nodeID, omsg);

    return true;
}

bool ReplicatedLog::OnStartCatchup(PaxosMessage& imsg)
{
#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::OnStartCatchup");
#endif

    if (imsg.nodeID == context->GetLeaseOwner())
        context->OnStartCatchup();

    return true;
}

void ReplicatedLog::OnCanaryTimeout()
{
    if (context->IsLeaseOwner() && !IsAppending())
        TryAppendDummy();

    EventLoop::Add(&canaryTimer);
}

void ReplicatedLog::ProcessLearnChosen(uint64_t nodeID, uint64_t runID)
{
    bool    ownAppend;
    Buffer  learnedValue;

    learnedValue.Write(acceptor.state.acceptedValue);

#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("Round completed for paxosID = %U", paxosID);
    Log_Trace("+++ Value for paxosID = %U: %B +++", paxosID, &learnedValue);
    if (context->GetHighestPaxosID() > 0 && paxosID < context->GetHighestPaxosID())
    {
        Log_Debug("Paxos-based catchup, highest seen paxosID is %U, currently at %U",
         context->GetHighestPaxosID(), paxosID);
        if (paxosID == (context->GetHighestPaxosID() - 1))
            Log_Debug("Paxos-based catchup complete...");
    }
#endif
    
    if (context->GetHighestPaxosID() > 0 && paxosID < (context->GetHighestPaxosID() - 1))
        context->GetDatabase()->Commit();
    
    ownAppend = proposer.state.multi;
    if (nodeID == MY_NODEID && runID == REPLICATION_CONFIG->GetRunID() && context->IsLeaseOwner())
    {
        proposer.state.multi = true;
        if (!ownAppend)
            context->OnIsLeader();
        Log_Trace("Multi paxos enabled");
    }
    else
    {
        proposer.state.multi = false;
        Log_Trace("Multi paxos disabled");
    }

    ownAppend &= proposer.state.multi;

    if (BUFCMP(&learnedValue, &dummy))
        OnAppendComplete();
    else
    {
        waitingOnAppend = true;
        context->OnAppend(paxosID, learnedValue, ownAppend);
        // QuorumContext::OnAppend() must call ReplicatedLog::OnAppendComplete()
    }
}

void ReplicatedLog::OnRequest(PaxosMessage& imsg)
{
    uint64_t        sendPaxosID;
    uint64_t        total;
    Buffer          value;
    PaxosMessage    omsg;

    Log_Trace();

    if (imsg.paxosID < GetPaxosID())
    {
        // the node is lagging and needs to catch-up
        // send more than one round for better thruput
        total = 0;
        for (sendPaxosID = imsg.paxosID; sendPaxosID <= GetPaxosID(); sendPaxosID++)
        {
            // TODO: use async get here
            context->GetDatabase()->GetAcceptedValue(sendPaxosID, value);
            if (value.GetLength() == 0)
                return;
            omsg.LearnValue(sendPaxosID, MY_NODEID, 0, value);
            context->GetTransport()->SendMessage(imsg.nodeID, omsg);
            total += value.GetLength();
            if (total >= PAXOS_CATCHUP_GRANULARITY)
                break;
        }
    }
    else if (GetPaxosID() < imsg.paxosID)
    {
        //  I am lagging and need to catch-up
        RequestChosen(imsg.nodeID);
    }
}

void ReplicatedLog::RequestChosen(uint64_t nodeID)
{
    PaxosMessage omsg;
    
    if (context->IsLeaseOwner())
        return;
    if (EventLoop::Now() - lastRequestChosenTime < REQUEST_CHOSEN_TIMEOUT)
        return;
    if (waitingOnAppend)
        return;

    lastRequestChosenTime = EventLoop::Now();
    
    omsg.RequestChosen(GetPaxosID(), MY_NODEID);
    
    context->GetTransport()->SendMessage(nodeID, omsg);

#ifdef RLOG_DEBUG_MESSAGES
    Log_Debug("ReplicatedLog::RequestChosen, paxosID = %U, to = %U", GetPaxosID(), nodeID);
#endif
}
