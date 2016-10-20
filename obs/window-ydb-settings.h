#ifndef WINDOWYDBSETTINGS_H
#define WINDOWYDBSETTINGS_H

#include <QDialog>
#include "util/base.h"
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QPaintEvent>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QImage>
#include <QImageReader>

#include "showimg.h"
#include "window-ydb-filepath.h"
#include "YDBUtil.h"
#include "window-basic-main.hpp"
#include "window-ydb-filename.h"
#include "qt-wrappers.hpp"
#include "uploadfiledialog.hpp"
#include "YDBUpdate.h"
#include <util/util.hpp>
#include "qmath.h"
//#include "window-basic-main.hpp"
#include "audio-encoders.hpp"

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

	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t =
		std::unique_ptr<obs_properties_t, properties_delete_t>;

public:
    explicit YDBSettings(QWidget *parent = 0, OBSSource monitor_capture_source = NULL);
    ~YDBSettings();
	int SavetoJPEG(AVFrame *pFrameYUV, AVStream *pVStream, char* vedio_path, int width, int height);
	ShowImg* show_img = NULL;
	QImage* img = new QImage;
	QString file_path;
	QString directory_path;
	//QListWidgetItem* item;
	QTreeWidgetItem* tree_item;

public:
	bool hasItem();
	
public slots:
	//void on_listWidget_itemClicked(QListWidgetItem* item);
	//void on_videoButton_clicked();
	//void on_videoSettingsButton_clicked();
	//void on_aboutButton_clicked();
	//void on_helpButton_clicked();
	//void on_pathSettingsButton_clicked();
	void on_deleteButton_clicked();
	void on_renameButton_clicked();
	void on_uploadButton_clicked();
	void update_file_list();	//invokemethod调用
	void update_file_name(QString file_name);	//invokemethod调用
	void on_treeWidget_itemClicked(QTreeWidgetItem*, int);
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem*, int);
	//void on_treeWidget_itemDoubleClicked(QTreeWidgetItem*, int);
private:
	QPoint last;
	QPointer<QThread> updateCheckThread;
    Ui::YDBSettings *ui;
	OBSBasic* main;
	OBSSource monitor_capture_source;
	QTreeWidgetItem* my_videos;
	void update_video_args(QString filepath);
	bool check_filetype(QString file);
	//void init_local_ini();
	//检查更新
	QNetworkAccessManager* manager;
	QString download_url;
	//录制设置
	properties_t properties;
	OBSData monitor_capture_settings;
	QString preset_key;
	QString preset;
	QString curPreset;
	QString curQSVPreset;
	QString curNVENCPreset;

	QSettings *local_ini;

	int old_simpleOutPreset_idx = -1;
	int old_simpleOutRecFormat_idx = -1;
	int old_sampleRate_idx = -1;
	int old_fpsCommon_idx = -1;
	int old_simpleOutStrEncoder_idx = -1;
	int old_auxAudioDevice1_idx = -1;
	int old_simpleOutputABitrate_idx = -1;
	int old_displaySelect_idx = -1;

	int old_simpleOutputVBitrate_value = -1;

	QString old_filePathEdit;

	QString old_simpleOutPreset;
	QString old_simpleOutRecFormat;
	QString old_sampleRate;
	QString old_fpsCommon;
	QString old_simpleOutStrEncoder;
	QString old_auxAudioDevice1;
	QString old_simpleOutputABitrate;

	bool simpleOutRecFormat_changed = false;
	bool sampleRate_changed = false;
	bool fpsCommon_changed = false;
	bool simpleOutStrEncoder_changed = false;
	bool simpleOutPreset_changed = false;
	bool auxAudioDevice1_changed = false;
	bool simpleOutputABitrate_changed = false;
	bool simpleOutputVBitrate_changed = false;
	bool captureCurosr_changed = false;
	bool displaySelect_changed = false;

	bool is_init_done = false;
	bool is_capture_cursor = NULL;
	bool isChanged = false;

	void update_config_local(QString key, QString value);
	void update_json_local(QString key, QString value);
	void SaveCombo(QComboBox *widget, const char *section,
		const char *value);
	void SaveComboData(QComboBox *widget, const char *section,
		const char *value);

	void LoadListValues(QComboBox *widget, obs_property_t *prop,
		int index);
	//是否全屏
	bool is_maximize = false;

private slots:
	
	//全屏/恢复函数
	void on_pushButton_5_clicked();
	//录制设置相关槽函数
	void SimpleRecordingEncoderChanged();
	void SimpleStreamingEncoderChanged();

	void on_simpleOutRecFormat_currentIndexChanged(int idx);
	void on_fpsCommon_currentIndexChanged(int idx);
	void on_simpleOutStrEncoder_currentIndexChanged(int idx);
	void on_simpleOutPreset_currentIndexChanged(int idx);
	void on_sampleRate_currentIndexChanged(int idx);
	void on_auxAudioDevice1_currentIndexChanged(int idx);
	void on_simpleOutputABitrate_currentIndexChanged(int idx);
	void on_checkBox_toggled(bool visible);
	void on_displaySelect_currentIndexChanged(int idx);
	void on_resetButton_clicked();
	void on_selectPathButton_clicked();
	void on_simpleOutputVBitrate_valueChanged(int value);
	
	//文件保存设置相关槽
	void on_changeFilePathYes_clicked();
	void on_changeFilePathCancel_clicked();
	//检查更新
	void on_checkUpdateButton_clicked();
	void replyFinished(QNetworkReply *);
protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	QPixmap background;
};

#endif // WINDOWYDBSETTINGS_H
