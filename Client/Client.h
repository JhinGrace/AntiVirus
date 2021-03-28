#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Client.h"
#include <Windows.h>
#include <Liba.h>
#include <Viruses.h>

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = Q_NULLPTR);
	~Client();

private:
	void serverConnect();
	void serverWakeUp();
	void requestScan();
	void requestDel(uint64_t index);
	void setupMonitor(const std::u16string& path);
	void loadMonitors();
	void loadScanners();

signals:
	void logShow(const QString& value);
	void setProgressBar(int value);
	void removeItem(int index);

private slots:
	//main window
	void on_targetButton_clicked();
	void on_scanButton_clicked();
	void on_stopScanButton_clicked();
	void on_logButton_clicked();
	void on_quitButton_clicked();
	//virus log
	void on_delButton_clicked();
	//timmer
	void on_timmerButton_clicked();
	void on_timmerStart_clicked();
	void on_timmerStop_clicked();
	void on_timmerBrowse_clicked();
	//monitor
	void on_monitorButton_clicked();
	void on_monitorBrowse_clicked();
	void on_monitorStart_clicked();
	void on_monitorStop_clicked();
	//return buttons
	void on_returnButton_clicked();
	void on_returnButton2_clicked();
	void on_returnButton3_clicked();

private:
	Ui::ClientClass ui;
	std::unique_ptr<Viruses> viruses;
	std::shared_ptr<Comms> comms;
	std::shared_ptr<Comms> scanComms;
	QThread* scanThread;
};