#ifndef WINDOWYDBFILENAME_H
#define WINDOWYDBFILENAME_H

#include <QDialog>

namespace Ui {
class YDBFileName;
}

class YDBFileName : public QDialog
{
    Q_OBJECT

public:
    explicit YDBFileName(QWidget *parent = 0, QString file_name = NULL);
    ~YDBFileName();

public slots:
	virtual void accept();
	//void on_fileNameEdit_textChanged(QString file_name);
private:
    Ui::YDBFileName *ui;
	QString file_name = NULL;
};

#endif // WINDOWYDBFILENAME_H
