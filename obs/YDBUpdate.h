#ifndef YDBUPDATE_H
#define YDBUPDATE_H

#include <QDialog>

#include "remote-text.hpp"
#include <QPointer>
#include <QThread>

//#include <QUrl>

//#include "YDB-net.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>

#include <QDesktopServices>

namespace Ui {
class YDBUpdate;
}

class YDBUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit YDBUpdate(QWidget *parent = 0);
    ~YDBUpdate();

private:
	QPointer<QThread> updateCheckThread;
    Ui::YDBUpdate *ui;
	QNetworkAccessManager* manager;
	QString download_url;

private slots:
	void updateFileFinished(const QString &text, const QString &error);
	void replyFinished(QNetworkReply *);
	void on_pushButton_clicked();
};

#endif // YDBUPDATE_H
