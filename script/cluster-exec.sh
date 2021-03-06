#!/bin/sh

. $(dirname $0)/scaliendb-env.sh

CMD=$*

for shard_server in $SCALIENDB_SHARDSERVERS; do
        remote_cmd="cd $SCALIENDB_HOME; $CMD"
        echo "$shard_server:\$ $remote_cmd"
        remote_cmd_redirect="$remote_cmd 2>&1 1>/dev/null"
        ssh $shard_server $remote_cmd_redirect | sed s/^/$shard_server:\ / &
done

wait
