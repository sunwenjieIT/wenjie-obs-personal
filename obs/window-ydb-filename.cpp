#include "window-ydb-filename.h"
#include "ui_YDBFileName.h"

YDBFileName::YDBFileName(QWidget *parent, QString file_name) :
    QDialog(parent),
    ui(new Ui::YDBFileName)
{
    ui->setupUi(this);
	this->file_name = file_name;
	ui->fileNameEdit->setText(file_name);
}
void YDBFileName::accept() {

	if (ui->fileNameEdit->text() != file_name) {
		QMetaObject::invokeMethod(this->parentWidget(), "update_file_name", Qt::AutoConnection, Q_ARG(QString, ui->fileNameEdit->text()));
	}
	QDialog::accept();
}

//void YDBFileName::on_fileNameEdit_textChanged(QString file_name) {
//	if (this->file_name == file_name)return;
//	QString a = file_name;
//	QMetaObject::invokeMethod(this->parentWidget(), "update_file_name", Qt::AutoConnection, Q_ARG(QString, file_name));
//}

YDBFileName::~YDBFileName()
{
    delete ui;
}
