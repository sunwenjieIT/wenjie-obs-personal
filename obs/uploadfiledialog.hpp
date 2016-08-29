#ifndef UPLOADFILEDIALOG_H
#define UPLOADFILEDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <qfile.h>
#include <QProgressDialog>  
//#include <MyInt64Mq.h>
#include <QThreadPool>


#define NUM_THREADS 5
#include "update-progressbar.h"


namespace Ui {
class UploadFileDialog;
}


class UploadFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadFileDialog(QWidget *parent = 0);
    ~UploadFileDialog();

signals:
	void stopTaskThread();

public slots:
	//void on_uploadButton_clicked();
	void on_selectFileButton_clicked();
	//void on_iniButton_clicked();
	void on_pushButton_clicked();
	void testSlot(int value);
	void taskFinished();
	void taskStarted();

public:
	
	QList<QVariant> defaultList;
	//MyInt64Mq *mq = NULL;
	MyClass* task = NULL;
	bool is_alive = false;

public:
	void easyUpload();

private:
    Ui::UploadFileDialog *ui;
};

#endif // UPLOADFILEDIALOG_H
