import sys
import scaliendb
import inspect
import time

# enable syntax completion
try:
    import readline
except ImportError:
    print("Module readline not available.")
else:
    import rlcompleter
    if sys.platform == "darwin":
        # this works on Snow Leopard
        readline.parse_and_bind ("bind ^I rl_complete")
    else:
        # this works on Linux    
        readline.parse_and_bind("tab: complete")

# try importing other useful modules
def try_import(name):
    try:
        globals()[name] = __import__(name)
        print("Module " + name + " loaded.")
    except:
        pass

print("")
try_import("json")
del try_import

def closure(func, *args):
    return lambda: func(*args)

# helper function for measure timing
def timer(func, *args):
    starttime = time.time()
    ret = func(*args)
    endtime = time.time()
    print("Function took " + str(endtime-starttime) + " secs")
    return ret


# helper function for other connections
def connect(nodes, database=None, table=None):
    client = scaliendb.Client(nodes)    
    # import client's member function to the global scope
    members = inspect.getmembers(client, inspect.ismethod)
    for k, v in members:
        if k[0] != "_":
            globals()[k] = v
    if database == None:
        return
    client.use_database(database)
    if table == None:
        return
    client.use_table(table)

# helper class for implementing 'shelp' command
class SHelp:
    def __repr__(self):
        output = "Use connect(nodes, database, table) to connect to the cluster\n"
        output += str(quit)
        return output


# helper function for welcome message
def welcome():
    header = "ScalienDB shell " + scaliendb.get_version()
    line = "".join(["=" for x in xrange(len(header))])
    print("\n" + line + "\n" + header + "\n" + line + "\n")
    print("This is a standard Python shell, enhanced with ScalienDB client library commands.")
    print("Type \"shelp\" for help.\n")

welcome()
del welcome

# register 'shelp' command
shelp = SHelp()

# create default client
connect("localhost:7080")
