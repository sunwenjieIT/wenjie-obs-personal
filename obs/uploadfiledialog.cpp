#include "uploadfiledialog.hpp"
#include "ui_uploadfiledialog.h"

#include "window-basic-main.hpp"


#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <qfile.h>
#include <QProgressBar>



UploadFileDialog::UploadFileDialog(QWidget *parent, QString file_path) :
    QDialog(parent),
    ui(new Ui::UploadFileDialog)
{
    ui->setupUi(this);
	ui->selectFileButton->setProperty("class", "normal");
	ui->pushButton->setProperty("class", "normal");

	defaultList.append(0);
	defaultList.append(10001);
	if (NULL != file_path) {
		ui->filePath->setText(file_path);
		this->file_path = file_path;

	}
}


/************************************************************************/
/* 选择文件                                                                     */
/************************************************************************/
void UploadFileDialog::on_selectFileButton_clicked()
{
	QString file_path = QFileDialog::getOpenFileName(this, "open file", "/", "textfile(*.txt);;C file(*.cpp);;All file(*.*)");
	ui->progressBar->setValue(0);
	ui->filePath->setText(file_path.toUtf8());
}

/*********工作线程结束响应函数******************/
void UploadFileDialog::testSlot(int value) {
	blog(LOG_INFO, "finish num:%d", value);
	
	if (value == 0) {
		//成功
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.Success", 0), QMessageBox::Yes, QMessageBox::Yes);
	}
	else if (value == 1) {
		//部分分块上传失败
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.PartialSuccess", 0), QMessageBox::Yes, QMessageBox::Yes);
	}
	else if (value == 2){
		//完成上传失败
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.Completed.Error", 0), QMessageBox::Yes, QMessageBox::Yes);
	}
	else if(value == 4){
		//初始化失败
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.Init.Error", 0), QMessageBox::Yes, QMessageBox::Yes);

	}
	else if (value == 5){
		//网络异常
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.Network.Error", 0), QMessageBox::Yes, QMessageBox::Yes);

	}
	else if (value == 6) {
		//文件不存在
		QMessageBox::information(NULL, QApplication::translate("OBSBasic", "Upload.Message", 0), QApplication::translate("OBSBasic", "Upload.Message.File.Error", 0), QMessageBox::Yes, QMessageBox::Yes);

	}
	ui->pushButton->setText(QApplication::translate("OBSBasic", "Upload.PushButton.Start", 0));
}

void UploadFileDialog::taskFinished() {
	is_alive = false;
	ui->pushButton->setText(QApplication::translate("OBSBasic", "Upload.PushButton.Start", 0));
	//os_http_io_deinitialize();
	//mq->putMessage(-1);
}
/********上传button 开始上传********/
void UploadFileDialog::on_pushButton_clicked(){
	//mq->clear();
	ui->progressBar->setValue(0);
	task = new MyClass(this, ui->filePath->text(), ui->progressBar);
	connect(this, SIGNAL(stopTaskThread()), task, SLOT(stopThread()));
	connect(task, SIGNAL(testSignal(int)), this, SLOT(testSlot(int)));
	connect(task, SIGNAL(finished()), this, SLOT(taskFinished()));
	connect(task, SIGNAL(started()), this, SLOT(taskStarted()));
	connect(task, SIGNAL(finished()), task, SLOT(deleteLater()));
	task->start();
	
	//is_alive = true;
}
void UploadFileDialog::taskStarted() {
	is_alive = true;
	ui->pushButton->setText(QApplication::translate("OBSBasic", "Upload.PushButton.Uploading", 0));
	//disconnect(ui->pushButton, SIGNAL(clicked()), this, SLOT());
	//ui->pushButton->dis
}

UploadFileDialog::~UploadFileDialog()
{
	blog(LOG_INFO, "~ uploadfiledialog");
	if (is_alive) QMetaObject::invokeMethod(task, "stopThread", Qt::QueuedConnection);
	
	delete ui;
}