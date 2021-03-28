#include "DBEdit.h"
#include "Liba.h"
#include <QFileDialog>
#include <sstream>
#include <../PicoSHA2/picosha2.h>
#include "Read.h"
#include "Write.h"

static std::vector<uint8_t> hex2byte(const std::string& hex) {
	std::vector<uint8_t> bytes;
	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}
	return bytes;
}

static bool isHex(const std::string& string) {
	for (auto character : string) {
		if (!std::isxdigit(character))
			return false;
	}
	return true;
}

DBEdit::DBEdit(QWidget* parent) : QMainWindow(parent) {
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	connect(ui.tableWidget, &QTableWidget::cellChanged, this, &DBEdit::cellChanged);
}

void DBEdit::on_Save_clicked() {
	QFileDialog* dialog = new QFileDialog(nullptr, "OpenFileDialog", "./", "*.gor");
	dialog->show();
	if (dialog->exec())
		save(dialog->selectedFiles()[0].toStdU16String());
}

void DBEdit::on_Load_clicked() {
	QFileDialog* dialog = new QFileDialog(nullptr, "OpenFileDialog", "./", "*.gor");
	dialog->show();
	if (dialog->exec())
		load(dialog->selectedFiles()[0].toStdU16String());
}

void DBEdit::save(const std::u16string& path) {
	Write writer(path);
	std::u16string header(u"gorelov");
	writer.writeU16String(header);
	writer.writeUInt64((uint64_t)ui.tableWidget->rowCount());
	for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
		std::u16string name = ui.tableWidget->item(i, 0)->data(Qt::DisplayRole).toString().toStdU16String();
		std::u16string type = ui.tableWidget->item(i, 1)->data(Qt::DisplayRole).toString().toStdU16String();
		std::string hash = ui.tableWidget->item(i, 6)->data(Qt::DisplayRole).toString().toStdString();
		writer.writeU16String(name);
		writer.writeU16String(type);
		writer.writeUInt64(ui.tableWidget->item(i, 2)->data(Qt::DisplayRole).toULongLong());
		QString hex = ui.tableWidget->item(i, 3)->data(Qt::DisplayRole).toString();
		bool success;
		uint64_t sigStart = hex.toULongLong(&success, 16);
		writer.writeUInt64(sigStart);
		writer.writeUInt64(ui.tableWidget->item(i, 4)->data(Qt::DisplayRole).toULongLong());
		writer.writeUInt64(ui.tableWidget->item(i, 5)->data(Qt::DisplayRole).toULongLong());
		writer.writeASCIIString(hash);
	}
	writer.close();
}

void DBEdit::load(const std::u16string& path) {
	Read reader(path);
	std::u16string header = reader.readU16String();
	if (header != std::u16string(u"gorelov"))
		return;
	uint64_t rowCount = reader.readUInt64();
	ui.tableWidget->setRowCount(rowCount);
	for (int i = 0; i < rowCount; i++) {
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(
			QString::fromUtf16(reader.readU16String().c_str())));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(
			QString::fromUtf16(reader.readU16String().c_str())));
		uint64_t length = reader.readUInt64();
		std::stringstream toHex;
		uint64_t sigStart = reader.readUInt64();
		toHex << std::hex << sigStart;
		std::string hex = toHex.str();
		if (hex.size() % 2 != 0)
			hex = "0" + hex;
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(hex.c_str()));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(length)));
		ui.tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(reader.readUInt64())));
		ui.tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(reader.readUInt64())));
		ui.tableWidget->setItem(i, 6, new QTableWidgetItem(QString(reader.readASCIIString().c_str())));
	}
	reader.close();
}

void DBEdit::cellChanged(int row, int column) {
	if (column != 3) //если не меняем сигнатуру, то ниче не делать
		return;
	if (ui.tableWidget->item(row, column)->text().isEmpty()) {
		ui.tableWidget->item(row, column)->setText("");
		return;
	}
	std::string cellValue;
	try {
		cellValue = ui.tableWidget->item(row, column)->data(Qt::DisplayRole).toString().toStdString();
		cellValue.erase(remove_if(cellValue.begin(), cellValue.end(), isspace), cellValue.end());
		uint64_t length = cellValue.size();	//проверить чтоб не меньше 8 байт
		if (cellValue.size() < 16) {			//1 байт = 2 символа 
			ui.tableWidget->item(row, column)->setText("");
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(""));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(""));
			return;
		}
		if (!isHex(cellValue)) { 	//проверка на хексовость 
			ui.tableWidget->item(row, column)->setText("");
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(""));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(""));
			return;
		}
		if (length == 16) { 	//для восьмибайтовых хеш не считать
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(length / 2)));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(""));
			return;
		}
		ui.tableWidget->setItem(row, column, new QTableWidgetItem(cellValue.substr(0, 16).c_str()));	//начало сигнатуры
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(length / 2)));				//длина
		cellValue.erase(0, 16);
		std::vector<uint8_t> bytes = hex2byte(cellValue);
		std::string hash_hex_str;
		picosha2::hash256_hex_string(bytes, hash_hex_str);
		ui.tableWidget->setItem(row, 6, new QTableWidgetItem(hash_hex_str.c_str()));
	}
	catch (...) {
		ui.tableWidget->item(row, column)->setText("");
		ui.tableWidget->item(row, 2)->setText("");
		ui.tableWidget->item(row, 6)->setText("");
	}
}

void DBEdit::on_Add_clicked() {
	ui.tableWidget->setRowCount(ui.tableWidget->rowCount() + 1);
}

void DBEdit::on_Delete_clicked() {
	ui.tableWidget->setRowCount(ui.tableWidget->rowCount() - 1);
}