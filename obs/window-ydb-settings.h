#ifndef WINDOWYDBSETTINGS_H
#define WINDOWYDBSETTINGS_H

#include <QDialog>
#include "util/base.h"
#include <QListWidgetItem>
#include "showimg.h"
#include "window-ydb-filepath.h"
#include "YDBUtil.h"
#include "window-basic-main.hpp"
#include "window-ydb-filename.h"
#include "qt-wrappers.hpp"
#include "uploadfiledialog.hpp"

extern "C" {
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavcodec/avcodec.h"  
#include "libavutil/time.h"  
#include "libavutil/imgutils.h"  
}

namespace Ui {
class YDBSettings;
}

class YDBSettings : public QDialog
{
    Q_OBJECT

public:
    explicit YDBSettings(QWidget *parent = 0, OBSSource monitor_capture_source = NULL);
    ~YDBSettings();
	int SavetoJPEG(AVFrame *pFrameYUV, AVStream *pVStream, char* vedio_path, int width, int height);
	ShowImg* show_img = NULL;
	QImage* img = new QImage;
	QString file_path;
	QString directory_path;
	QListWidgetItem* item;
	

public:
	bool hasItem();
	
public slots:
	void on_listWidget_itemClicked(QListWidgetItem* item);
	void on_videoButton_clicked();
	void on_videoSettingsButton_clicked();
	void on_aboutButton_clicked();
	void on_helpButton_clicked();
	void on_pathSettingsButton_clicked();
	void on_deleteButton_clicked();
	void on_renameButton_clicked();
	void on_uploadButton_clicked();
	void update_file_list();	//invokemethod调用
	void update_file_name(QString file_name);	//invokemethod调用

private:
    Ui::YDBSettings *ui;
	OBSBasic* main;
	OBSSource monitor_capture_source;
};

#endif // WINDOWYDBSETTINGS_H
