#ifndef QUORUMTRANSPORT_H
#define QUORUMTRANSPORT_H

#include "Quorum.h"
#include "Framework/Replication/ReplicationTransport.h"
#include "Framework/Replication/ReplicationMessage.h"

/*
===============================================================================

 QuorumTransport

===============================================================================
*/

class QuorumTransport
{
public:
	QuorumTransport();
	
	void					SetPriority(bool priority);
	void					SetReplicationTransport(ReplicationTransport* transport);
	void					SetQuorum(Quorum* quorum);
	
	ReplicationMessage*		GetMessage() const;

	void					SendMessage(unsigned nodeID, const ReplicationMessage& msg);
	void					BroadcastMessage(const ReplicationMessage& msg);

private:
	bool					priority;
	Quorum*					quorum;
	ReplicationTransport*	transport;
};

#endif
