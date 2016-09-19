#include "window-ydb-settings.h"
#include "ui_YDBSettings.h"
#include <QDir>
#include <QTextCodec>
#include <QDebug>
#include <windows.h>
#include "obs-app.hpp"

#include <QDateTime>
#include <QFile>

YDBSettings::YDBSettings(QWidget *parent, OBSSource monitor_capture_source) :
    QDialog(parent),
    ui(new Ui::YDBSettings)
{
	main = qobject_cast<OBSBasic*>(parent);
	this->monitor_capture_source = monitor_capture_source;
    ui->setupUi(this);
	//this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
	update_file_list();
	//setWindowFlags(windowFlags()&~Qt::CustomizeWindowHint);
	//this->setWindowFlags(Qt::FramelessWindowHint);
	//this->setWindowFlags(Qt::FramelessWindowHint);
	//this->setWindowTitle("Qt5.1 窗体应用");
	setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);
	//ui->horizontalLayout->:/icon/images/obs.png
	QIcon icon;
	icon.addFile(QStringLiteral(":/res/images/obs.png"), QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(icon);
	//this->setwindowc
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
	ui->listWidget->clear();
	for (unsigned int i = 0; i < myDir.count(); i++)
	{
		sext = myDir[i].toLower();
		int point = sext.lastIndexOf(".");
		if (point != -1) {
			file_type = sext.mid(point);
			if (file_type == ".flv" || file_type == ".mp4" || file_type == ".mov" || file_type == ".mkv"
				|| file_type == ".ts" || file_type == ".m3u8" ){
				ui->listWidget->addItem(sext);
			}
		}
		//flv,mp4,mov,mkv,ts,m3u8
		/*if (-1 != sext.indexOf(".flv") || -1 != sext.indexOf(".mp4") || -1 != sext.indexOf(".mov")
			|| -1 != sext.indexOf(".mkv") || -1 != sext.indexOf(".ts") || -1 != sext.indexOf(".m3u8")) {
			ui->listWidget->addItem(sext);
		}*/
	}
	if (hasItem()) {
		file_path = directory_path + ui->listWidget->item(0)->text();
		ui->listWidget->itemClicked(ui->listWidget->item(0));
	}
}
void YDBSettings::update_file_name(QString file_name) {
	item->setText(file_name);

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
	return ui->listWidget->count() > 0;
}
/************************************************************************/
/* 点击item                                                                     */
/************************************************************************/
void YDBSettings::on_listWidget_itemClicked(QListWidgetItem* item) {
	if (NULL == item) return;
	ui->listWidget->setItemSelected(item, true);
	
	file_path = directory_path + item->text();
	//file_path = "C:/Users/wenjie/Videos/" + item->text();
	this->item = item;
	QString text = item->text();
	//TODO
	QString filepathQstring = directory_path + text;
	//QString filepathQstring = "C:/Users/wenjie/Videos/" + text;
	QFileInfo fileInfo = QFileInfo(filepathQstring);
	double size_byte = (double)(fileInfo.size() * 100 / 1024 / 1024) / 100;
	ui->label_9->setText(QString::number(size_byte) + " M");
	AVFormatContext *pFormatCtx;
	int             i, videoindex, PictureSize;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame *pFrame, *pFrameRGB;
	AVPacket packet;
	int ret, got_picture;
	char* filepath = YDBUtil::getLocal8BitChar(filepathQstring);
	int64_t time_, time_total;

	struct SwsContext *pSwsCtx;
	uint8_t *outBuff;
	time_total = av_gettime_relative();
	av_register_all();
	avformat_network_init();

	pFormatCtx = avformat_alloc_context();
	time_ = av_gettime_relative();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		qDebug() << "Couldn't open input stream.";
		return;
	}
	
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		qDebug() << "Couldn't find stream information.";
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
		qDebug() << "Didn't find a video stream.";
		return;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		qDebug() << "Codec not found.";
		return;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		qDebug() << "Could not open codec.";
		return;
	}

	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	if (pFrame == NULL || pFrameRGB == NULL)
	{
		qDebug() << "avframe malloc failed!";
		return;
	}
	//PictureSize = avpicture_get_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
	//TODO
	PictureSize = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height, 1);
	outBuff = (uint8_t*)av_malloc(PictureSize * sizeof(uint8_t));

	if (outBuff == NULL) {
		qDebug() << "av malloc failed!";
		printf("av malloc failed!\n");
		return;
	}
	avpicture_fill((AVPicture *)pFrameRGB, outBuff, AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, outBuff, AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height, 1);

	pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);
	time_ = av_gettime_relative() - time_;
	qDebug() << "open file and find info, cost time : " << time_ / 1000000.0;
	printf("open file and find info, cost time: %0.3fs\n", time_ / 1000000.0);

	time_ = av_gettime_relative();
	int64_t timestamp = atoi("1");
	//int64_t timestamp = atoi(argv[1]);
	timestamp = av_rescale(timestamp, pFormatCtx->streams[videoindex]->time_base.den, (int64_t)pFormatCtx->streams[videoindex]->time_base.num);

	av_seek_frame(pFormatCtx, videoindex, timestamp, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[videoindex]->codec);
	time_ = av_gettime_relative() - time_;
	qDebug() << "seek frame, costs time:" << time_ / 1000000.0;
	printf("seek frame, costs time: %0.3fs\n", time_ / 1000000.0);
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = NULL;
	avpkt.size = 0;

	time_ = av_gettime_relative();
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoindex) {
			if (packet.flags) {
				if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet)) < 0) {
					qDebug() << "Decode Error!";
					printf("Decode Error!\n");
					return;
				}
				while (got_picture == 0) {
					Sleep(10);
					if ((ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &avpkt)) < 0) {
						printf("Decode Error!\n");
						qDebug() << "Decode Error!";
						return;
					}
				}
				if (got_picture) {
					time_ = av_gettime_relative() - time_;
					qDebug() << "read and decode frame, costs time:" << time_ / 1000000.0;

					time_ = av_gettime_relative();
					sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
					//if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], argv[3], pCodecCtx->width, pCodecCtx->height) < 0) {
					if (SavetoJPEG(pFrameRGB, pFormatCtx->streams[videoindex], filepath, pCodecCtx->width, pCodecCtx->height) < 0) {
						qDebug() << "Write Image Error!";
						return;
					}
					ui->label_7->setText(QString::number(pFormatCtx->bit_rate / 1000) + "kbps");
					QTime t = QTime::fromMSecsSinceStartOfDay(pFormatCtx->duration / 1000);
					ui->label_8->setText(t.toString("hh:mm:ss"));

					time_ = av_gettime_relative() - time_;
					qDebug() << "write frame, costs time" << time_ / 1000000.0;
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
	qDebug() << "all done, costs time:" << time_total / 1000000.0;
	//QMessageBox::information(NULL, "success title", "picture success!", QMessageBox::Yes, QMessageBox::Yes);

}
/************************************************************************/
/* 我的视频                                                                     */
/************************************************************************/
void YDBSettings::on_videoButton_clicked() {
	//ui->listWidget->isHidden
	bool isHidden = ui->listWidget->isHidden();
	if (isHidden)
		ui->listWidget->show();
	else
		ui->listWidget->hide();
}
/************************************************************************/
/* 录制设置                                                                     */
/************************************************************************/
void YDBSettings::on_videoSettingsButton_clicked() {
	YDBBasicSettings ydb_basic_settings(this->parentWidget(), monitor_capture_source);
	ydb_basic_settings.exec();
	//QMetaObject::invokeMethod(this->parentWidget(), "on_settingsButton_clicked", Qt::AutoConnection);
}
/************************************************************************/
/* 文件保存设置                                                                     */
/************************************************************************/
void YDBSettings::on_pathSettingsButton_clicked() {
	YDBFilePath pathDialog(this, main);
	pathDialog.exec();
}
/************************************************************************/
/* 关于                                                                     */
/************************************************************************/
void YDBSettings::on_aboutButton_clicked() {
	YDBUpdate update;
	update.exec();
}
/************************************************************************/
/* 帮助                                                                     */
/************************************************************************/
void YDBSettings::on_helpButton_clicked() {

}
/************************************************************************/
/* 文件删除                                                                     */
/************************************************************************/
void YDBSettings::on_deleteButton_clicked() {
	if (NULL == item) return;
	qDebug() << file_path;

	ui->listWidget->removeItemWidget(item);
	delete item;
	item = NULL;
	if (QFile::remove(file_path)) {
		if (hasItem()) {
			on_listWidget_itemClicked(ui->listWidget->item(0));
		}
		else {
			ui->horizontalLayout_3->removeWidget(show_img);
			ui->label_7->clear();
			ui->label_8->clear();
			ui->label_9->clear();
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
	if (NULL == item) return;
	QString file_name = item->text();
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
	pCodecCtx->time_base.num = pVStream->codec->time_base.num;
	pCodecCtx->time_base.den = pVStream->codec->time_base.den;
	//Output some information  
	//av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		qDebug() << "Codec not found.";
		printf("Codec not found.");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.");
		qDebug() << "Could not open codec.";
		return -1;
	}
	pCodecCtx->qmin = pCodecCtx->qmax = 3;
	pCodecCtx->flags |= CODEC_FLAG_QSCALE;
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
		qDebug() << "Encode Error.";
		printf("Encode Error.\n");
		return -1;
	}
	if (got_picture == 1) {
		pkt.stream_index = video_st->index;
		uint8_t* pkt_data = pkt.data;

		
		QByteArray q_byte_array;

		q_byte_array.append((char*)pkt_data, pkt.size);
		//pkt.data
		if (img->loadFromData(q_byte_array, "JPG")) {
			if (NULL == show_img) {
				show_img = new ShowImg(NULL, img, QString(video_path));
				ui->horizontalLayout_3->addWidget(show_img);
				//ui->horizontalLayout_3->r
			}
			else {
				QVariant qtest = QVariant::fromValue(img);
				//QImage* a = qtest.value<QImage*>();
				QMetaObject::invokeMethod(show_img, "update_image", Qt::AutoConnection,
					Q_ARG(QVariant, qtest), Q_ARG(QString, QString(video_path)));
			}
			
		}
	}

	av_packet_unref(&pkt);
	//Write Trailer  
	av_write_trailer(pFormatCtx);
	//pFormatCtx->
	qDebug() << "Encode Successful.";
	printf("Encode Successful.\n");

	if (video_st) {
		avcodec_close(video_st->codec);
		av_free(picture);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);
	
	return 0;
}

YDBSettings::~YDBSettings()
{
	delete img;
    delete ui;
	qDebug() << "~YDBSettings";
}


