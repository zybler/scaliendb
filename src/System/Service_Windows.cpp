#ifdef PLATFORM_WINDOWS
#include "System/Events/EventLoop.h"
#include "Service.h"
#include <windows.h>

static SERVICE_STATUS_HANDLE    serviceHandle;
static Callable                 serviceCallback;
static Buffer                   serviceName;
static Buffer                   serviceDescription;
static Buffer                   displayName;

extern bool IsController();

static void ReportServiceStatus(DWORD state)
{
    SERVICE_STATUS  serviceStatus;
    DWORD           controlsAccepted;

    if (serviceHandle == NULL)
        return;

    switch (state)
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:
    case SERVICE_STOPPED:
        controlsAccepted = 0;
        break;
    default:
        controlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        break;
    }

    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwControlsAccepted = controlsAccepted;
    serviceStatus.dwCurrentState = state;
    serviceStatus.dwWin32ExitCode = NO_ERROR;
    serviceStatus.dwWaitHint = 0;
    serviceStatus.dwCheckPoint = 0;

    if (!SetServiceStatus(serviceHandle, &serviceStatus))
    {
        Log_Errno();
    }
}

static void WINAPI ServiceCtrlHandler(DWORD fdwControl)
{
    Callable    empty;

    switch (fdwControl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        Log_Message("Service shutting down...");
        ReportServiceStatus(SERVICE_STOP_PENDING);
        EventLoop::Stop();
        // wake up IOProcessor
        IOProcessor::Complete(&empty);
        ReportServiceStatus(SERVICE_STOPPED);
        return;
    }
}

static void WINAPI InitService(DWORD argc, LPTSTR *argv)
{
    serviceHandle = RegisterServiceCtrlHandler(serviceName.GetBuffer(), ServiceCtrlHandler);
    if (!serviceHandle)
    {
        Log_Errno();
        return;
    }

    ReportServiceStatus(SERVICE_START_PENDING);

    Call(serviceCallback);

    ReportServiceStatus(SERVICE_STOPPED);
}

static bool ServiceGetFullPathname(const char* filename, Buffer& fullPath)
{
    char    buffer[MAX_PATH];
    DWORD   ret;

    ret = GetFullPathName(filename, sizeof(buffer), buffer, NULL);
    if (ret == 0)
        return false;

    fullPath.Clear();
    fullPath.Append('\"');
    for (DWORD i = 0; i < ret; i++)
    {
        if (buffer[i] == '\"')
            fullPath.Append('\\');
        fullPath.Append(buffer[i]);
    }
    fullPath.Append('\"');
    return true;
}

static void SetupNamesAndDescriptions()
{
    if (IsController())
    {
        serviceName.Write("ScalienController");
        displayName.Write("Scalien Database Controller");
        serviceDescription.Write("Scalien Database Controller");
    }
    else
    {
        serviceName.Write("ScalienShardServer");
        displayName.Write("Scalien Database Shard Server");
        serviceDescription.Write("Scalien Database Shard Server");
    }
    serviceName.NullTerminate();
    displayName.NullTerminate();
    serviceDescription.NullTerminate();
}

static bool UninstallService()
{
    // set up names and descriptions
    SetupNamesAndDescriptions();

    // connect to service control manager on the local machine
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scManager == NULL)
    {
        Log_Errno();
        return false;
    }

    // check if the service already exists
    SC_HANDLE scService = OpenService(scManager, serviceName.GetBuffer(), SERVICE_ALL_ACCESS);
    if (scService == NULL)
    {

        Log_Errno("Cannot find service: %s", serviceName.GetBuffer());
        CloseServiceHandle(scService);
        CloseServiceHandle(scManager);
        return false;
    }

    // stop the service
    SERVICE_STATUS serviceStatus;
    if (ControlService(scService, SERVICE_CONTROL_STOP, &serviceStatus))
    {
        Log_Message("Stopping service...");
        while (QueryServiceStatus(scService, &serviceStatus))
        {
            if (serviceStatus.dwCurrentState == SERVICE_STOP_PENDING)
                Sleep(1000);
            else
                break;
        }
    }

    Log_Message("Deleting service...");
    BOOL ret = DeleteService(scService);
    if (ret == 0)
        Log_Errno();

    return ret > 0;
}

static bool InstallService(int argc, char* argv[])
{
    Buffer  cmd;
    Buffer  params;
    Buffer  arg;
    Buffer  fullPath;
    bool    reinstall;

    reinstall = false;

    if (!ServiceGetFullPathname(argv[0], fullPath))
        return false;

    // make command line for service
    cmd.Write(fullPath);
    for (int i = 1; i < argc; i++ )
    {
        arg.Write(argv[i]);
        
        // change install parameter to service on command line
        if (arg.Cmp("--install") == 0 || arg.Cmp("--reinstall") == 0)
        {
            if (arg.Cmp("--reinstall") == 0)
                reinstall = true;
            arg.Write("--service");
        }

        if (!arg.BeginsWith("-"))
        {
            // prefix config file with full path
            if (!ServiceGetFullPathname(argv[i], fullPath))
                return false;

            arg.Write(fullPath);
        }

        cmd.Appendf(" %B", &arg);
    }
    cmd.NullTerminate();

    if (reinstall)
        UninstallService();

    // set up names and descriptions
    SetupNamesAndDescriptions();

    // connect to service control manager on the local machine
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scManager == NULL)
    {
        Log_Errno();
        return false;
    }

    // check if the service already exists
    SC_HANDLE scService = OpenService(scManager, serviceName.GetBuffer(), SERVICE_ALL_ACCESS);
    if (scService != NULL && !reinstall)
    {

        Log_Errno("Service already exists: %s", serviceName.GetBuffer());
        CloseServiceHandle(scService);
        CloseServiceHandle(scManager);
        return false;
    }
    
    if (!reinstall)
    {
        // create new service
        scService = CreateService(scManager, serviceName.GetBuffer(), displayName.GetBuffer(),
                      SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                      SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                      cmd.GetBuffer(), NULL, NULL, "\0\0", NULL, NULL);

        if (scService == NULL)
        {
            Log_Errno("Cannot create service: %s", serviceName.GetBuffer());
            CloseServiceHandle(scManager);
            return false;
        }
    }

    Log_Message("Service created.");
    Log_Message("Service can be started from the command line via 'net start \"%s\"", serviceName.GetBuffer());

    // set the service description
    SERVICE_DESCRIPTION description;
    description.lpDescription = (LPTSTR)serviceDescription.GetBuffer();
    if (!ChangeServiceConfig2(scService, SERVICE_CONFIG_DESCRIPTION, &description))
    {
        Log_Errno("Could not set service description");
        CloseServiceHandle(scService);
        CloseServiceHandle(scManager);
        return false;
    }

    // set service recovery options
    SC_ACTION aActions[ 3 ] = {{SC_ACTION_RESTART, 0}, {SC_ACTION_RESTART, 0}, {SC_ACTION_RESTART, 0}};
    SERVICE_FAILURE_ACTIONS serviceFailure;
    ZeroMemory(&serviceFailure, sizeof(SERVICE_FAILURE_ACTIONS));
    serviceFailure.cActions = 3;
    serviceFailure.lpsaActions = aActions;

    if (!ChangeServiceConfig2(scService, SERVICE_CONFIG_FAILURE_ACTIONS, &serviceFailure))
    {
        Log_Errno("Could not set service recovery options");
        CloseServiceHandle(scService);
        CloseServiceHandle(scManager);
        return false;
    }

    CloseServiceHandle(scService);
    CloseServiceHandle(scManager);

    return true;
}

static bool RunService()
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPTSTR)serviceName.GetBuffer(), InitService },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(serviceTable) == 0)
    {
        Log_Errno("Could not start service");
        return false;
    }

    return true;
}

bool Service::Main(int argc, char **argv, void (*serviceFunc)())
{
    Buffer  lastArg;

    serviceCallback = CFunc(serviceFunc);

    lastArg.Write(argv[argc - 1]);
    if (lastArg.Cmp("--install") == 0 || lastArg.Cmp("--reinstall") == 0)
        return InstallService(argc, argv);

    if (lastArg.Cmp("--uninstall") == 0)
        return UninstallService();

    if (lastArg.Cmp("--service") == 0)
        return RunService();

    serviceFunc();
    return true;
}

void Service::SetStatus(unsigned status)
{
    ReportServiceStatus(status);
}

#endif // PLATFORM_WINDOWS
