#include "Client.h"
#include "Liba.h"
#include "Write.h"
#include "Read.h"
#include "CommsMail.h"
#include <TlHelp32.h>
#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QThread>
#include <array>
#include <iostream>
#include <sstream>

Client::Client(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connectToServ();
}

Client::~Client()
{
	Write reader(comms);
	reader.writeUInt8((uint8_t)CMDCODE::CLIENTSHUTDOWN);
}

void Client::on_Scan_clicked()
{
	QThread* scanThread = QThread::create(&Client::requestScan, this);
	scanThread->start();
}

void Client::requestScan() 
{
	Write writer(comms);
	Read reader(comms);
}

void Client::connectToServ()
{
	serverWakeUp();

	comms = Comms::Mailslots(u"\\\\.\\mailslot\\client", u"\\\\.\\mailslot\\server");

	comms->connect();
}

void Client::serverWakeUp() 
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcscmp(entry.szExeFile, L"Server.exe") == 0)
			{
				// if Server.exe is running, don't wakeup
				CloseHandle(snapshot);
				return;
			}
		}
	}

	CloseHandle(snapshot);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	wchar_t path[256] = L"Server.exe";
	if (!CreateProcess(NULL,
		path,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)
		)
	{
		HRESULT error = GetLastError();
		return;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

