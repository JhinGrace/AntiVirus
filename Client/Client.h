#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Client.h"
#include <Windows.h>
#include <thread>

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = Q_NULLPTR);
	~Client();

private:
	void connectToServer();
	void wakeUpServer();
	void writeToServer();
	void readFromServer();
	void sendRequest();
	void TestRequest();

signals:
	void output(const QString& text);

private slots:
	void on_pushButton_clicked();

private:
	Ui::ClientClass ui;
	HANDLE hClient;
	HANDLE hServer;
	HANDLE clientUp;
};