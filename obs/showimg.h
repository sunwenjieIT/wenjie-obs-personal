#ifndef SHOWIMG_H
#define SHOWIMG_H

#include <QWidget>
#include "ui_showimg.h"
#include <QPainter>
#include "QPixmap"
#include <QPushButton>
#include <QDesktopServices>
#include <QProcess>

Q_DECLARE_METATYPE(QImage*)

class ShowImg : public QWidget
{
	Q_OBJECT

public:
	ShowImg(QWidget *parent = 0, QImage* image = NULL, QString local_path = NULL);
	~ShowImg();
	QPushButton* my_play_button;
private:
	Ui::ShowImg ui;
	QImage* image;
	QString local_path;
	//char* file_path = NULL;
	public slots:
	void my_play_button_clicked();
	void update_image(QVariant qtest, QString local_path);
	void update_path(QString local_path);
protected:
	void paintEvent(QPaintEvent*);
};

#endif // SHOWIMG_H
