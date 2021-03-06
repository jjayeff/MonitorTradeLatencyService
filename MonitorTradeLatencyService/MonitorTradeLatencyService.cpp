// MonitorTradeLatencyService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "stdafx.h"
#include <plog/vnlog.h>

#include "processor.h"

Processor		processor;
Configuration	config;

//---Pre-paire service -----------------------------------------
SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME  _T("MonitorTradeLatencyService")    
//---Pre-paire service -----------------------------------------

int _tmain(int argc, TCHAR *argv[])
{
	//------- Setting configuration
	config.setConfig("MonitorTradeLatencyService.ini");
	string appPath = config.getAbsolutePath();
	string log_path = appPath + "logs";
	string result_path = appPath + "results\\";
	processor.tmp_results_path = appPath + "\\results";

	config.setValue("Application", "KeyFrontName", "D0118__FIX__MD1");
	config.setValue("Application", "KeyBackName", "SET");
	config.setValue("Application", "LogPath", log_path);
	config.setValue("Application", "FilePath", log_path);
	config.setValue("Application", "ResultPath", result_path);
	config.setValue("Application", "Diff", "1");
	config.setValue("Application", "Deley", "300");
	config.setValue("Application", "Account", "D0013__FIX__CU3, D0019__FIX__CU2, D0025__FIX__CU3, D0032__FIX__CU3, D0034__FIX__CU5, D0060__FIX__CU2, D0063__FIX__CU2, D0117__FIX__CU1, D0118__FIX__CU1,");
	config.setValue("Application", "Date", "20190205");

	config.setValue("Database", "Driver", "SQL Server Native Client 11.0");
	config.setValue("Database", "Server", "172.17.1.43");
	config.setValue("Database", "Database", "acc_info");
	config.setValue("Database", "Username", "sa");
	config.setValue("Database", "Password", "P@ssw0rd");
	config.setValue("Database", "LogName", "MonitorTraderLatenceService");

	config.setValue("Email", "From", "infrastructure@toptrader.co.th");
	config.setValue("Email", "To", "infrastructure@toptrader.co.th");
	config.setValue("Email", "Template", "tt_monitor_trade_latency_service.html");

	processor.file_path = config.getValueString("Application", "FilePath");
	processor.result_path = config.getValueString("Application", "ResultPath");
	processor.key_front_name = config.getValueString("Application", "KeyFrontName");
	processor.key_back_name = config.getValueString("Application", "KeyBackName");
	processor.diff = config.getValueInt("Application", "Diff");
	processor.deley = config.getValueInt("Application", "Deley");
	processor.CutString(config.getValueString("Application", "Account"));
	processor.date_config = config.getValueInt("Application", "Date");
	processor.db_driver = config.getValueString("Database", "Driver");
	processor.db_server = processor.GetIpByName(config.getValueString("Database", "Server"));

	processor.db_user = config.getValueString("Database", "Username");
	processor.db_password = config.getValueString("Database", "Password");
	processor.db_logname = config.getValueString("Database", "LogName");
	processor.email_from = config.getValueString("Email", "From");
	processor.email_to = config.getValueString("Email", "To");
	processor.email_template = config.getValueString("Email", "Template");
	// init log
	processor.vnLog.InitialLog(config.getValueString("Application", "LogPath"), "MonitorTradeLatencyService", 10, true);
	//----------------------------------------------------------------------

	/*while (1) {
		processor.Run();
		Sleep(processor.deley * 1000);
	}*/

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		return GetLastError();
	}

	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	LOGI << "Entry";

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		LOGE << "RegisterServiceCtrlHandler returned error";
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LOGE << "SetServiceStatus returned error";
	}

	/*
	* Perform tasks unnecessary to start the service here
	*/
	LOGI << "Performing Service Start Operations";

	//Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		LOGI << "CreateEvent(g_ServiceStopEvent) returned error";

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			LOGE << "SetServiceStatus returned error";
		}
		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LOGE << "SetServiceStatus returned error";
	}

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	LOGI << "Waiting for Worker Thread to complete";

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	if (hThread != NULL)
		WaitForSingleObject(hThread, INFINITE);

	LOGI << "Worker Thread Stop Event signaled";

	/*
	* Perform any cleanup tasks
	*/
	LOGI << "Performing Cleanup Operations";

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LOGE << "SetServiceStatus returned error";
	}

EXIT:
	LOGI << "Exit";

	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	//LOGI << "Entry";

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		LOGI << "SERVICE_CONTROL_STOP Request";

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks unnecessary to stop the service here
		*/

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			LOGE << "SetServiceStatus returned error";
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}

	//LOGI << "Exit";
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	try
	{
		LOGI << "Entry";

		//  Periodically check if the service has been requested to stop
		while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
		{
			processor.Run();
			Sleep(processor.deley * 1000);
		}

	}
	catch (const exception& e)
	{
		LOGE << "!Exception " << e.what();
	}

	LOGI << "Exit";
	return ERROR_SUCCESS;
}