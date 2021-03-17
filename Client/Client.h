#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Client.h"
#include <Windows.h>
#include <Liba.h>

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = Q_NULLPTR);
	~Client();

private:
	void connectToServ();
	void serverWakeUp();
	void requestScan();

private slots:
	void on_Scan_clicked();

private:
	Ui::ClientClass ui;
	std::shared_ptr<Comms> comms;
};