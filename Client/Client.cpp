#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QThread>
#include "Client.h"
#include <Liba.h>
#include <CommsMail.h>
#include <Read.h>
#include <Write.h>
#include <TlHelp32.h>
#include <QTime>
#include <QProgressBar>
#include "FileDialogue.h"

#define SVCNAME TEXT("Anti-virus service")

Client::Client(QWidget* parent) : QMainWindow(parent) {
	ui.setupUi(this);
	ui.progressBar->hide();
	QHeaderView* monitorLogHeaderView = ui.monitorLog->horizontalHeader();
	monitorLogHeaderView->setSectionResizeMode(0, QHeaderView::Stretch);
	QHeaderView* timmerLogHeaderView = ui.timmerLog->horizontalHeader();
	timmerLogHeaderView->setSectionResizeMode(0, QHeaderView::Stretch);
	connect(this, &Client::logShow, ui.liveLog, &QTextEdit::append);
	connect(this, &Client::setProgressBar, ui.progressBar, &QProgressBar::setValue);
	connect(this, &Client::removeItem, ui.virusLog, &QListWidget::takeItem);
	serverConnect();
	viruses = std::make_unique<Viruses>(u"viruses.gor");
	loadMonitors();
	loadScanners();
}

Client::~Client() {
	Write reader(comms);
	reader.writeUInt8((uint8_t)CMDCODE::CLIENTSHUTDOWN);
}

void Client::on_scanButton_clicked() {
	ui.liveLog->setText("Scan started:");
	ui.progressBar->setValue(0);
	ui.progressBar->show();
	scanThread = QThread::create(&Client::requestScan, this);
	scanThread->start();
}

void Client::on_stopScanButton_clicked() {
	scanThread->terminate();
	scanThread->wait();
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::STOPSCAN);
	Read reader(comms);
	bool success = (bool)reader.readUInt8();
	if (success) {
		logShow("Stopped");
	}
}

void Client::on_targetButton_clicked() {
	FileDialogue* dialog = new FileDialogue(nullptr);
	dialog->setFileMode(QFileDialog::Directory);
	dialog->show();
	if (dialog->exec())
		ui.pathLabel->setText(dialog->selectedFiles()[0]);
}

void Client::on_quitButton_clicked() {
	Write reader(comms);
	reader.writeUInt8((uint8_t)CMDCODE::SERVERSHUTDOWN);
	QCoreApplication::quit();
}

void Client::on_timmerButton_clicked() {
	ui.stackedWidget->setCurrentIndex(3);
}

void Client::on_monitorButton_clicked() {
	ui.stackedWidget->setCurrentIndex(2);
}

void Client::on_logButton_clicked() {
	viruses->load();
	ui.virusLog->clear();
	for (size_t i = 0; i < viruses->size(); i++)
	{
		ui.virusLog->addItem(QString::fromUtf16(viruses->get(i).c_str()));
	}
	ui.stackedWidget->setCurrentIndex(1);
}

void Client::on_returnButton_clicked() {
	ui.stackedWidget->setCurrentIndex(0);
}

void Client::on_returnButton2_clicked() {
	ui.stackedWidget->setCurrentIndex(0);
}

void Client::on_returnButton3_clicked() {
	ui.stackedWidget->setCurrentIndex(0);
}

void Client::on_delButton_clicked() {
	if (ui.virusLog->selectedItems().empty())
		return;
	uint64_t index = ui.virusLog->row(ui.virusLog->selectedItems()[0]);
	QThread* deleteThread = QThread::create(&Client::requestDel, this, index);
	deleteThread->start();
}

void Client::on_monitorStart_clicked() {
	QThread* monitorSetupThread = QThread::create(&Client::setupMonitor, this, ui.monitorPath->text().toStdU16String());
	monitorSetupThread->start();
	ui.monitorLog->setRowCount(ui.monitorLog->rowCount() + 1);
	int lastIndex = ui.monitorLog->rowCount() - 1;
	ui.monitorLog->setItem(lastIndex, 0, new QTableWidgetItem(ui.monitorPath->text()));
}

void Client::on_monitorStop_clicked() {
	if (ui.monitorLog->selectedItems().empty())
		return;
	uint64_t index = ui.monitorLog->row(ui.monitorLog->selectedItems()[0]);
	ui.monitorLog->removeRow(index);
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::STOPMONITOR);
	writer.writeUInt64(index);
}

void Client::on_monitorBrowse_clicked() {
	FileDialogue* dialog = new FileDialogue(nullptr);
	dialog->setFileMode(QFileDialog::Directory);
	dialog->show();
	if (dialog->exec())
		ui.monitorPath->setText(dialog->selectedFiles()[0]);
}

void Client::on_timmerStart_clicked() {
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::TIMESCAN);
	writer.writeU16String(ui.timmerPath->text().toStdU16String());
	uint32_t hours = (uint32_t)(ui.timmerClock->time().hour());
	uint32_t minutes = (uint32_t)(ui.timmerClock->time().minute());
	writer.writeUInt32(hours);
	writer.writeUInt32(minutes);
	ui.timmerLog->setRowCount(ui.timmerLog->rowCount() + 1);
	int lastIndex = ui.timmerLog->rowCount() - 1;
	ui.timmerLog->setItem(lastIndex, 0, new QTableWidgetItem(ui.timmerPath->text()));
	QString time = QString::number(hours) + QString(":") + QString::number(minutes);
	ui.timmerLog->setItem(lastIndex, 1, new QTableWidgetItem(time));
}

void Client::on_timmerStop_clicked() {
	if (ui.timmerLog->selectedItems().empty())
		return;
	uint64_t index = ui.timmerLog->row(ui.timmerLog->selectedItems()[0]);
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::STOPTIMESCAN);
	writer.writeUInt64(index);
	ui.timmerLog->removeRow(index);
}

void Client::on_timmerBrowse_clicked() {
	FileDialogue* dialog = new FileDialogue(nullptr);
	dialog->setFileMode(QFileDialog::Directory);
	dialog->show();
	if (dialog->exec())
		ui.timmerPath->setText(dialog->selectedFiles()[0]);
}

void Client::serverConnect() {
	serverWakeUp();
	comms = Comms::Mailslots(u"\\\\.\\mailslot\\client", u"\\\\.\\mailslot\\server");
	comms->connect();
}

void Client::serverWakeUp() {
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive datadb 
		SC_MANAGER_CONNECT);
	if (NULL == schSCManager) {
		HRESULT error = GetLastError();
		printf("OpenSCManager failed (%d)\n", error);
		return;
	}
	SC_HANDLE schService = OpenService(
		schSCManager,         // SCM datadb 
		SVCNAME,            // name of service 
		SERVICE_START |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);
	if (schService == NULL) {
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	StartService(schService, NULL, NULL);
}

void Client::requestScan() {
	Write writer(comms);
	scanComms.reset();
	scanComms = Comms::Mailslots(u"\\\\.\\mailslot\\clientScan", u"\\\\.\\mailslot\\server");
	Read reader(scanComms);
	writer.writeUInt8((uint8_t)CMDCODE::SCAN);
	writer.writeU16String(ui.pathLabel->text().toStdU16String());
	writer.writeU16String(u"\\\\.\\mailslot\\clientScan");
	uint64_t fileCount = reader.readUInt64();
	uint64_t scannedFilesCount = 0;
	for (uint64_t i = 0; i < fileCount; i++) {
		std::u16string path = reader.readU16String();
		bool safe = reader.readUInt8();
		if (!safe) {
			std::u16string virusName = reader.readU16String();
			QString result = QString::fromUtf16(path.c_str()) + QString(" found the ") + QString::fromUtf16(virusName.c_str());
			logShow(result);
		}
		else {
			QString result = QString::fromUtf16(path.c_str()) + QString(" is safe");
			logShow(result);
		}
		scannedFilesCount++;
		int percents = (scannedFilesCount / (float)fileCount) * 100;
		setProgressBar(percents);
	}
	QString result = "Files scanned: " + QString::number(scannedFilesCount);
	logShow(result);
}

void Client::requestDel(uint64_t index) {
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::DEL);
	writer.writeUInt64(index);
	Read reader(comms);
	bool success = (bool)reader.readUInt8();
	if (success) {
		viruses->remove(index);
		removeItem(index);
	}
}

void Client::setupMonitor(const std::u16string& path) {
	Write writer(comms);
	writer.writeUInt8((uint8_t)CMDCODE::MONITOR);
	writer.writeU16String(path);
}

void Client::loadMonitors() {
	std::u16string filePath = u"monitors.gor";
	Read reader(filePath);
	if (!reader.isOpen())
		return;
	std::u16string header = reader.readU16String();
	if (header != u"gorelov") {
		reader.close();
		return;
	}
	uint64_t recordNumber = reader.readUInt64();
	for (size_t i = 0; i < recordNumber; i++) {
		std::u16string scanPath = reader.readU16String();
		ui.monitorLog->setRowCount(ui.monitorLog->rowCount() + 1);
		int lastIndex = ui.monitorLog->rowCount() - 1;
		ui.monitorLog->setItem(lastIndex, 0, new QTableWidgetItem(QString::fromUtf16(scanPath.c_str())));
	}
	reader.close();
}

void Client::loadScanners() {
	std::u16string filePath = u"scanners.gor";
	Read reader(filePath);
	if (!reader.isOpen())
		return;
	std::u16string header = reader.readU16String();
	if (header != u"gorelov") {
		reader.close();
		return;
	}
	uint64_t recordNumber = reader.readUInt64();
	for (size_t i = 0; i < recordNumber; i++) {
		std::u16string scanPath = reader.readU16String();
		uint32_t hours = reader.readUInt32();
		uint32_t minutes = reader.readUInt32();
		ui.timmerLog->setRowCount(ui.timmerLog->rowCount() + 1);
		int lastIndex = ui.timmerLog->rowCount() - 1;
		ui.timmerLog->setItem(lastIndex, 0, new QTableWidgetItem(QString::fromUtf16(scanPath.c_str())));
		QString time = QString::number(hours) + QString(":") + QString::number(minutes);
		ui.timmerLog->setItem(lastIndex, 1, new QTableWidgetItem(time));
	}
	reader.close();
}

