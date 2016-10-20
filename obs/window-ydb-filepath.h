#ifndef WINDOWYDBFILEPATH_H
#define WINDOWYDBFILEPATH_H

#include <QDialog>
#include "window-basic-main.hpp"
#include <QCloseEvent>
#include "util/base.h"

#include <crtdbg.h>
#include "qt-wrappers.hpp"
#include <shlobj.h>
#include <QDebug>
#include <QThread>

//class YDBFilePathRunnable : public QThread {
//public:
//	YDBFilePathRunnable(QString )
//};

namespace Ui {
class YDBFilePath;
}

class YDBFilePath : public QDialog
{
    Q_OBJECT

public:
    explicit YDBFilePath(QWidget *parent = 0, OBSBasic* main = NULL);
    ~YDBFilePath();
	void update_config_local(QString key, QString value);

public slots:
	void on_selectPathButton_clicked();
	virtual void accept();
	
private:
    Ui::YDBFilePath *ui;
	OBSBasic* main;
	QSettings *local_ini;
	wchar_t path_utf16[MAX_PATH];
	
protected:
	void paintEvent(QPaintEvent *event);
};

#endif // WINDOWYDBFILEPATH_H
