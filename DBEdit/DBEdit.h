#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_DBEdit.h"

class DBEdit : public QMainWindow
{
    Q_OBJECT
public:
    DBEdit(QWidget *parent = Q_NULLPTR);
	void save(const std::u16string& path);
	void load(const std::u16string& path);
private slots:
	void on_Save_clicked();
	void on_Load_clicked();
	void on_Add_clicked();
	void on_Delete_clicked();
	void cellChanged(int row, int column);
private:
    Ui::DBEditClass ui;
};

