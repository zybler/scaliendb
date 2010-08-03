//#ifndef CONTROLCHUNKCONTEXT_H
//#define CONTROLCHUNKCONTEXT_H 
//
//#include "Framework/Replication/Quorums/QuorumContext.h"
//#include "Framework/Replication/Quorums/DoubleQuorum.h"
//#include "Framework/Replication/ReplicatedLog/ReplicatedLog.h"
//#include "Framework/Replication/PaxosLease/PaxosLease.h"
//
//class Controller; // forward
//
///*
//===============================================================================
//
// ControlChunkContext
//
//===============================================================================
//*/
//
//class ControlChunkContext : public QuorumContext
//{
//public:
//	void							Start();
//	
//	void							SetController(Controller* controller);
//	void							SetContextID(uint64_t contextID);
//	void							SetQuorum(DoubleQuorum& quorum);
//	void							SetDatabase(QuorumDatabase& database);
//	void							SetTransport(QuorumTransport& transport);
//	
//	// implementation of QuorumContext interface:
//	virtual bool					IsLeaderKnown();
//	virtual bool					IsLeader();
//	virtual uint64_t				GetLeader();
//	virtual uint64_t				GetContextID() const;
//
//	virtual void					OnLearnLease();
//	virtual void					OnLeaseTimeout();
//
//	virtual uint64_t				GetPaxosID() const;
//	virtual void					SetPaxosID(uint64_t paxosID);
//	virtual uint64_t				GetHighestPaxosID() const;
//
//	virtual Quorum*					GetQuorum();
//	virtual QuorumDatabase*			GetDatabase();
//	virtual QuorumTransport*		GetTransport();
//
//	virtual	void					OnAppend(ReadBuffer value);
//	virtual Buffer*					GetNextValue();
//	virtual void					OnMessage();
//
//private:
//	void							OnPaxosMessage(ReadBuffer buffer);
//	void							RegisterPaxosID(uint64_t paxosID);
//
//	Controller*						controller;
//	DoubleQuorum					quorum;
//	QuorumDatabase					database;
//	QuorumTransport					transport;
//
//	ReplicatedLog					replicatedLog;
//
//	uint64_t						contextID;
//	uint64_t						highestPaxosID;
//	Buffer							nextValue;
//};
//
//#endif
