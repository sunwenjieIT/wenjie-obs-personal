#include "window-ydb-settings.h"
#include "ui_YDBSettings.h"
#include <QDir>
#include <QTextCodec>
#include <QDebug>
#include <windows.h>
#include "obs-app.hpp"
#include "remote-text.hpp"
#include "QPoint"
#include "QThread"
#include <QDateTime>
#include <QFile>
#include "QDebug"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#include "platform.hpp"

using namespace std;
static inline QString GetComboData(QComboBox *combo)
{
	int idx = combo->currentIndex();
	if (idx == -1)
		return QString();

	return combo->itemData(idx).toString();
}
static inline bool WidgetChanged(QWidget *widget)
{
	return widget->property("changed").toBool();
}
static inline void SetComboByName(QComboBox *combo, const char *name)
{
	int idx = combo->findText(QT_UTF8(name));
	if (idx != -1)
		combo->setCurrentIndex(idx);
}
static inline void LoadListValue(QComboBox *widget, const char *text,
	const char *val)
{
	widget->addItem(QT_UTF8(text), QT_UTF8(val));
}
static bool EncoderAvailable(const char *encoder)
{
	const char *val;
	int i = 0;

	while (obs_enum_encoder_types(i++, &val))
		if (strcmp(val, encoder) == 0)
			return true;

	return false;
}
static void PopulateAACBitrates(QComboBox* box) {
	//static void PopulateAACBitrates(initializer_list<QComboBox*> boxes) {
	auto &bitrateMap = GetAACEncoderBitrateMap();
	if (bitrateMap.empty())
		return;

	vector<pair<QString, QString>> pairs;
	for (auto &entry : bitrateMap)
		pairs.emplace_back(QString::number(entry.first),
			obs_encoder_get_display_name(entry.second));

	//for (auto box : boxes) {
	QString currentText = box->currentText();
	box->clear();

	for (auto &pair : pairs) {
		box->addItem(pair.first);
		box->setItemData(box->count() - 1, pair.second,
			Qt::ToolTipRole);
	}

	box->setCurrentText(currentText);
	//}
}
YDBSettings::YDBSettings(QWidget *parent, OBSSource monitor_capture_source) :
	//QDialog(parent),
	QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::YDBSettings),
	properties(obs_source_properties(monitor_capture_source), obs_properties_destroy)
{
	this->setAttribute(Qt::WA_TranslucentBackground);
	
	main = qobject_cast<OBSBasic*>(parent);
	
	this->monitor_capture_source = monitor_capture_source;
    ui->setupUi(this);
	ui->displaySelect->setView(new QListView());
	ui->sampleRate->setView(new QListView());
	ui->fpsCommon->setView(new QListView());
	ui->simpleOutputABitrate->setView(new QListView());
	ui->auxAudioDevice1->setView(new QListView());
	ui->simpleOutStrEncoder->setView(new QListView());
	ui->simpleOutPreset->setView(new QListView());
	ui->simpleOutRecFormat->setView(new QListView());

	ui->page_4->setProperty("class", "normal");
	ui->page_5->setProperty("class", "normal");
	ui->page_6->setProperty("class", "normal");
	//qss
	QString qss;
	QFile qssFile(":/data/themes/ydb.qss");

	qssFile.open(QFile::ReadOnly);

	if (qssFile.isOpen())

	{

		qss = QLatin1String(qssFile.readAll());

		qApp->setStyleSheet(qss);

		qssFile.close();

	}


	ui->simpleOutPreset->setVisible(false);
	ui->label_17->setVisible(false);
	//this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);

	//录制设置字体颜色
	//ui->groupBox_4->setStyleSheet("border-color：rgb（255, 85, 0）");

	//左侧导航栏 我的视频item
	my_videos = ui->treeWidget->topLevelItem(0);


	//ui->treeWidget->topLevelItem(1)->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
	//ui->treeWidget->topLevelItem(2)->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
	//ui->treeWidget->topLevelItem(3)->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
	//ui->treeWidget->topLevelItem(4)->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

	//ui->treeWidget->setRootIsDecorated(false);

	ui->treeWidget->setFocusPolicy(Qt::NoFocus);
	//ui->treeWidget->setStyleSheet(
		//"QTreeView{background-color:#25B186;color:white;show-decoration-selected: 1;}QTreeView::item{height:25px;}");
	//更新文件列表
	update_file_list();

	setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);
	//ui->horizontalLayout->:/icon/images/obs.png
	QIcon icon;
	icon.addFile(QStringLiteral(":/res/images/obs.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(icon);

	/************************************************************************/
	/* 视频录制设置初始化开始                                                                     */
	/************************************************************************/

	monitor_capture_settings = obs_source_get_settings(monitor_capture_source);
	//视频比特率
	int videoBitrate = config_get_uint(main->Config(), "SimpleOutput",
		"VBitrate");
	ui->simpleOutputVBitrate->setValue(videoBitrate);
	old_simpleOutputVBitrate_value = videoBitrate;
	//录制光标(全屏录制)
	is_capture_cursor = obs_data_get_bool(monitor_capture_settings, "capture_cursor");
	ui->checkBox->setChecked(is_capture_cursor);
	//音频比特率
	PopulateAACBitrates(ui->simpleOutputABitrate);

	int audioBitrate = config_get_uint(main->Config(), "SimpleOutput",
		"ABitrate");
	audioBitrate = FindClosestAvailableAACBitrate(audioBitrate);
	SetComboByName(ui->simpleOutputABitrate,
		std::to_string(audioBitrate).c_str());
	old_simpleOutputABitrate_idx = ui->simpleOutputABitrate->currentIndex();
	old_simpleOutputABitrate = ui->simpleOutputABitrate->currentText();

	connect(ui->simpleOutputABitrate, SIGNAL(currentIndexChanged(int)),
		this, SLOT(SimpleRecordingEncoderChanged()));
	//
	const char *format = config_get_string(main->Config(), "SimpleOutput",
		"RecFormat");
	int idx = ui->simpleOutRecFormat->findText(format);
	ui->simpleOutRecFormat->setCurrentIndex(idx);
	old_simpleOutRecFormat = ui->simpleOutRecFormat->currentText();
	old_simpleOutRecFormat_idx = ui->simpleOutRecFormat->currentIndex();
	//FPS
	const char *val = config_get_string(main->Config(), "Video",
		"FPSCommon");
	idx = ui->fpsCommon->findText(val);
	if (idx == -1) idx = 3;
	ui->fpsCommon->setCurrentIndex(idx);
	old_fpsCommon = ui->fpsCommon->currentText();
	old_fpsCommon_idx = ui->fpsCommon->currentIndex();

	//视频CODEC
#define ENCODER_STR(str) QTStr("Basic.Settings.Output.Simple.Encoder." str)
	ui->simpleOutStrEncoder->addItem(
		ENCODER_STR("Software"),
		QString(SIMPLE_ENCODER_X264));
	if (EncoderAvailable("obs_qsv11"))
		ui->simpleOutStrEncoder->addItem(
			ENCODER_STR("Hardware.QSV"),
			QString(SIMPLE_ENCODER_QSV));
	if (EncoderAvailable("ffmpeg_nvenc"))
		ui->simpleOutStrEncoder->addItem(
			ENCODER_STR("Hardware.NVENC"),
			QString(SIMPLE_ENCODER_NVENC));

	const char *streamEnc = config_get_string(main->Config(), "SimpleOutput",
		"StreamEncoder");
	idx = ui->simpleOutStrEncoder->findData(QString(streamEnc));
	if (idx == -1) idx = 0;
	ui->simpleOutStrEncoder->setCurrentIndex(idx);
	old_simpleOutStrEncoder = ui->simpleOutStrEncoder->currentText();
	old_simpleOutStrEncoder_idx = ui->simpleOutStrEncoder->currentIndex();

	connect(ui->simpleOutStrEncoder, SIGNAL(currentIndexChanged(int)),
		this, SLOT(SimpleStreamingEncoderChanged()));
	connect(ui->simpleOutStrEncoder, SIGNAL(currentIndexChanged(int)),
		this, SLOT(SimpleRecordingEncoderChanged()));


	curQSVPreset = config_get_string(main->Config(), "SimpleOutput",
		"QSVPreset");
	curPreset = config_get_string(main->Config(), "SimpleOutput",
		"Preset");
	curNVENCPreset = config_get_string(main->Config(), "SimpleOutput",
		"NVENCPreset");

	//采样频率
	uint32_t sampleRate = config_get_uint(main->Config(), "Audio",
		"SampleRate");
	const char *str;
	if (sampleRate == 48000)
		str = "48khz";
	else
		str = "44.1khz";
	int sampleRateIdx = ui->sampleRate->findText(str);
	if (sampleRateIdx != -1)
		ui->sampleRate->setCurrentIndex(sampleRateIdx);
	old_sampleRate = ui->sampleRate->currentText();
	old_sampleRate_idx = ui->sampleRate->currentIndex();

	//来源
	const char *input_id = App()->InputAudioSource();
	obs_properties_t *input_props = obs_get_source_properties(input_id);
	if (input_props) {
		obs_property_t *inputs = obs_properties_get(input_props,
			"device_id");
		LoadListValues(ui->auxAudioDevice1, inputs, 3);
		obs_properties_destroy(input_props);
	}
	old_auxAudioDevice1 = ui->auxAudioDevice1->currentText();
	old_auxAudioDevice1_idx = ui->auxAudioDevice1->currentIndex();
	SimpleStreamingEncoderChanged();
	//屏幕选择
	obs_property_t* monitor_property = obs_properties_get(properties.get(), "monitor");
	const char        *name = obs_property_name(monitor_property);
	obs_property_type type = obs_property_get_type(monitor_property);

	size_t count = obs_property_list_item_count(monitor_property);
	obs_combo_format my_format = obs_property_list_format(monitor_property);


	for (size_t i = 0; i < count; i++) {
		const char *item_name = obs_property_list_item_name(monitor_property, i);
		QString var = QString(obs_property_list_item_string(monitor_property, i));
		ui->displaySelect->addItem(QT_UTF8(item_name), var);
	}
	idx = obs_data_get_int(monitor_capture_settings, "monitor");
	if (idx != -1)
		ui->displaySelect->setCurrentIndex(idx);

	if (main->recording_status == main->Recording_status::Pausing) {
		ui->sampleRate->setDisabled(true);
		ui->displaySelect->setDisabled(true);
		ui->simpleOutputABitrate->setDisabled(true);
		ui->simpleOutStrEncoder->setDisabled(true);
		ui->fpsCommon->setDisabled(true);
		ui->simpleOutPreset->setDisabled(true);
		ui->simpleOutRecFormat->setDisabled(true);
		ui->auxAudioDevice1->setDisabled(true);
		ui->simpleOutputVBitrate->setDisabled(true);

	}
	is_init_done = true;
	/************************************************************************/
	/* 视频录制设置初始化结束                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* 文件保存设置初始化开始                                                                     */
	/************************************************************************/
	const char *path = config_get_string(main->Config(), "SimpleOutput",
		"FilePath");
	ui->filePathEdit->setText(path);
	old_filePathEdit = ui->filePathEdit->text();
	/************************************************************************/
	/* 文件保存设置初始化结束                                                                     */
	/************************************************************************/

}

void YDBSettings::update_file_list() {
	const char *path = config_get_string(main->Config(), "SimpleOutput",
		"FilePath");
	directory_path = QString(path) + "/";

	QDir myDir(path);
	QString file_type;
	QString sext;
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	myDir.setSorting(QDir::Time);

	for (int i = my_videos->childCount() - 1; i >= 0; i--) {
		my_videos->removeChild(my_videos->child(i));
	}
	//my_videos->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
	//ui->listWidget->clear();
	QColor black_color = QColor(0, 0, 0);
	QColor back_color = QColor(36, 164, 123);
	QBrush bg(Qt::red);
	for (unsigned int i = 0; i < myDir.count(); i++)
	{
		sext = myDir[i].toLower();
		if(check_filetype(sext)){
			QTreeWidgetItem* item = new QTreeWidgetItem(my_videos);
			item->setTextColor(0, black_color);
			item->setBackgroundColor(0, QColor(36, 164, 123));
			//item->setIcon(0, QIcon(":/ydb/images/YDB/vline-2.png"));
			//item->setroo
			item->setText(0, sext);
			/*QStandardItemModel* model = new QStandardItemModel(4,1);
			QStandardItem* itemb = new QStandardItem("test");
			itemb->set*/
			//item->setIcon(0, QIcon(":/ydb/images/YDB/back_green.png"));
			
			//item->setBackgroundColor(0, back_color);
		/*int point = sext.lastIndexOf(".");
		if (point != -1) {
			file_type = sext.mid(point);
			if (file_type == ".flv" || file_type == ".mp4" || file_type == ".mov" || file_type == ".mkv"
				|| file_type == ".ts" || file_type == ".m3u8" ){*/
				//ui->listWidget->addItem(sext);
				/************************************************************************/
				/*         QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem(treeWidget);
								__qtreewidgetitem1->setIcon(0, icon1);
								new QTreeWidgetItem(__qtreewidgetitem1);
								new QTreeWidgetItem(__qtreewidgetitem1);
								new QTreeWidgetItem(__qtreewidgetitem1);                                                                     */
				/************************************************************************/
				//QTreeWidgetItem* my_videos = ui->treeWidget->topLevelItem(0);
		}
	}
		//flv,mp4,mov,mkv,ts,m3u8
		/*if (-1 != sext.indexOf(".flv") || -1 != sext.indexOf(".mp4") || -1 != sext.indexOf(".mov")
			|| -1 != sext.indexOf(".mkv") || -1 != sext.indexOf(".ts") || -1 != sext.indexOf(".m3u8")) {
			ui->listWidget->addItem(sext);
		}*/
	if (hasItem()) {
		//file_path = directory_path + ui->listWidget->item(0)->text();
		//ui->listWidget->itemClicked(ui->listWidget->item(0));
		file_path = directory_path + my_videos->child(0)->text(0);
		on_treeWidget_itemClicked(my_videos->child(0), 0);
		//ui->treeWidget->click
	}
}
/************************************************************************/
/* 检查文件格式, 是视频嘛? true:false                                                                     */
/************************************************************************/
bool YDBSettings::check_filetype(QString file) {
	int point = file.lastIndexOf(".");
	if (point == -1)
		return false;

	QString file_type = file.mid(point);
	return (file_type == ".flv" || file_type == ".mp4" || file_type == ".mov" || file_type == ".mkv"
		|| file_type == ".ts" || file_type == ".m3u8");
}
void YDBSettings::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int count) {
	QString text = item->text(0);
	if (text != "\346\210\221\347\232\204\350\247\206\351\242\221") {
		item->setExpanded(true);
	}
}
/************************************************************************/
/* 响应左侧导航栏的点击事件                                                                     */
/************************************************************************/
void YDBSettings::on_treeWidget_itemClicked(QTreeWidgetItem* item, int count) {
	
	QString text = item->text(0);
	if (check_filetype(text)) {
		file_path = directory_path + text;
		//file_path = filepathQstring;
		this->tree_item = item;
		update_video_args(file_path);
		ui->stackedWidget->setCurrentIndex(0);
	}
	else {
		if (text == "\346\210\221\347\232\204\350\247\206\351\242\221") {
			//我的视频
			ui->stackedWidget->setCurrentIndex(0);
			if(hasItem())
				on_treeWidget_itemClicked(my_videos->child(0), 0);
		}
		else if (text == "\346\226\207\344\273\266\344\277\235\345\255\230\350\256\276\347\275\256") {
			//文件保存设置
			//YDBFilePath pathDialog(this, main);
			//pathDialog.exec();
			ui->stackedWidget->setCurrentIndex(1);
		}
		else if (text == "\345\275\225\345\210\266\350\256\276\347\275\256") {
			//录制设置
			//YDBBasicSettings ydb_basic_settings(this->parentWidget(), monitor_capture_source);
			//ydb_basic_settings.exec();
			ui->stackedWidget->setCurrentIndex(2);

		}
		else if (text.compare("\345\205\263\344\272\216") == 0) {
			//关于(更新)
			//YDBUpdate update;
			//update.exec();
			ui->stackedWidget->setCurrentIndex(3);
		}
		else if (text == "\345\270\256\345\212\251") {
			//TODO 
		}
	}

}
//void YDBSettings::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int count) {
//	QString text = item->text(0);
//	QString test;
//	int a = ui->treeWidget->currentColumn();
//	int b = item->columnCount();
//	if (text == "\346\226\207\344\273\266\344\277\235\345\255\230\350\256\276\347\275\256") {
//		//文件保存设置
//		YDBFilePath pathDialog(this, main);
//		pathDialog.exec();
//	}
//	else if (text == "\345\275\225\345\210\266\350\256\276\347\275\256"){
//		//录制设置
//		YDBBasicSettings ydb_basic_settings(this->parentWidget(), monitor_capture_source);
//		ydb_basic_settings.exec();
//	}
//	else if (text.compare("\345\205\263\344\272\216") == 0) {
//		//关于(更新)
//		YDBUpdate update;
//		update.exec();
//	}
//	else if (text == "\345\270\256\345\212\251") {
//		//TODO 
//	}
//}
/************************************************************************/
/* 更新文件名                                                                     */
/************************************************************************/
void YDBSettings::update_file_name(QString file_name) {
	tree_item->setText(0, file_name);
	//item->setText(file_name);

	//实例QFileInfo 函数
	QFileInfo file(file_path);
	//获取文件路径
	QString path = file.absolutePath();
	//bool型变量接收是否修改成功成功true，不成功 false。
	bool x = QFile::rename(file_path, path + "/" + file_name);
	if (x)
	{
		file_path = directory_path + file_name;
		QMetaObject::invokeMethod(show_img, "update_path", Qt::AutoConnection, Q_ARG(QString, file_path));
		qDebug() << "update file name success";
		//QMessageBox::warning(this, "修改文件名", "文件修改成功！");
	}
	else
	{
		//QMessageBox::warning(this, "修改文件名", "文件修改失败！");
	}
}
bool YDBSettings::hasItem() {
	return my_videos->childCount() > 0;
	//return ui->listWidget->count() > 0;
}
/************************************************************************/
/* 更新视频信息(视频大小 码率 时长 预览图)                                                                     */
/************************************************************************/
void YDBSettings::update_video_args(QString tmp_file_path) {
	QFileInfo fileInfo = QFileInfo(tmp_file_path);
	double size_byte = (double)(fileInfo.size() * 100 / 1024 / 1024) / 100;
	ui->size_value->setText(QString::number(size_byte) + " M");
	AVFormatContext *pFormatCtx;
	int             i, videoindex, PictureSize;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame *pFrame, *pFrameRGB;
	AVPacket packet;
	int ret, got_picture;
	char* filepath = YDBUtil::getLocal8BitChar(tmp_file_path);
	int64_t time_, time_total;

	struct SwsContext *pSwsCtx;
	uint8_t *outBuff;
	time_total = av_gettime_relative();
	av_register_all();
	avformat_network_init();

	pFormatCtx = avformat_alloc_context();
	time_ = av_gettime_relative();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		blog(LOG_ERROR, "Couldn't open input stream.");
		//qDebug() << "Couldn't open input stream.";
		return;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		//qDebug() << "Couldn't find stream information.";
		blog(LOG_ERROR, "Couldn't find stream information.");
		return;
	}

	videoindex = -1;

	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1) {
		blog(LOG_ERROR, "Didn't find a video stream.");
		//qDebug() << "Didn't find a video stream.";
		return;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		//qDebug() << "Codec not found.";
		blog(LOG_ERROR, "Codec not found.");
		return;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		blog(LOG_ERROR, "Could not open codec.");
		//qDebug() << "Could not open codec.";
		return;
	}

	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	if (pFrame == NULL || pFrameRGB == NULL)
	{
		blog(LOG_ERROR, "avframe malloc failed!");
		//qDebug() << "avframe malloc failed!";
		return;
	}
	//PictureSize = avpicture_get_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
	//TODO
	PictureSize = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height, 1);
	outBuff = (uint8_t*)av_malloc(PictureSize * sizeof(uint8_t));

	if (outBuff == NULL) {
		blog(LOG_ERROR, "av malloc failed!");
		//qDebug() << "av malloc failed!";
		return;
	}
	avpicture_fill((AVPicture *)pFrameRGB, outBuff, AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, outBuff, AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height, 1);

	pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);
	time_ = av_gettime_relative() - time_;
	blog(LOG_INFO, "open file and find info, cost time : %0.3fs", time_ / 1000000.0);
	//qDebug() << "open file and find info, cost time : " << time_ / 1000000.0;
	//printf("open file and find info, cost time: %0.3fs\n", time_ / 1000000.0);

	time_ = av_gettime_relative();
	int64_t timestamp = atoi("1");
	//int64_t timestamp = atoi(argv[1]);
	timestamp = av_rescale(timestamp, pFormatCtx->streams[videoindex]->time_base.den, (int64_t)pFormatCtx->streams[videoindex]->time_base.num);

	av_seek_frame(pFormatCtx, videoindex, timestamp, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[videoindex]->codec);
	time_ = av_gettime_relative() - time_;
	blog(LOG_INFO, "seek frame, costs time: %0.3fs", time_ / 1000000.0);
	//qDebug() << "seek frame, costs time:" << time_ / 1000000.0;
	//printf("seek frame, costs time: %0.3fs\n", time_ / 1000000.0);
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = NULL;
	avpkt.size = 0;

	time_ = av_gettime_relative();
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoindex) {
			if (packet.flags) {
				if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet)) < 0) {
					blog(LOG_ERROR, "Decode Error!");
					//qDebug() << "Decode Error!";
					//printf("Decode Error!\n");
					return;
				}
				while (got_picture == 0) {
					Sleep(10);
					if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &avpkt)) < 0) {
						blog(LOG_ERROR, "Decode Error!");
						//printf("Decode Error!\n");
						//qDebug() << "Decode Error!";
						return;
					}
				}
				if (got_picture) {
					time_ = av_gettime_relative() - time_;
					blog(LOG_INFO, "read and decode frame, costs time: %0.3fs", time_ / 1000000.0);
					//qDebug() << "read and decode frame, costs time:" << time_ / 1000000.0;

					time_ = av_gettime_relative();
					sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
					//if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], argv[3], pCodecCtx->width, pCodecCtx->height) < 0) {
					if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], filepath, pCodecCtx->width, pCodecCtx->height) < 0) {
						blog(LOG_ERROR, "Write Image Error!");
						//qDebug() << "Write Image Error!";
						return;
					}
					ui->kbps_value->setText(QString::number(pFormatCtx->bit_rate / 1000) + "kbps");
					QTime t = QTime::fromMSecsSinceStartOfDay(pFormatCtx->duration / 1000);
					ui->time_value->setText(t.toString("hh:mm:ss"));

					time_ = av_gettime_relative() - time_;
					blog(LOG_INFO, "write frame, costs time: %0.3fs", time_ / 1000000.0);
					//qDebug() << "write frame, costs time" << time_ / 1000000.0;
					av_packet_unref(&packet);
					break;
				}

			}

		}
		av_packet_unref(&packet);
	}
	sws_freeContext(pSwsCtx);
	av_packet_unref(&avpkt);
	av_free(outBuff);
	av_free(pFrameRGB);
	av_free(pFrame);
	delete filepath;
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	time_total = av_gettime_relative() - time_total;
	blog(LOG_INFO, "all done, costs time: %0.3fs", time_total / 1000000.0);
	//qDebug() << "all done, costs time:" << time_total / 1000000.0;
	//QMessageBox::information(NULL, "success title", "picture success!", QMessageBox::Yes, QMessageBox::Yes);
}
/************************************************************************/
/* 点击item                                                                     */
/************************************************************************/
//void YDBSettings::on_listWidget_itemClicked(QListWidgetItem* item) {
//	if (NULL == item) return;
//	ui->listWidget->setItemSelected(item, true);
//	
//	file_path = directory_path + item->text();
//	//file_path = "C:/Users/wenjie/Videos/" + item->text();
//	this->item = item;
//	QString text = item->text();
//	//TODO
//	QString filepathQstring = directory_path + text;
//	//QString filepathQstring = "C:/Users/wenjie/Videos/" + text;
//	QFileInfo fileInfo = QFileInfo(filepathQstring);
//	double size_byte = (double)(fileInfo.size() * 100 / 1024 / 1024) / 100;
//	ui->size_value->setText(QString::number(size_byte) + " M");
//	AVFormatContext *pFormatCtx;
//	int             i, videoindex, PictureSize;
//	AVCodecContext  *pCodecCtx;
//	AVCodec         *pCodec;
//	AVFrame *pFrame, *pFrameRGB;
//	AVPacket packet;
//	int ret, got_picture;
//	char* filepath = YDBUtil::getLocal8BitChar(filepathQstring);
//	int64_t time_, time_total;
//
//	struct SwsContext *pSwsCtx;
//	uint8_t *outBuff;
//	time_total = av_gettime_relative();
//	av_register_all();
//	avformat_network_init();
//
//	pFormatCtx = avformat_alloc_context();
//	time_ = av_gettime_relative();
//
//	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
//		qDebug() << "Couldn't open input stream.";
//		return;
//	}
//	
//	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
//		qDebug() << "Couldn't find stream information.";
//		return;
//	}
//
//	videoindex = -1;
//
//	for (i = 0; i < pFormatCtx->nb_streams; i++) {
//		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//			videoindex = i;
//			break;
//		}
//	}
//
//	if (videoindex == -1) {
//		qDebug() << "Didn't find a video stream.";
//		return;
//	}
//
//	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
//	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
//	if (pCodec == NULL) {
//		qDebug() << "Codec not found.";
//		return;
//	}
//	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
//		qDebug() << "Could not open codec.";
//		return;
//	}
//
//	pFrame = av_frame_alloc();
//	pFrameRGB = av_frame_alloc();
//	if (pFrame == NULL || pFrameRGB == NULL)
//	{
//		qDebug() << "avframe malloc failed!";
//		return;
//	}
//	//PictureSize = avpicture_get_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
//	//TODO
//	PictureSize = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height, 1);
//	outBuff = (uint8_t*)av_malloc(PictureSize * sizeof(uint8_t));
//
//	if (outBuff == NULL) {
//		qDebug() << "av malloc failed!";
//		printf("av malloc failed!\n");
//		return;
//	}
//	avpicture_fill((AVPicture *)pFrameRGB, outBuff, AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
//	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, outBuff, AV_PIX_FMT_YUV420P,
//		pCodecCtx->width, pCodecCtx->height, 1);
//
//	pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);
//	time_ = av_gettime_relative() - time_;
//	qDebug() << "open file and find info, cost time : " << time_ / 1000000.0;
//	printf("open file and find info, cost time: %0.3fs\n", time_ / 1000000.0);
//
//	time_ = av_gettime_relative();
//	int64_t timestamp = atoi("1");
//	//int64_t timestamp = atoi(argv[1]);
//	timestamp = av_rescale(timestamp, pFormatCtx->streams[videoindex]->time_base.den, (int64_t)pFormatCtx->streams[videoindex]->time_base.num);
//
//	av_seek_frame(pFormatCtx, videoindex, timestamp, AVSEEK_FLAG_BACKWARD);
//	avcodec_flush_buffers(pFormatCtx->streams[videoindex]->codec);
//	time_ = av_gettime_relative() - time_;
//	qDebug() << "seek frame, costs time:" << time_ / 1000000.0;
//	printf("seek frame, costs time: %0.3fs\n", time_ / 1000000.0);
//	AVPacket avpkt;
//	av_init_packet(&avpkt);
//	avpkt.data = NULL;
//	avpkt.size = 0;
//
//	time_ = av_gettime_relative();
//	while (av_read_frame(pFormatCtx, &packet) >= 0) {
//		if (packet.stream_index == videoindex) {
//			if (packet.flags) {
//				if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet)) < 0) {
//					qDebug() << "Decode Error!";
//					printf("Decode Error!\n");
//					return;
//				}
//				while (got_picture == 0) {
//					Sleep(10);
//					if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &avpkt)) < 0) {
//						printf("Decode Error!\n");
//						qDebug() << "Decode Error!";
//						return;
//					}
//				}
//				if (got_picture) {
//					time_ = av_gettime_relative() - time_;
//					qDebug() << "read and decode frame, costs time:" << time_ / 1000000.0;
//
//					time_ = av_gettime_relative();
//					sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
//					//if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], argv[3], pCodecCtx->width, pCodecCtx->height) < 0) {
//					if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], filepath, pCodecCtx->width, pCodecCtx->height) < 0) {
//						qDebug() << "Write Image Error!";
//						return;
//					}
//					ui->kbps_value->setText(QString::number(pFormatCtx->bit_rate / 1000) + "kbps");
//					QTime t = QTime::fromMSecsSinceStartOfDay(pFormatCtx->duration / 1000);
//					ui->time_value->setText(t.toString("hh:mm:ss"));
//
//					time_ = av_gettime_relative() - time_;
//					qDebug() << "write frame, costs time" << time_ / 1000000.0;
//					av_packet_unref(&packet);
//					break;
//				}
//
//			}
//
//		}
//		av_packet_unref(&packet);
//	}
//	sws_freeContext(pSwsCtx);
//	av_packet_unref(&avpkt);
//	av_free(outBuff);
//	av_free(pFrameRGB);
//	av_free(pFrame);
//	delete filepath;
//	avcodec_close(pCodecCtx);
//	avformat_close_input(&pFormatCtx);
//	time_total = av_gettime_relative() - time_total;
//	qDebug() << "all done, costs time:" << time_total / 1000000.0;
//	//QMessageBox::information(NULL, "success title", "picture success!", QMessageBox::Yes, QMessageBox::Yes);
//
//}
/************************************************************************/
/* 我的视频                                                                     */
/************************************************************************/
//void YDBSettings::on_videoButton_clicked() {
//	//ui->listWidget->isHidden
//	bool isHidden = ui->listWidget->isHidden();
//	if (isHidden)
//		ui->listWidget->show();
//	else
//		ui->listWidget->hide();
//}
/************************************************************************/
/* 录制设置                                                                     */
/************************************************************************/
//void YDBSettings::on_videoSettingsButton_clicked() {
//	YDBBasicSettings ydb_basic_settings(this->parentWidget(), monitor_capture_source);
//	ydb_basic_settings.exec();
//	//QMetaObject::invokeMethod(this->parentWidget(), "on_settingsButton_clicked", Qt::AutoConnection);
//}
/************************************************************************/
/* 文件保存设置                                                                     */
/************************************************************************/
//void YDBSettings::on_pathSettingsButton_clicked() {
//	YDBFilePath pathDialog(this, main);
//	pathDialog.exec();
//}
/************************************************************************/
/* 关于                                                                     */
/************************************************************************/
//void YDBSettings::on_aboutButton_clicked() {
//	YDBUpdate update;
//	update.exec();
//}
/************************************************************************/
/* 帮助                                                                     */
/************************************************************************/
//void YDBSettings::on_helpButton_clicked() {
//
//}
/************************************************************************/
/* 文件删除                                                                     */
/************************************************************************/
void YDBSettings::on_deleteButton_clicked() {
	//if (NULL == item) return;
	if (NULL == tree_item) return;

	qDebug() << file_path;

	my_videos->removeChild(tree_item);
	delete tree_item;
	tree_item = NULL;
	//ui->listWidget->removeItemWidget(item);
	//delete item;
	//item = NULL;
	if (QFile::remove(file_path)) {
		if (hasItem()) {
			on_treeWidget_itemClicked(my_videos->child(0), 0);
			//on_listWidget_itemClicked(ui->listWidget->item(0));
		}
		else {
			ui->horizontalLayout_3->removeWidget(show_img);
			ui->kbps_value->clear();
			ui->time_value->clear();
			ui->size_value->clear();
			delete show_img;
		}
	}
	else
	{
		QMessageBox::warning(NULL, "message", "delete file error!", QMessageBox::Yes);
	}
	
}
/************************************************************************/
/* 文件改名                                                                     */
/************************************************************************/
void YDBSettings::on_renameButton_clicked() {
	//if (NULL == item) return;
	//QString file_name = item->text();
	if (NULL == tree_item) return;
	QString file_name = tree_item->text(0);
	YDBFileName fileNameDialog(this, file_name);
	fileNameDialog.exec();
}
void YDBSettings::on_uploadButton_clicked() {
	UploadFileDialog uploadDialog(this, file_path);
	uploadDialog.exec();
}
int YDBSettings::SavetoJPEG(AVFrame *pFrameYUV, AVStream *pVStream, char* video_path, int width, int height) {
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* video_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	const char* out_file = "cuc_view_encode.jpg";

	uint8_t* picture_buf;
	AVFrame* picture;
	AVPacket pkt;
	int y_size;
	int got_picture = 0;
	int size;

	int ret = 0;

	int in_w = width, in_h = height;    //YUV's width and height  
	//char* out_file = filepath;    //Output file  

								  //Method 1  
	pFormatCtx = avformat_alloc_context();
	//Guess format  
	fmt = av_guess_format("mjpeg", NULL, NULL);
	pFormatCtx->oformat = fmt;
	//Output URL  
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		blog(LOG_ERROR, "Couldn't open output file.");
		return -1;
	}

	video_st = avformat_new_stream(pFormatCtx, 0);
	video_st->time_base.num = pVStream->time_base.num;
	video_st->time_base.den = pVStream->time_base.den;
	if (video_st == NULL) {
		return -1;
	}
	pCodecCtx = video_st->codec;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;

	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	//pCodecCtx->time_base.num = pVStream->codec->time_base.num;
	//pCodecCtx->time_base.den = pVStream->codec->time_base.den;
	
	//Output some information  
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		blog(LOG_ERROR, "Codec not found.");
		//qDebug() << "Codec not found.";
		//printf("Codec not found.");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		blog(LOG_ERROR, "Could not open codec.");
		//printf("Could not open codec.");
		//qDebug() << "Could not open codec.";
		return -1;
	}
	//test
	//pCodecCtx->qmin = pCodecCtx->qmax = 3;
	//pCodecCtx->flags |= CODEC_FLAG_QSCALE;
	
	picture = av_frame_alloc();
	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t *)av_malloc(size);
	if (!picture_buf)
	{
		return -1;
	}
	avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	//Write Header  
	avformat_write_header(pFormatCtx, NULL);

	y_size = pCodecCtx->width * pCodecCtx->height;
	av_new_packet(&pkt, y_size * 3);
	//read YUV
	//if(fread(picture_buf, 1, y_size*3/2, in_file))
	picture->data[0] = pFrameYUV->data[0];// Y  
	picture->data[1] = pFrameYUV->data[1];// U  
	picture->data[2] = pFrameYUV->data[2];// V  
	picture->width = in_w;
	picture->height = in_h;
	picture->format = AV_PIX_FMT_YUVJ420P;
	picture->pts = 0;


	//Encode  
	ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);
	if (ret < 0) {
		blog(LOG_ERROR, "Encode Error.");
		//qDebug() << "Encode Error.";
		//printf("Encode Error.\n");
		return -1;
	}
	QList<QByteArray> abc = QImageReader::supportedImageFormats();
	for (QByteArray tmp_array:abc){
		blog(LOG_INFO, "type: %s", tmp_array.data());
		
	}
	qDebug() << "Supported formats:" << QImageReader::supportedImageFormats();

	if (got_picture == 1) {
		pkt.stream_index = video_st->index;
		uint8_t* pkt_data = pkt.data;

		blog(LOG_INFO, "MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
			pkt_data[0] & 0xff, pkt_data[1] & 0xff, pkt_data[2] & 0xff,
			pkt_data[3] & 0xff, pkt_data[4] & 0xff, pkt_data[5] & 0xff);

		ret = av_write_frame(pFormatCtx, &pkt);
		blog(LOG_INFO, "write frame ret: %d", ret);
		//blog(LOG_INFO, "pkt_data:")
		QByteArray q_byte_array;
		
		q_byte_array.append((char*)pkt_data, pkt.size);
		//pkt.data
		
		//for (int i = 0; i < q_byte_array.length(); i++)
		//{
		//	blog(LOG_INFO, "%4x", q_byte_array.at(i) & 0xff);
		//}
		//blog(LOG_INFO, "data:%s", q_byte_array.l);

		img->load("cuc_view_encode.jpg", "jpg")?blog(LOG_INFO, "QImage load local jpg file success!"):blog(LOG_ERROR, "QImage load local jpg file failed!");
		
		/*img->loadFromData(q_byte_array, "jpeg") ?
			blog(LOG_ERROR, "load from data to jpeg success!") : blog(LOG_ERROR, "load from data to jpeg error!");
		img->loadFromData(q_byte_array, "jpg") ?
			blog(LOG_ERROR, "load from data to jpg success!") : blog(LOG_ERROR, "load from data to jpg error!");
*/
		if (NULL == show_img) {
			show_img = new ShowImg(NULL, img, QString(video_path));
			ui->horizontalLayout_3->addWidget(show_img);
		}
		else {
			QVariant qtest = QVariant::fromValue(img);
			QMetaObject::invokeMethod(show_img, "update_image", Qt::AutoConnection,
				Q_ARG(QVariant, qtest), Q_ARG(QString, QString(video_path)));
		}
	}

	av_packet_unref(&pkt);
	//Write Trailer  
	//test
	av_free_packet(&pkt);
	av_write_trailer(pFormatCtx);
	//pFormatCtx->
	blog(LOG_INFO, "Encode Successful.");
	//qDebug() << "Encode Successful.";
	//printf("Encode Successful.\n");

	if (video_st) {
		avcodec_close(video_st->codec);
		av_free(picture);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);
	
	//fclose(in_fi)

	return 0;
}


//文件设置相关函数
void YDBSettings::SimpleRecordingEncoderChanged() {

}
void YDBSettings::SimpleStreamingEncoderChanged() {
	QString encoder = ui->simpleOutStrEncoder->currentData().toString();
	//QString preset;
	const char *defaultPreset = nullptr;

	ui->simpleOutPreset->clear();

	if (encoder == SIMPLE_ENCODER_QSV) {
		ui->simpleOutPreset->addItem("speed", "speed");
		ui->simpleOutPreset->addItem("balanced", "balanced");
		ui->simpleOutPreset->addItem("quality", "quality");

		defaultPreset = "balanced";
		preset = curQSVPreset;
		preset_key = "SimpleOutput/QSVPreset";

	}
	else if (encoder == SIMPLE_ENCODER_NVENC) {
		obs_properties_t *props =
			obs_get_encoder_properties("ffmpeg_nvenc");

		obs_property_t *p = obs_properties_get(props, "preset");
		size_t num = obs_property_list_item_count(p);
		for (size_t i = 0; i < num; i++) {
			const char *name = obs_property_list_item_name(p, i);
			const char *val = obs_property_list_item_string(p, i);

			/* bluray is for ideal bluray disc recording settings,
			* not streaming */
			if (strcmp(val, "bd") == 0)
				continue;
			/* lossless should of course not be used to stream */
			if (astrcmp_n(val, "lossless", 8) == 0)
				continue;

			ui->simpleOutPreset->addItem(QT_UTF8(name), val);
		}

		obs_properties_destroy(props);

		defaultPreset = "default";
		preset = curNVENCPreset;
		preset_key = "SimpleOutput/NVENCPreset";

	}
	else {
		ui->simpleOutPreset->addItem("ultrafast", "ultrafast");
		ui->simpleOutPreset->addItem("superfast", "superfast");
		ui->simpleOutPreset->addItem("veryfast", "veryfast");
		ui->simpleOutPreset->addItem("faster", "faster");
		ui->simpleOutPreset->addItem("fast", "fast");
		ui->simpleOutPreset->addItem("medium", "medium");
		ui->simpleOutPreset->addItem("slow", "slow");
		ui->simpleOutPreset->addItem("slower", "slower");

		defaultPreset = "veryfast";
		preset = curPreset;
		preset_key = "SimpleOutput/Preset";
	}

	int idx = ui->simpleOutPreset->findData(QVariant(preset));
	if (idx == -1)
		idx = ui->simpleOutPreset->findData(QVariant(defaultPreset));
	//ui->simpleOutPreset->setProperty("changed", QVariant(true));
	ui->simpleOutPreset->setCurrentIndex(idx);
}

//修改槽函数
/************************************************************************/
/* 编码器预设                                                                     */
/************************************************************************/
void YDBSettings::on_simpleOutputVBitrate_valueChanged(int value) {
	if (is_init_done && value != old_simpleOutputVBitrate_value) {
		sender()->setProperty("changed", QVariant(true));
		simpleOutPreset_changed = true;

	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
void YDBSettings::on_simpleOutRecFormat_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutRecFormat_idx) {
		qDebug() << "on_simpleOutRecFormat_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutRecFormat_changed = true;
	}
}
void YDBSettings::on_simpleOutPreset_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutPreset_idx) {
		sender()->setProperty("changed", QVariant(true));
		simpleOutPreset_changed = true;
	}
}
void YDBSettings::on_sampleRate_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_sampleRate_idx) {
		qDebug() << "on_sampleRate_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		sampleRate_changed = true;
	}
}
void YDBSettings::on_simpleOutStrEncoder_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutStrEncoder_idx) {
		qDebug() << "on_simpleOutStrEncoder_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutStrEncoder_changed = true;
	}
}
void YDBSettings::on_fpsCommon_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_fpsCommon_idx) {
		qDebug() << "on_fpsCommon_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		fpsCommon_changed = true;
	}
}
void YDBSettings::on_auxAudioDevice1_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_auxAudioDevice1_idx) {
		qDebug() << "on_auxAudioDevice1_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		auxAudioDevice1_changed = true;
	}
}
void YDBSettings::on_simpleOutputABitrate_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutputABitrate_idx) {
		qDebug() << "on_simpleOutputABitrate_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutputABitrate_changed = true;
	}
}
void YDBSettings::on_checkBox_toggled(bool visible) {
	if (is_init_done && is_capture_cursor != visible) {
		qDebug() << captureCurosr_changed;
		captureCurosr_changed = true;
	}
}
void YDBSettings::on_displaySelect_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_displaySelect_idx) {
		qDebug() << "on_displaySelect_currentIndexChanged";
		sender()->setProperty("changed", QVariant(true));
		displaySelect_changed = true;
	}
}

void YDBSettings::LoadListValues(QComboBox *widget, obs_property_t *prop,
	int index)
{
	size_t count = obs_property_list_item_count(prop);

	obs_source_t *source = obs_get_output_source(index);
	const char *deviceId = nullptr;
	obs_data_t *settings = nullptr;

	if (source) {
		settings = obs_source_get_settings(source);
		if (settings)
			deviceId = obs_data_get_string(settings, "device_id");
	}

	widget->addItem(QTStr("Disabled"), "disabled");

	for (size_t i = 0; i < count; i++) {
		const char *name = obs_property_list_item_name(prop, i);
		const char *val = obs_property_list_item_string(prop, i);
		LoadListValue(widget, name, val);
	}

	if (deviceId) {
		QVariant var(QT_UTF8(deviceId));
		int idx = widget->findData(var);
		if (idx != -1) {
			widget->setCurrentIndex(idx);
		}
		else {
			widget->insertItem(0,
				QTStr("Basic.Settings.Audio."
					"UnknownAudioDevice"),
				var);
			widget->setCurrentIndex(0);
		}
	}

	if (settings)
		obs_data_release(settings);
	if (source)
		obs_source_release(source);
}

void YDBSettings::update_config_local(QString key, QString value){
	QString str2 = local_ini->value("SimpleOutput/FilePath").toString();
	local_ini->setValue(key, value);
	local_ini->sync();
}

void YDBSettings::update_json_local(QString a, QString b){
	QFile file;

	wchar_t path_utf16[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);
	char new_path[512] = "/obs-studio/basic/scenes/";
	strcat(new_path, Str("Untitled"));
	QString path1 = QString::fromWCharArray(path_utf16);
	QString path = path1 + new_path + ".json";

	file.setFileName(path);
	file.open(QIODevice::ReadWrite | QIODevice::Text);

	QString val = file.readAll();
	qDebug() << val;
	QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
	QJsonObject obj = d.object();
	QJsonValue value = obj.value("sources");
	QJsonArray array;
	if (value.isArray()) {
		array = value.toArray();
		int i = 0;
		for (QJsonValue v : array) {
			QJsonObject ob = v.toObject();

			QJsonValue id_value = ob["id"];
			qDebug() << id_value.toString();
			if (id_value.toString() == "monitor_capture") {
				qDebug() << ob.value("settings").toObject().value("capture_cursor").toBool();

				QJsonObject settings = ob.value("settings").toObject();
				settings.insert(a, b);
				ob.insert("settings", settings);
				QString tmp = QString(QJsonDocument(ob).toJson());
				qDebug() << tmp;
				array.replace(i, ob);
			}
			i++;
		}
	}
	obj.insert("sources", array);
	QString obj_qstt = QJsonDocument(obj).toJson();
	qDebug() << val;
	file.resize(0);
	file.write(obj_qstt.toUtf8());
	file.close();

}

void YDBSettings::SaveCombo(QComboBox *widget, const char *section,
	const char *value)
{
	if (WidgetChanged(widget))
		config_set_string(main->Config(), section, value,
			QT_TO_UTF8(widget->currentText()));
}
void YDBSettings::SaveComboData(QComboBox *widget, const char *section,
	const char *value)
{
	if (WidgetChanged(widget)) {
		QString str = GetComboData(widget);
		config_set_string(main->Config(), section, value,
			QT_TO_UTF8(str));
	}
}
void YDBSettings::mousePressEvent(QMouseEvent *event) {
	last = event->globalPos();
}
void YDBSettings::mouseMoveEvent(QMouseEvent *event) {
	int dx = event->globalX() - last.x();
	int dy = event->globalY() - last.y();

	last = event->globalPos();
	move(x() + dx, y() + dy);
}
void YDBSettings::mouseReleaseEvent(QMouseEvent *event) {
	int dx = event->globalX() - last.x();
	int dy = event->globalY() - last.y();

	move(x() + dx, y() + dy);
}
void YDBSettings::paintEvent(QPaintEvent* event) {
	//绘制边框
	/*QPainter painter(this);
	painter.setPen(QColor(139, 139, 139));
	painter.drawLine(0, 0, this->width() - 1, 0);
	painter.drawLine(0, 0, 0, this->height() - 1);
	painter.drawLine(this->width() - 1, 0, this->width() - 1, this->height() - 1);
	painter.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);*/
	

	//绘制边框阴影
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRect(10, 10, this->width() - 20, this->height() - 20);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillPath(path, QBrush(Qt::white));

	QColor color(0, 0, 0, 50);
	for (int i = 0; i < 10; i++)
	{
		QPainterPath path;
		path.setFillRule(Qt::WindingFill);
		path.addRect(10 - i, 10 - i, this->width() - (10 - i) * 2, this->height() - (10 - i) * 2);
		color.setAlpha(150 - qSqrt(i) * 50);
		painter.setPen(color);
		painter.drawPath(path);
	}

}
/************************************************************************/
/* 重置默认设置                                                                     */
/************************************************************************/
void YDBSettings::on_resetButton_clicked() {
	//ui->fpsCommon = 
	ui->simpleOutRecFormat->setCurrentIndex(0);
	ui->fpsCommon->setCurrentIndex(3);
	ui->simpleOutStrEncoder->setCurrentIndex(0);
	
	ui->simpleOutputABitrate->setCurrentIndex(ui->simpleOutputABitrate->count() / 2);
	ui->auxAudioDevice1->setCurrentIndex(1);
	ui->sampleRate->setCurrentIndex(0);
	ui->checkBox->setChecked(true);
	ui->simpleOutputVBitrate->setValue(500);
}
/************************************************************************/
/* 选择路径                                                                     */
/************************************************************************/
void YDBSettings::on_selectPathButton_clicked() {
	QString directory = QFileDialog::getExistingDirectory(this);
	if (directory.size() > 0) {
		ui->filePathEdit->setText(directory);
		ui->filePathEdit->setProperty("changed", QVariant(true));
	}
}
/************************************************************************/
/* 更新保存路径                                                                     */
/************************************************************************/
void YDBSettings::on_changeFilePathYes_clicked(){
	if (WidgetChanged(ui->filePathEdit)) {
		QString new_file_path = ui->filePathEdit->text();
		config_set_string(main->Config(), "SimpleOutput", "FilePath",
			QT_TO_UTF8(new_file_path));
		old_filePathEdit = new_file_path;
		//qst_path = "C:/Users/wenjie/Videos/Captures";

		//QMetaObject::invokeMethod(this->parentWidget(), "update_file_list", Qt::AutoConnection);
		update_file_list();

		wchar_t path_utf16[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
			path_utf16);
		char new_path[512] = "/obs-studio/basic/profiles/";

		strcat(new_path, Str("Untitled"));
		QString path1 = QString::fromWCharArray(path_utf16);
		QString path = path1 + new_path + "/basic.ini";
		local_ini = new QSettings(path, QSettings::IniFormat);

		update_config_local("SimpleOutput/FilePath", new_file_path);

		new_file_path = new_file_path.left(new_file_path.indexOf("/"));
		main->updateDriver(new_file_path);
		main->update_free_space();
	}
}
/************************************************************************/
/* 恢复原有的保存路径                                                                */
/************************************************************************/
void YDBSettings::on_changeFilePathCancel_clicked() {
	if (WidgetChanged(ui->filePathEdit)) {
		ui->filePathEdit->setText(old_filePathEdit);
		//QString new_file_path = ui->filePathEdit->text();
		//config_set_string(main->Config(), "SimpleOutput", "FilePath",
		//	QT_TO_UTF8(new_file_path));

		////qst_path = "C:/Users/wenjie/Videos/Captures";

		////QMetaObject::invokeMethod(this->parentWidget(), "update_file_list", Qt::AutoConnection);
		//update_file_list();
		//update_config_local("SimpleOutput/FilePath", new_file_path);

		//new_file_path = new_file_path.left(new_file_path.indexOf("/"));
		//main->updateDriver(new_file_path);
		//main->update_free_space();
	}
}
/************************************************************************/
/* 检查更新                                                                     */
/************************************************************************/
void YDBSettings::on_checkUpdateButton_clicked() {

	QByteArray postArray;

	postArray.append("&softKey=com.zbsd.obs");
	postArray.append("&deviceOS=windows");


	manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	QNetworkRequest request(QUrl("http://localhost:20000/System/latestVersion"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, postArray.size());
	manager->post(request, postArray);

}
/************************************************************************/
/* 检查更新响应函数                                                                     */
/************************************************************************/
void YDBSettings::replyFinished(QNetworkReply *reply) {
	QString all = reply->readAll();

	QJsonDocument d = QJsonDocument::fromJson(all.toUtf8());
	QJsonObject obj = d.object();
	QJsonValue value = obj.value("retObj");
	QJsonObject retObj;

	if (value.isObject()) {
		retObj = value.toObject();
		QJsonValue version = retObj.value("softVersion");

		QString version_qstr = version.toString();

		QStringList server_list = version_qstr.split(".");

		QString local_version = ui->version->text();
		QStringList local_list = local_version.split(".");

		for (int i = 0; i < local_list.size(); i++) {
			if (server_list.at(i).toInt() > local_list.at(i).toInt()) {
				//需要更新
				QString message = retObj.value("message").toString();

				QStringList message_list = message.split("\\n");
				int num = 1;
				for (QString test : message_list) {

					QLabel* label = new QLabel(test);
					label->setMaximumWidth(300);
					label->adjustSize();
					label->setWordWrap(true);
					label->setAlignment(Qt::AlignTop);
					ui->gridLayout_5->addWidget(label, num, 0, 1, 3);
					num++;

				}
				//ui->label_3->setText(message);
				download_url = retObj.value("softUpdateUrl").toString();
				//ui->pushButton->setVisible(true);
				QDesktopServices::openUrl(QUrl(download_url));

				break;
			}
		}

	}

}
/************************************************************************/
/* 全屏/恢复                                                                     */
/************************************************************************/
void YDBSettings::on_pushButton_5_clicked() {

	if (is_maximize) {

		this->showNormal();
		ui->pushButton_5->setStyleSheet("border-image: url(:/ydb/images/YDB/max.png);");
	}
	else {
		ui->pushButton_5->setStyleSheet("border-image: url(:/ydb/images/YDB/max-nor.png);");
		this->showMaximized();
	}
	is_maximize = !is_maximize;

}
YDBSettings::~YDBSettings()
{

	qDebug() << "close ydb settings dialog!";
	if (simpleOutRecFormat_changed || sampleRate_changed || fpsCommon_changed || simpleOutPreset_changed || displaySelect_changed
		|| simpleOutStrEncoder_changed || auxAudioDevice1_changed || simpleOutputABitrate_changed) {
		isChanged = true;
		wchar_t path_utf16[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
			path_utf16);
		char new_path[512] = "/obs-studio/basic/profiles/";

		strcat(new_path, Str("Untitled"));
		QString path1 = QString::fromWCharArray(path_utf16);
		QString path = path1 + new_path + "/basic.ini";
		local_ini = new QSettings(path, QSettings::IniFormat);
	}
	//采样频率(重进软件生效)
	if (sampleRate_changed) {
		//if (WidgetChanged(ui->sampleRate)) {
		qDebug() << "sampleRate save";
		QString sampleRateStr = ui->sampleRate->currentText();
		int sampleRate = 44100;
		if (sampleRateStr == "48khz")
			sampleRate = 48000;

		config_set_uint(main->Config(), "Audio", "SampleRate",
			sampleRate);
		update_config_local("Audio/SampleRate", QString::number(sampleRate));
	}

	//文件类型
	if (simpleOutRecFormat_changed) {
		qDebug() << "simpleOutRecFormat_changed save";
		SaveCombo(ui->simpleOutRecFormat, "SimpleOutput", "RecFormat");
		update_config_local("SimpleOutput/RecFormat", ui->simpleOutRecFormat->currentText());
	}

	//比特率
	if (simpleOutputABitrate_changed) {
		qDebug() << "sampleRate_changed save";
		SaveCombo(ui->simpleOutputABitrate, "SimpleOutput", "ABitrate");
		//SaveComboData(ui->simpleOutputABitrate, "SimpleOutput", "ABitrate");
		update_config_local("SimpleOutput/ABitrate", ui->simpleOutputABitrate->currentText());
	}

	//编码器预设
	QString encoder = ui->simpleOutStrEncoder->currentData().toString();
	if (simpleOutPreset_changed) {
		const char *presetType;
		if (encoder == SIMPLE_ENCODER_QSV)
			presetType = "QSVPreset";
		else if (encoder == SIMPLE_ENCODER_NVENC)
			presetType = "NVENCPreset";
		else
			presetType = "Preset";
		QString str = GetComboData(ui->simpleOutPreset);
		SaveComboData(ui->simpleOutPreset, "SimpleOutput", presetType);
		update_config_local(preset_key, QT_TO_UTF8(str));
	}
	//视频编码器
	if (simpleOutStrEncoder_changed) {
		qDebug() << "simpleOutStrEncoder_changed save";
		SaveComboData(ui->simpleOutStrEncoder, "SimpleOutput", "StreamEncoder");
		QString str = GetComboData(ui->simpleOutPreset);
		update_config_local("SimpleOutput/StreamEncoder", encoder);
	}
	//FPS
	if (fpsCommon_changed) {
		SaveCombo(ui->fpsCommon, "Video", "FPSCommon");
		update_config_local("Video/FPSCommon", ui->fpsCommon->currentText());
		//main->ResetVideo();
	}
	//显示器鼠标捕获
	if (captureCurosr_changed) {
		qDebug() << "captureCurosr_changed save";
		obs_data_set_bool(monitor_capture_settings, "capture_cursor", ui->checkBox->isChecked());

		//obs_data_set_bool(obs_source_get_settings(monitor_capture_source), "capture_cursor", ui->checkBox->isChecked());
	}
	//显示器选择
	if (displaySelect_changed) {
		int index = ui->displaySelect->currentIndex();
		obs_data_set_int(monitor_capture_settings, "monitor", index);

		OBSSceneItem scene_item = main->GetCurrentSceneItem();

		/*	obs_video_info ovi;
		obs_get_video_info(&ovi);*/



		vector<MonitorInfo> monitors;
		GetMonitors(monitors);
		MonitorInfo default_info = monitors.at(0);
		MonitorInfo monitor_info = monitors.at(index);
		uint32_t cx = monitor_info.cx;
		uint32_t cy = monitor_info.cy;

		config_set_uint(main->Config(), "Video", "BaseCX", cx);
		config_set_uint(main->Config(), "Video", "BaseCY", cy);
		update_config_local("Video/BaseCX", QString::number(cx));
		update_config_local("Video/BaseCY", QString::number(cy));

		config_set_uint(main->Config(), "Video", "OutputCX", cx);
		config_set_uint(main->Config(), "Video", "OutputCY", cy);
		update_config_local("Video/OutputCX", QString::number(cx));
		update_config_local("Video/OutputCY", QString::number(cy));

		vec2 pos;
		vec2_set(&pos, float(cx), float(cy));
		//vec2_set(&pos, float(default_info.cx), float(default_info.cy));

		obs_sceneitem_defer_update_begin(scene_item);
		obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_STRETCH);
		obs_sceneitem_set_bounds(scene_item, &pos);
		obs_sceneitem_defer_update_end(scene_item);
		//main->ResetVideo();
	}
	//来源	SaveSpinBox(ui->simpleOutputVBitrate, "SimpleOutput", "VBitrate");
	if (auxAudioDevice1_changed) {
		main->ResetAudioDevice(App()->InputAudioSource(), QT_TO_UTF8(GetComboData(ui->auxAudioDevice1)), Str("Basic.AuxDevice1"), 3);
	}
	if (simpleOutPreset_changed) {
		config_set_int(main->Config(), "SimpleOutput", "VBitrate", ui->simpleOutputVBitrate->value());
	}
	//应用改动
	if (fpsCommon_changed || displaySelect_changed)
		main->ResetVideo();
	if (captureCurosr_changed || displaySelect_changed) {
		obs_source_update(monitor_capture_source, monitor_capture_settings);
	}
	if (simpleOutRecFormat_changed || simpleOutputABitrate_changed || simpleOutPreset_changed
		|| simpleOutStrEncoder_changed) {
		main->ResetOutputs();
	}

	delete img;
    delete ui;
	properties.release();
	
	if (isChanged) {
		local_ini->sync();
		delete local_ini;
	}

	qDebug() << "~YDBSettings";
}


