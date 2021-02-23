#include "Client.h"
#include "Liba.h"
#include <QString>
#include <QThread>
#include <array>
#include <iostream>
#include <sstream>

Client::Client(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(this, &Client::output, ui.textEdit, &QTextEdit::append);
	connectToServer();
}

Client::~Client()
{
	ResetEvent(clientUp);
	Liba::Disconnect(hServer);
	Liba::Disconnect(hClient);
}

void Client::connectToServer()
{
	hClient = Liba::CreateSlot(u"\\\\.\\mailslot\\client");
	hServer = Liba::ConnectToSlot(u"\\\\.\\mailslot\\server");
	if (Liba::IsInvalid(hServer))
	{
		wakeUpServer();

		while (true)
		{
			hServer = Liba::ConnectToSlot(u"\\\\.\\mailslot\\server");

			if (!Liba::IsInvalid(hServer))
				break;

			Sleep(10);
		}
	}
	clientUp = OpenEvent(EVENT_ALL_ACCESS, NULL, TEXT("ClientUpEvent"));
	SetEvent(clientUp);
}

void Client::sendRequest()
{
	writeToServer();
	readFromServer();
}

void Client::on_pushButton_clicked()
{
	QThread* thread = QThread::create(&Client::TestRequest, this);
	thread->start();
}

void Client::TestRequest()
{
	// TEST DATA
	int32_t testNumber = 12345;

	std::u16string testString = u"A Few Words";

	// send to server
	if (!Liba::IsInvalid(hServer))
	{
		QString report = QString::fromUtf16(u"Отправлено:\n");
		report += QString::number(testNumber) + "\n";
		report += QString::fromUtf16(testString.c_str()) + "\n";

		output(report);

		Liba::WriteInt8(hServer, (int8_t)CMDCODE::TEST);
		Liba::WriteInt32(hServer, testNumber);
		Liba::WriteU16String(hServer, testString);
	}

	if (!Liba::IsInvalid(hClient))
	{
		// read echo from server
		int32_t testNumberResponse = Liba::ReadInt32(hClient);
		std::u16string testStringResponse = Liba::ReadU16String(hClient);


		// output

		QString report = QString::fromUtf16(u"Получено:\n");
		report += QString::number(testNumberResponse) + "\n";
		report += QString::fromUtf16(testStringResponse.c_str()) + "\n";

		output(report);
	}
}

void Client::wakeUpServer()
{
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

void Client::readFromServer() {

}

void Client::writeToServer() {

}