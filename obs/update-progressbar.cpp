#include "update-progressbar.h"
//#include <WinBase.h>
#include <windows.h>
#include "util/base.h"

#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

#include "upload_config.c"

extern void append_object_sample();
extern void delete_object_sample();
extern void put_object_sample();
extern void get_object_sample();
extern void head_object_sample();
extern void multipart_object_sample();

#include <pthread.h>
#define NUM_THREADS 5
#pragma comment(lib,"pthreadVC2.lib") 
pthread_t threads[NUM_THREADS];

char* getLocal8BitChar(QString &str)
{
	QByteArray temp_ba;
	temp_ba = str.toLocal8Bit();
	//temp_ba.da
	char* data = temp_ba.data();
	char* ret = new char[strlen(data) + 1];
	strcpy(ret, data);
	return ret;
}
char* getLatin1Char(QString &str)
{
	QByteArray temp_ba;
	temp_ba = str.toLatin1();
	//temp_ba.da
	char* data = temp_ba.data();
	char* ret = new char[strlen(data) + 1];
	strcpy(ret, data);
	return ret;
}
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

RunnableProgressBarTask::RunnableProgressBarTask(QObject *parent, QProgressBar* progressBar, MyInt64Mq* mq)
{
	m_ProgressBar = progressBar;
	m_mq = mq;
}

void RunnableProgressBarTask::run()
{
	int64_t tmp = 0;
	int cout = 0;
	int oldtype;
	/*pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);*/
	while (true)
	{
		//pthread_testcancel();
		tmp = m_mq->getMessage();
		if (tmp > 0) {
			cout = cout + tmp;
			QMetaObject::invokeMethod(m_ProgressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, cout));
			blog(LOG_INFO, "refreshProgressBar value %d", cout);
		}
		else if (tmp == -1) {
			break;
		}
		Sleep(100);
	}
	blog(LOG_INFO, "refreshProgressBar finish");
	
}

void MyClass::updateProgressBar() {
	//this->mq.
		/*int64_t tmp = this->mq->getMessage();
	if (tmp > 0) {

	}
	tmp = m_mq->getMessage();
	if (tmp > 0) {
		cout = cout + tmp;
		QMetaObject::invokeMethod(m_ProgressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, cout));
		blog(LOG_INFO, "refreshProgressBar value %d", cout);
	}*/
}



void MyClass::run()
{
	if (!check_upload_file()) {
		blog(LOG_INFO, "file not exist");
		emit testSignal(6);//文件异常
		exit(1);
		return;
	}
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
		emit testSignal(5);//网络异常
		exit(1);
		return;
	}
	mq = new MyInt64Mq();
	progressBar->setRange(0, total_size);

	aos_pool_t *p;
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
	char *key = NULL;
	int is_cname = 0;
	int rc;

	key = (char*)malloc(strlen(DIR_NAME) + strlen(file_name_char) + 1);
	strcpy(key, DIR_NAME);
	strcat(key, file_name_char);

	aos_str_set(&bucket, BUCKET_NAME);
	aos_str_set(&object, key);
	//aos_str_set(&object, file_name_char);
	aos_pool_create(&p, NULL);
	/* 创建并初始化options */
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);

	/* 初始化参数 */
	headers = aos_table_make(p, 1);
	QSettings *uploadIni;	//
	uploadIni = init_upload_local_config(threads_num);

	if (!isResumeUpload) {
		s = oss_init_multipart_upload(options, &bucket, &object,
			&upload_id, headers, &resp_headers);
		/* 判断是否初始化分片上传成功 */
		if (aos_status_is_ok(s)) {
			uploadIni->setValue(file_path, upload_id.data);
			uploadIni->setValue(file_path + "/is_finish", false);
			uploadIni->setValue(file_path + "/total_part", total_part);
			uploadIni->setValue(file_path + "/total_size", total_size);
			uploadIni->setValue(file_path + "/e_thread_parts", e_thread_parts);
			uploadIni->setValue(file_path + "/l_thread_parts", l_thread_parts);
			uploadIni->beginWriteArray(file_path + "/thread");
			for (int i = 0; i < threads_num; ++i)
			{
				lists[i] = getDefaultList();
				uploadIni->setArrayIndex(i);
				uploadIni->setValue("partlist", lists[i]);
			}
			uploadIni->endArray();
			uploadIni->sync();
			blog(LOG_INFO, "Init multipart upload succeeded, upload_id:%.*s\n",
				upload_id.len, upload_id.data);
		}
		else {
			//string a;
			QString a;
			a.sprintf("init upload failed!error_code: %s error_msg: %s", s->error_code, s->error_msg);
			blog(LOG_INFO, "Init multipart upload failed! error_code: %s, error_msg: %s", s->error_code, s->error_msg);
			emit testSignal(4);//初始化失败
			return;
		}
	}
	else {
		aos_str_set(&upload_id, uploadId);
		threads_num = uploadIni->beginReadArray(file_path + "/thread");
		for (int i = 0; i < threads_num; ++i)
		{
			uploadIni->setArrayIndex(i);
			lists[i] = uploadIni->value("partlist").toList();
		}
		uploadIni->endArray();
	}
	blog(LOG_INFO, "size:%I64d, num:%d\n", total_size, total_part);
	blog(LOG_INFO, "e_thread_parts %d\n l_thread_parts %d ", e_thread_parts, l_thread_parts);
	//启动进度条线程
	test = new RunnableProgressBarTask(this, progressBar, mq);
	connect(test, SIGNAL(finished()), test, SLOT(deleteLater()));
	test->start();
	blog(LOG_INFO, "start");
	for (int i = 0; i < threads_num; ++i) {
		
		upload_file = oss_create_upload_file(p);
		aos_str_set(&upload_file->filename, file_path_char);
		QString successKey = file_path + "/thread/" + (QString::number(i + 1) + "/success");
		QString partListKey = file_path + "/thread/" + (QString::number(i + 1) + "/partlist");
		
		int start_part = i * e_thread_parts + 1;
		int64_t start_length = (start_part - 1) * partSize;
		int64_t end_length = i + 1 == threads_num ? total_size : start_length + partSize * e_thread_parts;
		if(!isResumeUpload) uploadIni->setValue(successKey, false);//默认上传未完成
	
		/************************************************************************/
		/* 用qthread重构                                                                     */
		/************************************************************************/
		upload_threads[i] = new UploadTask(this, i, successKey, partListKey, upload_id.data,
			file_path_char, key, start_part, end_length, lists[i], mq);
		/*	upload_threads[i] = new UploadTask(this, i, successKey, partListKey, upload_id.data,
				file_path_char, file_name_char, start_part, end_length, lists[i], mq);*/
		connect(upload_threads[i], SIGNAL(finished()), upload_threads[i], SLOT(deleteLater()), Qt::QueuedConnection);
		upload_threads[i]->start();
		blog(LOG_INFO, "upload thread %d start", i);

	}
	/************************************************************************/
	/* qthread线程等待                                                                     */
	/************************************************************************/
	for (int i = 0; i< threads_num; i++){
		if(NULL != upload_threads[i] && upload_threads[i]->isRunning()) upload_threads[i]->wait();
		blog(LOG_INFO, "Main: completed thread id : %d\n", i);
	}
	//上传的工作线程结束
	is_done = true;
	//进度条线程终止信号
	mq->putMessage(-1);
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
		}
	}
	uploadIni->endArray();

	if (isFinishUpload && !is_delete_mq) {
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
			blog(LOG_INFO, "List multipart failed, error_code:%s error_msg:%s ", s->error_code, s->error_msg);
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
			uploadIni->setValue(file_path + "/is_finish", true);
			uploadIni->sync();
			emit testSignal(0);//完成所有上传
		}
		else {
			blog(LOG_INFO, "Complete multipart upload from file failed error_code:%s error_msg:%s ", s->error_code, s->error_msg);
			emit testSignal(2);//完成上传失败
		}
	}
	else {
		blog(LOG_INFO, "file multipart upload have part left");
		emit testSignal(1);//完成部分上传
	}
	//释放资源
	blog(LOG_INFO, "task run method aos http io deinitialize");
	uploadIni->sync();
	aos_http_io_deinitialize();
	for (int i = 0; i< threads_num; ++i){
		if (NULL != upload_threads[i]) delete upload_threads[i];
	}
	
	if(NULL != test){
		test->wait();
		delete test;
	}
	blog(LOG_INFO, "delete mq in work thread");
	delete mq;
	blog(LOG_INFO, "finish upload\n");
}
/************************************************************************/
/* 关闭线程                                                                     */
/************************************************************************/
void MyClass::stopThread() {

	
	//threads
	is_delete_mq = true;
	blog(LOG_INFO, "work thread stop slot ");
	for (int i = 0; i < threads_num; i++) {
		//void *status;
		if (NULL != upload_threads[i] && upload_threads[i]->isRunning()) {
			upload_threads[i]->terminate();
			blog(LOG_INFO, "message thread %d terminate over", i);
			upload_threads[i]->wait();
			blog(LOG_INFO, "message thread %d wait over", i);
		}
	}

	if (NULL != test && test->isRunning()) {
		test->terminate();
		blog(LOG_INFO, "terminate progress thread over");
		test->wait();
		blog(LOG_INFO, "wait progress thread over");
	}

	blog(LOG_INFO, "stop thread over");
	//delete mq;
}

MyClass::MyClass(QObject *parent,QString file_path, QProgressBar* progressBar, MyInt64Mq* mq)
{
	//connect(parent, SIGNAL(stopTaskThread()), this, SLOT(stopThread()));
	this->progressBar = progressBar;
	this->mq = mq;
	this->file_path = file_path;
	//this->filePath = filePath;
}
MyClass::MyClass(QObject *parent, QString file_path, QProgressBar* progressBar)
{
	this->progressBar = progressBar;
	this->file_path = file_path;
}
MyClass::~MyClass()
{
	delete file_path_char;
	delete file_name_char;
	delete uploadId;
	blog(LOG_INFO, "~myclass");
	//delete progressBar;
}
/************************************************************************/
/* 检查文件完整性                                                                     */
/************************************************************************/
bool MyClass::check_upload_file()
{
	file_path_char = getLocal8BitChar(file_path);
	QFileInfo fi;
	fi = QFileInfo(file_path);
	file_name = fi.fileName();
	QFile qfile(file_path);
	qint64 q_total_size = qfile.size();

	file_name_char = getLocal8BitChar(file_name);

	//std::string test_str = file_name.toStdString();
	//const char* aaaa = test_str.c_str();

	file_name_char = getLatin1Char(file_name);
	//file_name_char = getChar(file_name);

	file_root_path = fi.absolutePath();

	total_size = q_total_size;

	//total_size = get_file_size(file_path_char);
	return total_size != -1;
	//return total_size != -1;
}
/************************************************************************/
/* 初始化本地信息                                                                       */
/************************************************************************/
QSettings* MyClass::init_upload_local_config(int& threads_num) {
	QSettings* uploadIni = new QSettings(UPLOAD_INI_PATH, QSettings::IniFormat);
	//QSettings* uploadIni = new QSettings("D:/test4.ini", QSettings::IniFormat);

	isResumeUpload = uploadIni->contains(file_path) && total_size == uploadIni->value(file_path + "/total_size").toLongLong() ? !uploadIni->value(file_path + "/is_finish").toBool() : false;
	if (isResumeUpload)
	{
		total_part = uploadIni->value(file_path + "/total_part").toInt();
		uploadId = getLocal8BitChar(uploadIni->value(file_path).toString());
		e_thread_parts = uploadIni->value(file_path + "/e_thread_parts").toInt();
		l_thread_parts = uploadIni->value(file_path + "/l_thread_parts").toInt();
		blog(LOG_INFO, uploadId);
	}
	else {
		total_part = (total_size + partSize - 1) / partSize; //分块 总块数
		threads_num = total_part > 5 ? 5 : total_part;
		e_thread_parts = total_part / threads_num; //除最后一个线程外每个线程的任务量
		l_thread_parts = e_thread_parts + total_part % threads_num; //最后一个线程的任务量
	}
	return uploadIni;
}

UploadTask::UploadTask(QObject *parent, int threadNum, QString successKey, QString patrListKey, char* upload_id, char* upload_file_char, char* object_char, int start_part, int64_t end_length, QList<QVariant> uploaded_list, MyInt64Mq* mq) {
	this->threadNum = threadNum;
	this->successKey = successKey;
	this->partListKey = patrListKey;
	this->upload_id = upload_id;
	this->upload_file_char = upload_file_char;
	this->object_char = object_char;
	this->start_part = start_part;
	this->end_length = end_length;
	this->uploaded_list = uploaded_list;
	this->mq = mq;
}
UploadTask::~UploadTask() {
	blog(LOG_INFO, "~uploadtask ");
	if (NULL != testSetting) {
		blog(LOG_INFO, "delete qsettings");
	}
}
void UploadTask::run() {
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	int is_cname = 0;
	aos_table_t *headers = NULL;
	aos_table_t *complete_headers = NULL;
	aos_table_t *resp_headers = NULL;
	oss_request_options_t *options = NULL;
	aos_string_t upload_id;
	oss_upload_file_t *upload_file = NULL;
	aos_status_t *s = NULL;
	oss_list_upload_part_params_t *params = NULL;
	aos_list_t complete_part_list;
	oss_list_part_content_t *part_content = NULL;
	oss_complete_part_content_t *complete_part_content = NULL;
	int part_num = this->start_part;
	int partSize = 1024 * 1024 * 5;
	int64_t pos = (part_num - 1) * partSize;
	int64_t end_length = this->end_length;

	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	headers = aos_table_make(p, 1);
	complete_headers = aos_table_make(p, 1);
	aos_str_set(&bucket, BUCKET_NAME);
	aos_str_set(&object, this->object_char);
	aos_str_set(&upload_id, this->upload_id);

	//QSettings *testSetting;
	testSetting = new QSettings(UPLOAD_INI_PATH, QSettings::IniFormat);

	int retry_cout = 0;
	int fail_cout = 0;
	int success_cout = 0;
	while (pos < end_length) {

		if (this->uploaded_list.contains(part_num)) {
			part_num++;
			pos += partSize;
			mq->putMessage(pos > end_length ? end_length - pos + partSize : partSize);
			blog(LOG_INFO, "contains %d", part_num);
			continue;
		}

		upload_file = oss_create_upload_file(p);
		aos_str_set(&upload_file->filename, this->upload_file_char);
		upload_file->file_pos = pos;
		
		upload_file->file_last = pos + partSize < end_length ? pos + partSize : end_length; 
		s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
			part_num, upload_file, &resp_headers);

		if (aos_status_is_ok(s)) {
			mq->putMessage(upload_file->file_last - upload_file->file_pos);
			blog(LOG_INFO, "Multipart upload from file success thread %d part %d\n", threadNum, part_num);
			uploaded_list.append(part_num);
			retry_cout = 0, success_cout++;
			testSetting->setValue(this->partListKey, uploaded_list);
			testSetting->sync();
			/*if (success_cout % 3 == 0)
			{
				testSetting->setValue(this->partListKey, uploaded_list);
				blog(LOG_INFO, "upload ini file update from thread %d", threadNum);
			}*/
		}
		else {
			if (++retry_cout < 4) {
				blog(LOG_INFO, "retry thread %d upload part %d , error_code:%s error_msg:%s \n", threadNum, part_num, s->error_code, s->error_msg);
				continue;
			}
			retry_cout = 0, fail_cout++;
			
			blog(LOG_INFO, "thread %d upload part %d failed error_code:%s error_msg:%s \n", threadNum, part_num, s->error_code, s->error_msg);
		}
		pos += partSize;
		part_num++;
	}
	if(fail_cout == 0) testSetting->setValue(this->successKey, true);
	//testSetting->setValue(this->partListKey, uploaded_list);
	testSetting->sync();
	blog(LOG_INFO, "upload ini file update from thread %d", threadNum);
	blog(LOG_INFO, "upload thread %d finish, start part num: %d success: %d, fail: %d", threadNum, part_num, success_cout, fail_cout);
	//blog(LOG_INFO, "thread %d start upload", thread_num);
	//blog(LOG_INFO, "done start %d end %d now %d\n", my_data->start, my_data->end, upload_file->file_pos);
	
}
