import random
import sys
import scaliendb

CONTROLLERS=["localhost:7080"]
#CONTROLLERS=["192.168.137.51:7080"]

def sizeof_fmt(num):
	for x in ['bytes','KB','MB','GB','TB']:
		if num < 1024.0:
			return "%3.1f%s" % (num, x)
		num /= 1024.0

limit = 0
if len(sys.argv) > 1:
	limit = int(sys.argv[1])
client = scaliendb.Client(CONTROLLERS)
#client._set_trace(True)

if False:
	quorum_id = client.create_quorum(["100"])
	database_id = client.create_database("testdb")
	client.create_table(database_id, quorum_id, "testtable")
                        
client.use_database("testdb")
client.use_table("testtable")
sent = 0
value = "%100s" % " "
i = 0
batch = 10000
while limit == 0 or sent < limit:
	client.begin()
	for x in xrange(batch):
		client.set(str(random.randint(1, 1000000000)), value)
		sent += len(value)
	client.submit()
	i += batch
	print("Sent bytes: %s, num: %i" % (sizeof_fmt(sent), i))