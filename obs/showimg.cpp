#include "showimg.h"
#include <QtTest/QTest>
//#include <QTest>

ShowImg::ShowImg(QWidget *parent, QImage* image, QString local_path)
	: QWidget(parent)
{
	this->image = image;
	this->local_path = local_path;
	//this->file_path = file_path;
	ui.setupUi(this);


	my_play_button = new QPushButton(this);//
	/*my_play_button->setStyleSheet("QPushButton{border-image: url(:/qttest/play);}"
		"QPushButton:hover{border-image: url(:/qttest/pause);}"
		"QPushButton:pressed{border-image: url(:/qttest/stop);}");*/

	//my_play_button->setStyleSheet("QPushButton{border-image: url(:/view/images/view/play.png);}");
	my_play_button->setCursor(QCursor(Qt::PointingHandCursor));
	my_play_button->setStyleSheet("QPushButton{border-image: url(:/ydb/images/YDB/play.png);background:transparent;}QPushButton:hover{border-image:url(:/ydb/images/YDB/play-hover.png);background:transparent;}");
	connect(my_play_button, SIGNAL(clicked()), this, SLOT(my_play_button_clicked()));
}
void ShowImg::my_play_button_clicked() {
	//QString local_path = QString("C:/Users/wenjie/Videos/2016-08-15 18-52-07.flv"); //a.txt、a.exe、a.mp3、a.mp4、a.rmvb吉
	QString path = QString("file:///") + local_path;
	//QUrl url(QString("C:/Users/wenjie/Videos/2016-08-15 18-52-07.flv"));
	QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));
}
void ShowImg::paintEvent(QPaintEvent*) {
	if (NULL == image)return;
	QPixmap pix_image = QPixmap::fromImage(*image);
	QPainter painter(this);
	//image->scaled()
	int w = pix_image.width();
	
	//image->scaled(this->width(), this->height());
	//painter.drawPixmap(0, 0, pix_image);
	
	my_play_button->setGeometry(this->width() / 2 - 20, this->height() / 2 - 20, 40, 40);
	//play_button->setGeometry(this->width() / 2 - 20, this->height() / 2 - 20, 40, 40);
	painter.drawPixmap(0, 0, this->width(), this->height(), pix_image);
	//qDebug() << "paint event";
	//painter.drawPixmap(0, 0, pix_image.width(), pix_image.height(), pix_image);

	painter.setPen(QColor(139, 139, 139));
	painter.drawLine(0, 0, this->width() - 1, 0);
	painter.drawLine(0, 0, 0, this->height() - 1);
	painter.drawLine(this->width() - 1, 0, this->width() - 1, this->height() - 1);
	painter.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);
}
void ShowImg::update_image(QVariant qtest, QString local_path) {
	
	this->image = qtest.value<QImage*>();
	this->local_path = local_path;
	this->update();

	
}
void ShowImg::update_path(QString local_path) {
	this->local_path = local_path;
	qDebug() << "update path success";
}
ShowImg::~ShowImg()
{

}
