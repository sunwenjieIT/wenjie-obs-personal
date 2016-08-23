#include "uploadfiledialog.hpp"
#include "ui_uploadfiledialog.h"

#include "window-basic-main.hpp"

#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <qfile.h>

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <util/threading.h>
#define NUM_THREADS 5
#pragma comment(lib,"pthreadVC2.lib") 


struct thread_data
{
	oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;
	int threadNum;
	int start;
	int end;
	int startPart;
	int endPart;
	QString successKey;
	QString partListKey;
	QList<QVariant> uploadedPartList;
	//QStringList uploadedPartList;
	//QStringList *uploadedPartList;
	QSettings *iniSetting;
};

extern void append_object_sample();
extern void delete_object_sample();
extern void put_object_sample();
extern void get_object_sample();
extern void head_object_sample();
extern void multipart_object_sample();


//
//#include "sdk/aos_http_io.c"

char* getChar(QString& str)
{
	QByteArray temp_ba;

	temp_ba = str.toUtf8();
	//temp_ba.da
	char* data = temp_ba.data();
	char* ret = new char[strlen(data) + 1];
	strcpy(ret, data);
	return ret;
}

QList<QVariant> getDefaultList()
{
	QList<QVariant> defaultList;
	defaultList.append(0);
	defaultList.append(10001);
	return defaultList;
}

UploadFileDialog::UploadFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadFileDialog)
{
    ui->setupUi(this);
	defaultList.append(0);
	defaultList.append(10001);
	//struct thread_data td[NUM_THREADS];
}

//extern void easyMultipart_upload();
void UploadFileDialog::on_uploadButton_clicked()
{
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
		exit(1);
	}

	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	oss_request_options_t *options = NULL;
	aos_status_t *s = NULL;
	int part_size = 5 * 1024 * 1024;
	aos_string_t upload_id;
	aos_string_t filepath;
	int is_cname = 0;
	aos_pool_create(&p, NULL);

	/* 创建并初始化options */
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	//init_options(options);
	aos_str_set(&bucket, "wenjie-backet");
	//aos_str_set()
	
	

	char *char2 = "obs-upload2.txt";
	
	//aos_str_set(&object, getChar(file_name));
	//aos_str_set(&object, "obs-upload1.txt");
	aos_str_null(&upload_id);
	//aos_str_set(&upload_id, "9B79B397977341C7AE0740E115A52F1F");//断点续传
	blog(LOG_INFO, "file local path:@@%s@@", getChar(file_path));
	
	//char *char1 = "D:/windows_version2-1.txt";
	
	
	//aos_str_set(&filepath, char1);
	
	
	
	QByteArray te2 = file_name.toUtf8();
	char *filenamechar = te2.data();
	//char* filenamechar = getChar(file_name);
	blog(LOG_INFO, "file name: %s", file_name);
	
	//aos_str_set(&object, file_name_char);
	//aos_str_set(&object, getChar(file_name));

	//char* filenamechar = getChar(file_name);
	aos_str_set(&object, filenamechar);

	QByteArray te = file_path.toUtf8();
	char *filepathchar = te.data();

	//char* filepathchar = getChar(file_full);
	//aos_str_set(&filepath, getChar(file_full));
	aos_str_set(&filepath, filepathchar);
	//aos_str_set(&filepath, filepathchar);
	//aos_str_set(&filepath, "D:/windows_version2-1.txt");

	/* 分片上传 */
	s = oss_upload_file(options, &bucket, &object, &upload_id, &filepath,
		part_size, NULL);

	/* 判断是否上传成功 */
	if (aos_status_is_ok(s)) {
		//printf("upload file succeeded\n");
		QMessageBox::information(NULL, "title", "upload success", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	}
	else {
		//printf("upload file failed\n");
		blog(LOG_INFO, "failed %s %s", s->error_code, s->error_msg);
		QMessageBox::information(NULL, "title", "upload failed", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

	}
	

	/* 释放资源*/
	aos_pool_destroy(p);



	aos_http_io_deinitialize();
}

/************************************************************************/
/* 选择文件                                                                     */
/************************************************************************/
void UploadFileDialog::on_selectFileButton_clicked()
{
	file_path = QFileDialog::getOpenFileName(this, "open file", "/", "textfile(*.txt);;C file(*.cpp);;All file(*.*)");
	
	ui->filePath->setText(file_path.toUtf8());
}

void UploadFileDialog::on_iniButton_clicked()
{
	QString str = QFileDialog::getOpenFileName(this, "open file", "/", "textfile(*.txt);;C file(*.cpp);;All file(*.*)");

	QFileInfo info;
	info = QFileInfo(str);

	QSettings *settings;
	settings = new QSettings("D:/test2.ini", QSettings::IniFormat);

	QString test2;
	test2 = "abc";
	bool hasKey = settings->contains(str);
	bool hasKey2 = settings->contains("abc");
	bool hasKey3 = settings->contains(QString("123"));
	

	QStringList list = settings->allKeys();
	
	blog(LOG_INFO, "test and is has key? %d\n", hasKey);

}

void* say_hello(void* args)
{
	int thread_num;
	oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;
	QList<QVariant> list;
	//QStringList *list;
	//QString qfilePath;

	struct thread_data *my_data;
	my_data = (struct thread_data *)args;
	bucket = my_data->bucket;
	object = my_data->object;
	upload_id = my_data->upload_id;
	resp_headers = my_data->resp_headers;
	options = my_data->options;
	upload_file = my_data->upload_file;
	list = my_data->uploadedPartList;
	thread_num = my_data->threadNum;

	QSettings *threadSetting;
	
	QSettings *testSetting;
	testSetting = new QSettings("D:/test4.ini", QSettings::IniFormat);



	int po = my_data->start;
	int partSize = 1024 * 1024 * 5;
	aos_status_t *s = NULL;
	int last = po + partSize;

	upload_file->file_pos = my_data->start;//起始偏移量
	upload_file->file_last = my_data->endPart - my_data->startPart == 0 ? my_data->end : my_data->start + partSize;//第一个分块偏移量
	blog(LOG_INFO, "thread %d start upload %s", thread_num, getChar(my_data->successKey));
	int cout = 0;
	int failCout = 0;
	for (int n = my_data->startPart; n <= my_data->endPart; n++)
	{
		//判断是否需要上传
		/*if (n <= 5) {
			testSetting->setValue(my_data->successKey, false);
			testSetting->sync();
		}*/
		if (!list.contains(n))
			//if (!list.contains(n) && n > 5)
		{
			//上传
			s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
				n, upload_file, &resp_headers);
			//判断上传是否成功
			if (aos_status_is_ok(s))
			{
				blog(LOG_INFO, "Multipart upload from file success thread %d part %d\n", my_data->threadNum, n);
				cout++;
				list.append(n);
				failCout = 0;
				if (cout % 3 == 0)
				{
					testSetting->setValue(my_data->partListKey, list);
					blog(LOG_INFO, "upload ini file update from thread %s", my_data->threadNum);
				}
				//po = last;
			}
			else
			{
				if (++failCout < 4) {
					n--;
					blog(LOG_INFO, "retry thread %d upload part %d , error_code:%s error_msg:%s \n", my_data->threadNum, n, s->error_code, s->error_msg);
					continue;
				}
				testSetting->setValue(my_data->successKey, false);
				testSetting->sync();
				blog(LOG_INFO, "thread %d upload part %d failed error_code:%s error_msg:%s \n", my_data->threadNum, n, s->error_code, s->error_msg);
			}
		}
		//更新偏移量
		upload_file->file_pos = upload_file->file_last;
		upload_file->file_last = n + 1 == my_data->endPart ? my_data->end : upload_file->file_last + partSize;
	}
	

	testSetting->setValue(my_data->partListKey, list);

	blog(LOG_INFO, "upload ini file update from thread %d", thread_num);
	//blog(LOG_INFO, "thread %d start upload", thread_num);
	blog(LOG_INFO, "done start %d end %d now %d\n", my_data->start, my_data->end, upload_file->file_pos);

	pthread_exit(NULL);
	blog(LOG_INFO, "thread exit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%d", my_data->threadNum);
	free(upload_file);
	free(options);
	free(resp_headers);
	free(&upload_id);
	free(&bucket);
	free(&object);
	free(my_data);

	delete threadSetting;
	delete testSetting;
	pthread_exit(NULL);
	/************************************************************************/
	/* oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;                                                                     */
	/************************************************************************/
	
	return NULL;
}

void UploadFileDialog::on_pushButton_clicked()
{
	file_path = ui->filePath->text();
	file_path_char = getChar(file_path);
	QFileInfo fi;
	fi = QFileInfo(file_path);
	file_name = fi.fileName();
	file_name_char = getChar(file_name);
	file_root_path = fi.absolutePath();
	//获取文件大小
	total_size = get_file_size(file_path_char);
	if (total_size == -1){
		blog(LOG_INFO, "文件不存在");
		return;
	}
	if (NULL != uploadIni)
		delete uploadIni;
	uploadIni = new QSettings("D:/test4.ini", QSettings::IniFormat);
	
	aos_pool_t *plist[NUM_THREADS];
	oss_request_options_t *optionslist[NUM_THREADS];

	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	aos_table_t *headers = NULL;
	aos_table_t *resp_headers = NULL;
	aos_table_t *complete_headers = NULL;
	oss_request_options_t *options = NULL;
	aos_string_t upload_id;
	oss_upload_file_t *upload_file = NULL;
	aos_status_t *s = NULL;
	oss_list_upload_part_params_t *params = NULL;
	aos_list_t complete_part_list;
	oss_list_part_content_t *part_content = NULL;
	oss_complete_part_content_t *complete_part_content = NULL;
	int is_cname = 0;
	int part_num1 = 1;


	pthread_t threads[NUM_THREADS];		//工作线程
	struct thread_data td[NUM_THREADS];	//线程参数结构体
	int threads_num;	//线程数
	//isResumeUpload = uploadIni->contains(file_path);//是否断点续传 第一次判断 绝对路径
	isResumeUpload = uploadIni->contains(file_path) && total_size == uploadIni->value(file_path + "/total_size").toLongLong();
	if (isResumeUpload)
	{
		//if (total_size == uploadIni->value(file_path + "/total_size").toLongLong())	//第二次判断 文件大小
		//{
		total_part = uploadIni->value(file_path + "/total_part").toInt();
		uploadId = getChar(uploadIni->value(file_path).toString());
		e_thread_parts = uploadIni->value(file_path + "/e_thread_parts").toInt();
		l_thread_parts = uploadIni->value(file_path + "/l_thread_parts").toInt();
		blog(LOG_INFO, uploadId);

		//线程数		已上传的分块信息
		threads_num = uploadIni->beginReadArray(file_path + "/thread");
		for (int i = 0; i < threads_num; ++i)
		{
			uploadIni->setArrayIndex(i);
			//uploadIni->value("partlist").toList();
			td[i].uploadedPartList = uploadIni->value("partlist").toList();
			//td[i].uploadedPartList = &uploadIni->value("partlist").toStringList();

		}
		uploadIni->endArray();
		aos_str_set(&upload_id, uploadId);
		/*}
		else
		{
			isResumeUpload = false;
		}*/
	}
	else {
		total_part = (total_size + partSize - 1) / partSize; //分块 总块数
		threads_num = total_part > 5 ? 5 : total_part;
		e_thread_parts = total_part / threads_num; //除最后一个线程外每个线程的任务量
		l_thread_parts = e_thread_parts + total_part % threads_num; //最后一个线程的任务量
		for (int i = 0; i < threads_num; ++i)
		{
			td[i].uploadedPartList = getDefaultList();

		}
	}
	//if (isResumeUpload)
	//{
	//	//线程数		已上传的分块信息
	//	threads_num = uploadIni->beginReadArray(file_path + "/thread");
	//	for (int i = 0; i < threads_num; ++i)
	//	{
	//		uploadIni->setArrayIndex(i);
	//		td[i].uploadedPartList = uploadIni->value("partlist").toStringList();
	//		//td[i].uploadedPartList = &uploadIni->value("partlist").toStringList();

	//	}
	//	uploadIni->endArray();
	//	aos_str_set(&upload_id, uploadId);
	//}
	//else
	//{
	//	total_part = (total_size + partSize - 1) / partSize; //分块 总块数
	//	threads_num = total_part > 5 ? 5 : total_part;
	//	e_thread_parts = total_part / threads_num; //除最后一个线程外每个线程的任务量
	//	l_thread_parts = e_thread_parts + total_part % threads_num; //最后一个线程的任务量
	//	for (int i = 0; i < threads_num; ++i)
	//	{
	//		QStringList list;
	//		td[i].uploadedPartList = list;
	//		//td[i].uploadedPartList = new QStringList();

	//	}
	//}
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
		exit(1);
	}
	
	
	

	aos_str_set(&bucket, "wenjie-backet");
	
	aos_str_set(&object, file_name_char);

	//aos_str_set(&object, getChar(file_name));//可行
	//aos_str_set(&object, file_name.toUtf8().data());//报错
	//aos_str_set(&object, "thread10.mp4");
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/ActivePerl_5.16.2.3010812913.msi";
	
	//char UPLOAD_FILE[] = file_path.toUtf8().data();
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/one piece715-dmxz.zerodm.com.mp4";
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/ftpserver-1.0.6.zip";

	//int64_t totalSize = get_file_size(UPLOAD_FILE);
	

	aos_pool_create(&p, NULL);
	/* 创建并初始化options */
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	//init_options(options);
	/* 初始化参数 */
	headers = aos_table_make(p, 1);



	
	//aos_str_set(&upload_id, "76E295F65D274DDFA032881E03C89536");

	if (!isResumeUpload) {
		s = oss_init_multipart_upload(options, &bucket, &object,
			&upload_id, headers, &resp_headers);
		/* 判断是否初始化分片上传成功 */
		if (aos_status_is_ok(s)) {
			if (!isResumeUpload) {
				uploadIni->setValue(file_path, upload_id.data);
				uploadIni->setValue(file_path + "/total_part", total_part);
				uploadIni->setValue(file_path + "/total_size", total_size);
				uploadIni->setValue(file_path + "/e_thread_parts", e_thread_parts);
				uploadIni->setValue(file_path + "/l_thread_parts", l_thread_parts);
			}
			blog(LOG_INFO, "Init multipart upload succeeded, upload_id:%.*s\n",
				upload_id.len, upload_id.data);
		}
		else {
			blog(LOG_INFO, "Init multipart upload failed, upload_id:%.*s\n",
				upload_id.len, upload_id.data);
		}
	}

	
	
	int rc;
	//int i;

	pthread_attr_t attr;
	void *status;
	
	blog(LOG_INFO, "size:%I64d, num:%d\n", total_size, total_part);
	blog(LOG_INFO, "e_thread_parts %d\n l_thread_parts %d ", e_thread_parts, l_thread_parts);
	//int thread_nums = total_part / threads_num; //除最后一个线程外每个线程的任务量
	//int lastThread = thread_nums + total_part % threads_num; //最后一个线程的任务量

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	//QList<QVariant> defaultList;
	//defaultList.append(0);
	//defaultList.append(10001);
	blog(LOG_INFO, "start");
	for (int i = 0; i < threads_num; ++i) {
		td[i].threadNum = i + 1;
		td[i].startPart = i * e_thread_parts + 1;
		td[i].endPart = i + 1 == threads_num ? i * e_thread_parts + l_thread_parts : (i + 1) * e_thread_parts;
		
		

		//QString temp1 = file_path + "/thread/";
		//QString temp2 = QString::number(i+1) + "/partlist";
		//QString temp = temp1 + temp2;
		td[i].partListKey = file_path + "/thread/" + (QString::number(i+1) + "/partlist");
		//td[i].arrayPath = file_path + "/thread/" + i + "/partlist";

		//td[i].uploadedPartList = 
		
		/* 创建并初始化options */
		aos_pool_create(&plist[i], NULL);
		optionslist[i] = oss_request_options_create(plist[i]);
		init_sample_request_options(optionslist[i], is_cname);
		td[i].options = optionslist[i];

		//td[i].options = options;

		td[i].upload_id = upload_id;
		td[i].bucket = bucket;
		td[i].object = object;

		//upload_file = oss_create_upload_file(p);
		upload_file = oss_create_upload_file(plist[i]);

		aos_str_set(&upload_file->filename, file_path_char);
		//aos_str_set(&upload_file->filename, UPLOAD_FILE);
		
		td[i].iniSetting = uploadIni;

		td[i].successKey = file_path + "/thread/" + (QString::number(i + 1) + "/success");

		td[i].upload_file = upload_file;
		td[i].resp_headers = resp_headers;
		td[i].start = (td[i].startPart - 1) * partSize;
		td[i].end = i + 1 == threads_num ? total_size : td[i].start + partSize * e_thread_parts;

		uploadIni->setValue(td[i].partListKey, defaultList);
		uploadIni->setValue(td[i].successKey, true);
		uploadIni->sync();
		rc = pthread_create(&threads[i], NULL, say_hello, (void *)&td[i]);
		if (i + 1 == threads_num) {
			uploadIni->setValue(file_path + "/thread/size", threads_num);
			uploadIni->sync();
		}

		if (rc) {
			blog(LOG_ERROR, "ERROR: unable to create thread, %d\n", rc);
			exit(-1);
		}
	}
	//WaitForSingleObject()
	// 删除属性，并等待其他线程
	pthread_attr_destroy(&attr);
	for (int i = 0; i < threads_num; i++) {
		rc = pthread_join(threads[i], &status);
		if (rc) {
			blog(LOG_ERROR, "ERROR: unable to join, : %d\n", rc);
			exit(-1);
		}
		blog(LOG_INFO, "Main: completed thread id : %d\n", i);
		blog(LOG_INFO, "exiting with status: %d\n", status);
	}

	/************************************************************************/
	/* 判断是否完成上传                                                                     */
	/************************************************************************/
	int iniThreadSize = uploadIni->beginReadArray(file_path + "/thread");
	if (iniThreadSize != threads_num) {
		//TUDO 未知错误 上传线程数与ini文件里的线程数不匹配
		
	}
	bool isFinishUpload = true;
	for (int i = 0; i < iniThreadSize; ++i) {
		uploadIni->setArrayIndex(i);
		if (!uploadIni->value("success").toBool()) {
			isFinishUpload = false;
			QMessageBox::information(NULL, "upload message", "upload success", QMessageBox::Yes, QMessageBox::Yes);
		}
	}
	uploadIni->endArray();

	if (isFinishUpload) {
		/* 完成分片上传*/
		params = oss_create_list_upload_part_params(p);
		params->max_ret = 1000;
		aos_list_init(&complete_part_list);
		s = oss_list_upload_part(options, &bucket, &object, &upload_id, params
			, &resp_headers);
		if (aos_status_is_ok(s)) {
			blog(LOG_INFO, "List multipart secceeded %d \n", s->req_id);
		}
		else {
			blog(LOG_INFO, "List multipart failed");
		}

		aos_list_for_each_entry(oss_list_part_content_t, part_content, &params->part_list, node) {
			complete_part_content = oss_create_complete_part_content(p);
			aos_str_set(&complete_part_content->part_number,
				part_content->part_number.data);
			aos_str_set(&complete_part_content->etag, part_content->etag.data);
			aos_list_add_tail(&complete_part_content->node, &complete_part_list);

		}

		s = oss_complete_multipart_upload(options, &bucket, &object,
			&upload_id, &complete_part_list, complete_headers, &resp_headers);

		if (aos_status_is_ok(s)) {
			blog(LOG_INFO, "Complete multipart upload from file succeeded, upload_id:%.*s\n",
				upload_id.len, upload_id.data);
			
			QMessageBox::information(NULL, "upload message", "upload success", QMessageBox::Yes, QMessageBox::Yes);

		}
		else {
			blog(LOG_INFO, "Complete multipart upload from file failed\n");
		}
	}
	else {
		blog(LOG_INFO, "file multipart upload have part left");
	}
	//struct thread_data test;
	aos_http_io_deinitialize();
	blog(LOG_INFO, "finish upload\n");
	//pthread_exit(NULL);
}




void UploadFileDialog::easyUpload()
{
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
		exit(1);
	}

	QSettings *settings;
	settings = new QSettings("D:/test2.ini", QSettings::IniFormat);

	

	aos_http_io_deinitialize();

}

UploadFileDialog::~UploadFileDialog()
{
	//delete uploadId;
	delete uploadIni;
    delete ui;
}