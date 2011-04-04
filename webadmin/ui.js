
function contains(arr, obj)
{
	var i = arr.length;
	while (i--)
	{
		if (arr[i] === obj)
			return true;
	}
	return false;
}

function init()
{	
	scaliendb.disconnect();

	scaliendb.util.elem("controller").textContent = "Not connected...";
	scaliendb.util.elem("clusterState").textContent = "Unable to connect!";
	scaliendb.util.elem("clusterState").className = "status-message critical";

	scaliendb.util.elem("shardservers").innerHTML = "";

	scaliendb.util.removeElement("quorums");
	var quorumsDiv = document.createElement("div");
	quorumsDiv.setAttribute("id", "quorums");
	scaliendb.util.elem("tabPageQuorums").appendChild(quorumsDiv);

	scaliendb.util.removeElement("databases");
	var databasesDiv = document.createElement("div");
	databasesDiv.setAttribute("id", "databases");
	scaliendb.util.elem("tabPageSchema").appendChild(databasesDiv);
}

function onLoad()
{
	scaliendb.onDisconnect = onDisconnect;
	init();
	scaliendb.util.elem("loginContainer").style.display = "block";
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("createQuorumContainer").style.display = "none";
	scaliendb.util.elem("deleteQuorumContainer").style.display = "none";
	scaliendb.util.elem("addNodeContainer").style.display = "none";
	scaliendb.util.elem("removeNodeContainer").style.display = "none";
	scaliendb.util.elem("createDatabaseContainer").style.display = "none";
	scaliendb.util.elem("renameDatabaseContainer").style.display = "none";
	scaliendb.util.elem("deleteDatabaseContainer").style.display = "none";
	scaliendb.util.elem("createTableContainer").style.display = "none";
	scaliendb.util.elem("renameTableContainer").style.display = "none";
	scaliendb.util.elem("truncateTableContainer").style.display = "none";
	scaliendb.util.elem("deleteTableContainer").style.display = "none";
	scaliendb.util.elem("splitShardContainer").style.display = "none";
	scaliendb.util.elem("migrateShardContainer").style.display = "none";
	scaliendb.util.elem("loginCluster").focus();
	removeOutline();
	
	activateDashboardTab();
	hideDialog = function() {}
	
	document.onkeydown = function(e) { if (getKeycode(e) == 27) hideDialog(); }
}

var md = false;
function onMouseDown()
{
	md = true;	
}

function onMouseUp()
{
	md = false;
}

function logout()
{
	clearTimeout(timer);
	onLoad();
}

function connect()
{
	scaliendb.util.elem("loginContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
	
	var controller = scaliendb.util.elem("loginCluster").value;
	if (!scaliendb.util.startsWith(controller, "http://"))
		controller = "http://" + controller;
	if (!scaliendb.util.endsWith("controller", "/json/"))
		controller = controller + "/json/";
	if (controller !== scaliendb.controller)
		scaliendb.controller = controller;
	updateConfigState();
}

tabs = ["Dashboard", "Quorums", "Schema", "Migration"];

function activateTab(name)
{
	for (var t in tabs)
	{
		var tab = tabs[t];
		if (tab == name)
		{
			scaliendb.util.elem("tabHead" + tab).className = "tab-head tab-head-active";
			scaliendb.util.elem("tabPage" + tab).className = "tab-page tab-page-active";
		}
		else
		{
			scaliendb.util.elem("tabHead" + tab).className = "tab-head tab-head-inactive";
			scaliendb.util.elem("tabPage" + tab).className = "tab-page tab-page-inactive";			
		}
	}
}

function activateDashboardTab()
{
	activateTab("Dashboard");
}

function activateQuorumsTab()
{
	activateTab("Quorums");
}

function activateSchemaTab()
{
	activateTab("Schema");
}

function activateMigrationTab()
{
	activateTab("Migration");
}

var tableShardsVisible = new Array();
function showhideShardsDiv(tableID)
{
	if (tableShardsVisible[tableID])
	{
		tableShardsVisible[tableID] = false;
		scaliendb.util.elem("shards_" + tableID).style.display = "none";
		scaliendb.util.elem("showhideShardsButton_" + tableID).innerHTML = "show shards";
	}
	else
	{
		tableShardsVisible[tableID] = true;
		scaliendb.util.elem("shards_" + tableID).style.display = "block";
		scaliendb.util.elem("showhideShardsButton_" + tableID).innerHTML = "hide shards";
	}
}

function showCreateQuorum()
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("createQuorumContainer").style.display = "block";
	scaliendb.util.elem("createQuorumShardServers").focus();
	hideDialog = hideCreateQuorum;
}

var deleteQuorumID;
function showDeleteQuorum(quorumID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("deleteQuorumContainer").style.display = "block";
	deleteQuorumID = quorumID;
	hideDialog = hideDeleteQuorum;
}

var addNodeQuorumID;
function showAddNode(quorumID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("addNodeContainer").style.display = "block";
	addNodeQuorumID = quorumID;
	hideDialog = hideAddNode;
}

var removeNodeQuorumID;
function showRemoveNode(quorumID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("removeNodeContainer").style.display = "block";
	removeNodeQuorumID = quorumID;
	hideDialog = hideRemoveNode;
}

function showCreateDatabase()
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("createDatabaseContainer").style.display = "block";
	scaliendb.util.elem("createDatabaseName").focus();
	hideDialog = hideCreateDatabase;
}

var renameDatabaseID;
function showRenameDatabase(databaseID, databaseName)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("renameDatabaseName").value = databaseName;
	scaliendb.util.elem("renameDatabaseContainer").style.display = "block";
	scaliendb.util.elem("renameDatabaseName").focus();
	renameDatabaseID = databaseID;
	hideDialog = hideRenameDatabase;
}

var deleteDatabaseID;
function showDeleteDatabase(databaseID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("deleteDatabaseContainer").style.display = "block";
	deleteDatabaseID = databaseID;
	hideDialog = hideDeleteDatabase;
}

var createTableDatabaseID;
function showCreateTable(databaseID, databaseName)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("createTableContainer").style.display = "block";
	scaliendb.util.elem("createTableName").focus();
	createTableDatabaseID = databaseID;
	hideDialog = hideCreateTable;
}

var renameTableID;
function showRenameTable(tableID, tableName)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("renameTableName").value = tableName;
	scaliendb.util.elem("renameTableContainer").style.display = "block";
	scaliendb.util.elem("renameTableName").focus();
	renameTableID = tableID;
	hideDialog = hideRenameTable;
}

var truncateTableID;
function showTruncateTable(tableID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("truncateTableContainer").style.display = "block";
	truncateTableID = tableID;
	hideDialog = hideTruncateTable;
}

var deleteTableID;
function showDeleteTable(tableID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("deleteTableContainer").style.display = "block";
	deleteTableID = tableID;
	hideDialog = hideDeleteTable;
}

var splitShardID;
function showSplitShard(shardID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("splitShardContainer").style.display = "block";
	splitShardID = shardID;
	hideDialog = hideSplitShard;
}

var migrateShardID;
function showMigrateShard(shardID)
{
	scaliendb.util.elem("mainContainer").style.display = "none";
	scaliendb.util.elem("migrateShardContainer").style.display = "block";
	migrateShardID = shardID;
	hideDialog = hideMigrateShard;
}

function hideCreateQuorum()
{
	scaliendb.util.elem("createQuorumContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideDeleteQuorum()
{
	scaliendb.util.elem("deleteQuorumContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideAddNode()
{
	scaliendb.util.elem("addNodeContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideRemoveNode()
{
	scaliendb.util.elem("removeNodeContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideCreateDatabase()
{
	scaliendb.util.elem("createDatabaseContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideRenameDatabase()
{
	scaliendb.util.elem("renameDatabaseContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideDeleteDatabase()
{
	scaliendb.util.elem("deleteDatabaseContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideCreateTable()
{
	scaliendb.util.elem("createTableContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideRenameTable()
{
	scaliendb.util.elem("renameTableContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideTruncateTable()
{
	scaliendb.util.elem("truncateTableContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideDeleteTable()
{
	scaliendb.util.elem("deleteTableContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideSplitShard()
{
	scaliendb.util.elem("splitShardContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function hideMigrateShard()
{
	scaliendb.util.elem("migrateShardContainer").style.display = "none";
	scaliendb.util.elem("mainContainer").style.display = "block";
}

function createQuorum()
{
	hideCreateQuorum();
	
	var nodes = scaliendb.util.elem("createQuorumShardServers").value;
	nodes = scaliendb.util.removeSpaces(nodes);
	scaliendb.onResponse = onResponse;
	scaliendb.createQuorum(nodes);
}

function deleteQuorum()
{
	hideDeleteQuorum();
	scaliendb.onResponse = onResponse;
	scaliendb.deleteQuorum(deleteQuorumID);
}

function addNode()
{
	hideAddNode();

	var nodeID = scaliendb.util.elem("addNodeShardServer").value;
	nodeID = scaliendb.util.removeSpaces(nodeID);
	scaliendb.onResponse = onResponse;
	scaliendb.addNode(addNodeQuorumID, nodeID);
}

function removeNode()
{
	hideRemoveNode();

	var nodeID = scaliendb.util.elem("removeNodeShardServer").value;
	nodeID = scaliendb.util.removeSpaces(nodeID);
	scaliendb.onResponse = onResponse;
	scaliendb.removeNode(removeNodeQuorumID, nodeID);
}

function activateNode(quorumID, nodeID)
{
	scaliendb.onResponse = onResponse;
	scaliendb.activateNode(quorumID, nodeID);
}

function createDatabase()
{
	hideCreateDatabase();
	var name = scaliendb.util.elem("createDatabaseName").value;
	name = scaliendb.util.removeSpaces(name);
	scaliendb.onResponse = onResponse;
	scaliendb.createDatabase(name);
}

function renameDatabase()
{
	hideRenameDatabase();
	var name = scaliendb.util.elem("renameDatabaseName").value;
	name = scaliendb.util.removeSpaces(name);
	scaliendb.onResponse = onResponse;
	scaliendb.renameDatabase(renameDatabaseID, name);
}

function deleteDatabase()
{
	hideDeleteDatabase();
	scaliendb.onResponse = onResponse;
	scaliendb.deleteDatabase(deleteDatabaseID);
}

function createTable()
{
	hideCreateTable();
	var name = scaliendb.util.elem("createTableName").value;
	name = scaliendb.util.removeSpaces(name);
	var quorumID = scaliendb.util.elem("createTableQuorum").value;
	quorumID = scaliendb.util.removeSpaces(quorumID);
	scaliendb.onResponse = onResponse;
	scaliendb.createTable(createTableDatabaseID, quorumID, name);
}

function renameTable()
{
	hideRenameTable();
	var name = scaliendb.util.elem("renameTableName").value;
	name = scaliendb.util.removeSpaces(name);
	scaliendb.onResponse = onResponse;
	scaliendb.renameTable(renameTableID, name);
}

function truncateTable()
{
	hideTruncateTable();
	scaliendb.onResponse = onResponse;
	scaliendb.truncateTable(truncateTableID);
}

function deleteTable()
{
	hideDeleteTable();
	scaliendb.onResponse = onResponse;
	scaliendb.deleteTable(deleteTableID);
}

function freezeTable(tableID)
{
	scaliendb.onResponse = onResponse;
	scaliendb.freezeTable(tableID);
}

function unfreezeTable(tableID)
{
	scaliendb.onResponse = onResponse;
	scaliendb.unfreezeTable(tableID);
}

function splitShard()
{
	hideSplitShard();
	var key = scaliendb.util.elem("splitShardKey").value;
	scaliendb.onResponse = onResponse;
	scaliendb.splitShard(splitShardID, key);
}

function migrateShard()
{
	hideMigrateShard();
	var quorumID = scaliendb.util.elem("migrateShardQuorum").value;
	quorumID = scaliendb.util.removeSpaces(quorumID);
	scaliendb.onResponse = onResponse;
	scaliendb.migrateShard(migrateShardID, quorumID);
}

function onResponse()
{
	updateConfigState();
}

function onDisconnect()
{
	init();
	createDashboard({});
	clearTimeout(timer);
	timer = setTimeout("connect()", 1000);	
}

function updateConfigState()
{
	scaliendb.getConfigState(onConfigState);
}

var timer;
function onConfigState(configState)
{
	scaliendb.util.elem("controller").textContent = "Connected to " + scaliendb.controller;

	createDashboard(configState);
	
	for (var key in configState)
	{
		if (key == "quorums")
			createQuorumDivs(configState, configState[key]);
		else if (key == "databases")
			createDatabaseDivs(configState, configState[key]);
		else if (key == "shardServers")
			createMigrationDivs(configState, configState[key]);
	}
	
	scaliendb.util.elem("clusterState").textContent = "The ScalienDB cluster is " + scaliendb.getClusterState(configState);
	scaliendb.util.elem("clusterState").className = "status-message " + scaliendb.getClusterState(configState);
	
	// clearTimeout(timer);
	// timer = setTimeout("onTimeout()", 1000);
	scaliendb.pollConfigState(onConfigState);
}

function onTimeout()
{
	if (!md)
		updateConfigState();
}

function createDashboard(configState)
{
	var numDatabases = 0;
	var numDatabasesPrefixText = "is ";
	var numDatabasesText = "";
	var numTables = 0;
	var numTablesText = "";
	var maxTables = 0;
	var maxTablesText = "";
	var minTables = 999999999;
	var minTablesText = "";
	var avgTables = 0;
	var avgTablesText = "";
	var numShards = 0;
	var numShardsText = "";
	var maxShards = 0;
	var maxShardsText = "";
	var minShards = 999999999;
	var minShardsText = "";
	var avgShards = 0;
	var avgShardsText = "";
	var numShardServers = 0;
	var numShardServersText = "";
	var numQuorums = 0;
	var numQuorumsText = "";
	var shardServerIDs = new Array();
	var cardinality = scaliendb.util.cardinality;
	for (var key in configState)
	{
		if (key == "quorums")
		{
			var quorums = configState[key];
			numQuorums = quorums.length;
		}
		else if (key == "databases")
		{
			var databases = configState[key];  
			numDatabases = databases.length;
			if (numDatabases == 1)
				numDatabasesPrefixText = "is ";
			else
				numDatabasesPrefixText = "are ";
			for (var database in databases)
			{
				var db = databases[database];
				num = db["tables"].length;
				if (num < minTables)
					minTables = num;
				if (num > maxTables)
					maxTables = num;
			}
		}
		else if (key == "tables")
		{
			var tables = configState[key];
			numTables = tables.length;
			numTablesText = cardinality(numTables, "table");
			for (var table in tables)
			{
				var tbl = tables[table];
				var num = tbl["shards"].length;
				if (num < minShards)
					minShards = num;
				if (num > maxShards)
					maxShards = num;
			}
		}
		else if (key == "shards")
		{
			var shards = configState[key]; 
			numShards = shards.length;
		}
		else if (key == "shardServers")
		{
			var shardServers = configState[key];   
			numShardServers = shardServers.length;
			for (var shardServer in shardServers)
			{
				var ss = shardServers[shardServer];
				shardServerIDs.push(ss["nodeID"]);
			}
		}
	}
	numQuorumsText = cardinality(numQuorums, "quorum");
	numDatabasesText = cardinality(numDatabases, "database");
	minTablesText = cardinality(minTables, "table");
	maxTablesText = cardinality(maxTables, "table");
	minShardsText = cardinality(minShards, "shard");
	maxShardsText = cardinality(maxShards, "shard");
	numShardsText = cardinality(numShards, "shard");
	numShardServersText = cardinality(numShardServers, "shard server");
	avgTables = Math.round(numTables / numDatabases * 10) / 10;
	avgTablesText = cardinality(avgTables, "table");
	avgShards = Math.round(numShards / numTables * 10) / 10;
	avgShardsText = cardinality(avgShards, "shard")
	
	scaliendb.util.elem("numDatabasesPrefix").textContent = numDatabasesPrefixText;
	scaliendb.util.elem("numDatabases").textContent = numDatabasesText;
	scaliendb.util.elem("numTables").textContent = numTablesText;
	scaliendb.util.elem("numShards").textContent = numShardsText;
	scaliendb.util.elem("numQuorums").textContent = numQuorumsText;
	scaliendb.util.elem("numShardServers").textContent = numShardServersText;
	

	scaliendb.util.elem("minTables").textContent = minTablesText;
	scaliendb.util.elem("maxTables").textContent = maxTablesText;
	scaliendb.util.elem("avgTables").textContent = avgTablesText;

	scaliendb.util.elem("minShards").textContent = minShardsText;
	scaliendb.util.elem("maxShards").textContent = maxShardsText;
	scaliendb.util.elem("avgShards").textContent = avgShardsText;
	
	if (numDatabases == 0 || numTables == 0)
		scaliendb.util.elem("dashboardStats").style.display = "none";
	else
		scaliendb.util.elem("dashboardStats").style.display = "inline";
	
	scaliendb.util.clear(scaliendb.util.elem("shardservers"));
	
	var html = '';
	for (var i in shardServerIDs)
	{
		nodeID = shardServerIDs[i];
		shardServer = scaliendb.getShardServer(configState, nodeID);
		if (shardServer["hasHeartbeat"])
			html += ' <span class="healthy shardserver-number">' + nodeID + '</span> ';
		else
			html += ' <span class="no-heartbeat shardserver-number">' + nodeID + '</span> ';
	}
	scaliendb.util.elem("shardservers").innerHTML = html;
}

function createQuorumDivs(configState, quorums)
{
	scaliendb.util.removeElement("quorums");
	var quorumsDiv = document.createElement("div");
	quorumsDiv.setAttribute("id", "quorums");
	
	for (var quorum in quorums)
	{
		var q = quorums[quorum];
		quorumDiv = createQuorumDiv(configState, q);
		quorumsDiv.appendChild(quorumDiv);
	}
	
	scaliendb.util.elem("tabPageQuorums").appendChild(quorumsDiv);
}

function createQuorumDiv(configState, quorum)
{
	var state;
	var primaryID = null;
	explanation = "";
	if (quorum["hasPrimary"] == "true")
	{
		if (quorum["inactiveNodes"].length == 0)
			state = "healthy";
		else
			state = "unhealthy";
		primaryID = quorum["primaryID"];
	}
	else
		state = "critical";
	
	var paxosID = "unknown";
	if (quorum["paxosID"] > 0)
		paxosID = quorum["paxosID"];
	
	var html =
	'																									\
	<table class="quorum ' + state + '">																\
		<tr>																							\
			<td class="quorum-head">																	\
				<span class="quorum-head">quorum ' + quorum["quorumID"] + '<br/>(' + scaliendb.getQuorumState(configState, quorum["quorumID"]) + ')</span>						\
			</td>																						\
			<td>																						\
				Shardservers: 																			\
	';
	quorum["activeNodes"].sort();
	for (var i in quorum["activeNodes"])
	{
		var nodeID = quorum["activeNodes"][i];
		var shardServer = scaliendb.getShardServer(configState, nodeID);
		var quorumInfo = scaliendb.getQuorumInfo(configState, nodeID, quorum["quorumID"]);
		var infoText = "";
		var catchupText = "";
		if (quorumInfo != null)
		{
			infoText += " [" + quorumInfo["paxosID"] + "]";
			if (quorumInfo["isSendingCatchup"])
				catchupText += "Shard server " + nodeID + " is sending catchup: " + scaliendb.util.humanBytes(quorumInfo["catchupBytesSent"]) + "/" + scaliendb.util.humanBytes(quorumInfo["catchupBytesTotal"]) + " (" + scaliendb.util.humanBytes(quorumInfo["catchupThroughput"]) + "/s)";
		}
		if (nodeID == primaryID)
		{
		 	html += ' <span class="shardserver-number ' + (shardServer["hasHeartbeat"] ? "healthy" : "no-heartbeat") + '"><b>Primary: ' + nodeID + infoText + '</b></span> ';
			explanation = "The quorum has a primary (" + primaryID + "), it is writable. ";
		}
		else
		{
	 		html += ' <span class="shardserver-number ' + (shardServer["hasHeartbeat"] ? "healthy" : "no-heartbeat") + '">' + nodeID + infoText + '</span> ';
		}
	}
	if (primaryID == null)
		explanation += "The quorum has no primary, it is not writable. ";
	quorum["inactiveNodes"].sort();
	for (var i in quorum["inactiveNodes"])
	{
		if (primaryID != null)
			explanation += "The quorum has inactive nodes. These can be brought back into the quorum (once they are up and running and catchup is complete) by clicking them above. ";
		var nodeID = quorum["inactiveNodes"][i];
		var shardServer = scaliendb.getShardServer(configState, nodeID);
		var quorumInfo = scaliendb.getQuorumInfo(configState, nodeID, quorum["quorumID"]);
		var infoText = "";
		if (quorumInfo != null)
			infoText += " [" + quorumInfo["paxosID"] + "]";
		if (shardServer["hasHeartbeat"] && primaryID != null)
			html += ' <a class="no-line" style="color:black" title="Activate shard server" href="javascript:activateNode(' + quorum["quorumID"] + ", " + nodeID + ')"><span class="shardserver-number healthy">' + nodeID + infoText + ' (click to activate)</span></a> ';
		else
			html += ' <span class="shardserver-number ' + (shardServer["hasHeartbeat"] ? "healthy" : "no-heartbeat") + '">' + nodeID + infoText + '</span> ';
	}
	html +=
	'			<br/>																					\
				Shards: 																				\
	';
	quorum["shards"].sort();
	for (var i in quorum["shards"])
	{
		shardID = quorum["shards"][i]
		html += ' <span class="shard-number">' + shardID + '</span> ';
	}
	html +=
	'																									\
				<br/>																					\
				Replication round: ' + paxosID + '<br/>													\
				<!-- Size: 0GB -->																		\
				<div class="explanation">Explanation: ' + explanation + '<br/>' + catchupText + '</span>						\
			</td>																						\
			<td class="table-actions">																	\
				<a class="no-line" href="javascript:showAddNode(' + quorum["quorumID"] +
				')"><span class="create-button">add server</span></a><br/><br/>							\
				<a class="no-line" href="javascript:showRemoveNode(' + quorum["quorumID"] +
				')"><span class="modify-button">remove server</span></a><br/><br/>						\
				<a class="no-line" href="javascript:showDeleteQuorum(' + quorum["quorumID"] +
				')"><span class="delete-button">delete quorum</span></a>								\
			</td>																						\
		</tr>																							\
	</table>																							\
	';
	
	var div = document.createElement("div");
	div.setAttribute("class", "quorum " + state);
	div.innerHTML = html;
	return div;
}

function createDatabaseDivs(configState, databases)
{
	scaliendb.util.removeElement("databases");
	var databasesDiv = document.createElement("div");
	databasesDiv.setAttribute("id", "databases");
	
	for (var database in databases)
	{
		var db = databases[database];
		databaseDiv = createDatabaseDiv(configState, db);
		databasesDiv.appendChild(databaseDiv);
	}
	
	scaliendb.util.elem("tabPageSchema").appendChild(databasesDiv);
}

function createDatabaseDiv(configState, database)
{
	var html =
	'																									\
	<span class="database-head">Listing tables for database: <span class="database-name">' + database["name"] + '</span></span>		\
	 - <a class="no-line" href="javascript:showDeleteDatabase(\'' + database["databaseID"] + '\')">		\
	<span class="delete-button">delete database</span></a>												\
	 - <a class="no-line" href="javascript:showRenameDatabase(\'' +
	database["databaseID"] + '\', \'' + database["name"] + '\')">										\
	<span class="modify-button">rename database</span></a>												\
	 - <a class="no-line" href="javascript:showCreateTable(\''
	 + database["databaseID"] + '\', \'' + database["name"] + '\')">									\
	<span class="create-button">create new table</span></a><br/><br/>									\
	';
	var div = document.createElement("div");
	div.setAttribute("class", "database");
	div.innerHTML = html;
	
	for (var i in database["tables"])
	{
		tableID = database["tables"][i];
		table = locateTable(configState, tableID);
		if (table == null)
			continue;
		var n = createTableDiv(configState, table);
		div.appendChild(n);
	}
	
	return div;
}

function createTableDiv(configState, table)
{
	var html =
	'																									\
		<table class="table">																			\
			<tr>																						\
				<td class="table-head">																	\
					<span class="table-head">' + table["name"] + '</span>								\
				</td>																					\
				<td>																					\
					Table number: ' + table["tableID"] + '<br/>											\
					Shards: 																			\
	';
	
	var quorumIDs = new Array();
	var rfactor = 0;
	var size = 0;
	table["shards"].sort();
	for (var i in table["shards"])
	{
		var shardID = table["shards"][i];
		var shard = locateShard(configState, shardID);
		if (shard == null)
			continue;
		size += shard["shardSize"];
		html += ' <span class="shard-number ' + scaliendb.getQuorumState(configState, shard["quorumID"]) + '">' + shardID + '</span> ';
		quorumID = shard["quorumID"];
		if (!contains(quorumIDs, quorumID))
			quorumIDs.push(quorumID);
		var quorum = locateQuorum(configState, quorumID);
		if (quorum == null)
			continue;
		if (quorum["activeNodes"].length > rfactor)
			rfactor = quorum["activeNodes"].length;
	}
	
	html +=
	'				<br/>																				\
					Mapped quorums: 																	\
	';
	quorumIDs.sort();
	for (var i in quorumIDs)
	{
			var quorumID = quorumIDs[i];
			html += '<span class="quorum-number ' + scaliendb.getQuorumState(configState, quorumID) + '">' + quorumID + '</span> ';
	}
	
	html += 
	'																									\
					<br/>																				\
					Replication factor: ' + rfactor + '<br/>											\
					Shard splitting frozen: ' + (table["isFrozen"] ? "yes" : "no") + '<br/>				\
					Size: ' + scaliendb.util.humanBytes(size) + '<br/><br/>												\
					<a class="no-line" href="javascript:showhideShardsDiv(\'' +
					table["tableID"] +  '\')">															\
					<span id="showhideShardsButton_' + table["tableID"] + '" class="modify-button">' + 
					(tableShardsVisible[tableID] ? 'hide shards' : 'show shards') + '</span></a>			\
				</td>																					\
				<td class="table-actions">																\
					<a class="no-line" href="javascript:showRenameTable(\'' +
					table["tableID"] + '\', \'' + table["name"] +  '\')">								\
					<span class="modify-button">rename table</span></a><br/><br/>						\
					<a class="no-line" href="javascript:showTruncateTable(\'' +
					table["tableID"] +  '\')">															\
					<span class="delete-button">truncate table</span></a><br/><br/>						\
					<a class="no-line" href="javascript:showDeleteTable(\'' +
					table["tableID"] +  '\')">															\
					<span class="delete-button">delete table</span>										\
					<a class="no-line" href="javascript:' + (table["isFrozen"] ? "unfreeze" : "freeze" ) + 'Table(\'' +
					table["tableID"] + '\')"><br/><br/>													\
					<span class="modify-button">' + (table["isFrozen"] ? "unfreeze" : "freeze" ) + ' table</span></a><br/><br/>						\
				</td>																					\
			</tr>																						\
		</table>																						\
	';
	var div = document.createElement("div");
	div.setAttribute("class", "table " + scaliendb.getTableState(configState, table["tableID"]));
	div.innerHTML = html;
	
	var shardsDiv = document.createElement("div");
	shardsDiv.setAttribute("id", "shards_" + table["tableID"]);
	if (tableShardsVisible[tableID])
		shardsDiv.setAttribute("style", "display:block");
	else
		shardsDiv.setAttribute("style", "display:none");
	
	for (var i in table["shards"])
	{
		shardID = table["shards"][i];
		shard = locateShard(configState, shardID);
		if (shard == null)
			continue;
		var shardDiv = createShardDiv(configState, shard);
		shardsDiv.appendChild(shardDiv);
	}
	
	div.appendChild(shardsDiv);
	
	return div;
}

function createShardDiv(configState, shard)
{
	var html =
	'																									\
		<table class="shard">																			\
			<tr>																						\
				<td class="shard-head">																	\
					<span class="shard-head">Shard ' + shard["shardID"] + '</span>						\
				</td>																					\
				<td>																					\
	';

	
	html += 
	'																									\
					Quorum: <span class="quorum-number ' + scaliendb.getQuorumState(configState, shard["quorumID"]) + '">' + shard["quorumID"] + '</span> (' + scaliendb.getQuorumState(configState, shard["quorumID"]) + ')<br/>												\
					Start key: ' + (shard["firstKey"] == "" ? "(empty)" : shard["firstKey"]) + '<br/>							  		  		\
					End key: ' + (shard["lastKey"] == "" ? "(empty)" : shard["lastKey"]) + '<br/>												\
					Splitable: ' + (shard["isSplitable"] ? "yes" : "no") + '<br/>';
					if (shard["isSplitable"])
						html +=  'Split key: ' + (scaliendb.util.defstr(shard["splitKey"]) == "" ? "(empty)" : scaliendb.util.defstr(shard["splitKey"])) + '<br/>';
					html += '																			\
					Size: ' + scaliendb.util.humanBytes(shard["shardSize"]) + '<br/>	  				\
				</td>																					\
				<td class="shard-actions">																\
					<a class="no-line" href="javascript:showSplitShard(\'' +
					shard["shardID"] + '\')">								\
					<span class="modify-button">split shard</span></a><br/><br/>						\
					<a class="no-line" href="javascript:showMigrateShard(\'' +
					shard["shardID"] +  '\')">															\
					<span class="modify-button">migrate shard</span></a><br/><br/>						\
				</td>																					\
			</tr>																						\
		</table>																						\
	';
	
	var div = document.createElement("div");
	div.setAttribute("class", "shard " + scaliendb.getQuorumState(configState, shard["quorumID"]));
	div.innerHTML = html;
	return div;
}

function createMigrationDivs(configState, shardServers)
{
	scaliendb.util.removeElement("migrations");
	var migrationsDiv = document.createElement("div");
	migrationsDiv.setAttribute("id", "migrations");
	
	var count = 0;
	for (var i in shardServers)
	{
		var shardServer = shardServers[i];
		for (var j in shardServer["quorumShardInfos"])
		{
			var quorumShardInfo = shardServer["quorumShardInfos"][j];
			if (quorumShardInfo["isSendingShard"])
			{
				migrationsDiv.appendChild(createMigrationDiv(configState, shardServer, quorumShardInfo));
				count++;
			}
		}
	}
	
	scaliendb.util.elem("tabPageMigration").appendChild(migrationsDiv);
	scaliendb.util.elem("tabHeadMigration").innerHTML = "Shard migration";
	if (count > 0)
		scaliendb.util.elem("tabHeadMigration").innerHTML += " (" + count + ")";
}

function createMigrationDiv(configState, shardServer, quorumShardInfo)
{
	var shard = locateShard(configState, quorumShardInfo["shardID"]);
	var html =
	'																									\
	<table class="migration healthy">																	\
		<tr>																							\
			<td class="migration-head">																	\
				<span class="migration-head">shard ' + quorumShardInfo["shardID"] + ' </span>			\
			</td>																						\
			<td>																						\
				Shard servers: <span class="shardserver-number healthy">' +  shardServer["nodeID"] + '</span> ' + ' &rarr; <span class="shardserver-number healthy">' + quorumShardInfo["migrationNodeID"] +  '</span><br/>								\
				Quorum: <span class="quorum-number healthy">' + shard["quorumID"] + '</span> &rarr; <span class="quorum-number healthy">' + quorumShardInfo["migrationQuorumID"] + '</span><br/>						\
				Progress: ' + scaliendb.util.humanBytes(quorumShardInfo["migrationBytesSent"]) + '/' + scaliendb.util.humanBytes(quorumShardInfo["migrationBytesTotal"]) + ' (' + scaliendb.util.humanBytes(quorumShardInfo["migrationThroughput"]) + '/s)	\
			</td>																						\
		</tr>																							\
	</table>																							\
	';

	var div = document.createElement("div");
	div.setAttribute("class", "migration healthy");
	div.innerHTML = html;
	return div;
}

function locateTable(configState, tableID)
{
	for (var key in configState)
	{
		if (key == "tables")
		{
			tables = configState[key];
			for (var table in tables)
			{
				tbl = tables[table];
				if (tbl["tableID"] == tableID)
					return tbl;
			}
		}
	}
	
	return null;
}

function locateShard(configState, shardID)
{
	for (var key in configState)
	{
		if (key == "shards")
		{
			shards = configState[key];
			for (var shard in shards)
			{
				shd = shards[shard];
				if (shd["shardID"] == shardID)
					return shd;
			}
		}
	}
	
	return null;
}

function locateQuorum(configState, quorumID)
{
	for (var key in configState)
	{
		if (key == "quorums")
		{
			quorums = configState[key];
			for (var quorum in quorums)
			{
				q = quorums[quorum];
				if (q["quorumID"] == quorumID)
					return q;
			}
		}
	}
	
	return null;
}

function consoleEvalExpression()
{
	var consoleForm = scaliendb.util.elem("console-form");
	var cmd = consoleForm.input.value;
	consoleForm.output.value += "Executing " + cmd + "\n";
	eval(cmd);
}

function consoleOnKeyDown(e)
{
	if ((window.event && window.event.keyCode == 13) ||
	    (e && e.keyCode == 13))
	{
		alert("Enter pressed");
	}
}

function removeOutline()
{
	var links = document.getElementsByTagName("a");
 	for( i=0; i < links.length; i++)
 	{
 		links[i].onfocus = links[i].blur;
 	}
}

function getKeycode(e)
{
	if (e == null)
		return event.keyCode;
	else
		return e.which;
}
