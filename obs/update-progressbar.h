#pragma once
#include <QProgressBar>
#include <QThreadPool>
#include <QThread>
#include <QFileInfo>
#include <QProgressBar>
#include <QMessageBox>
#include <QSettings>
#include <QList>
#include "MyInt64Mq.h"
#define HAVE_STRUCT_TIMESPEC


//#define NUM_THREADS 5
//#include <QTest>
//#include "window-basic-main.hpp"
class UploadTask :public QThread {
	Q_OBJECT
public:
	UploadTask(QObject *parent, int threadNum, QString successKey, QString patrListKey, char* upload_id, char* upload_file_char, char* object_char, int start_part, int64_t end_length, QList<QVariant> uploaded_list, MyInt64Mq* mq);
	~UploadTask();
	void run();
private:
	QSettings *testSetting = NULL;
	int threadNum;
	MyInt64Mq* mq = NULL;
	QString successKey;
	QString partListKey;
	char* upload_id; 
	char* upload_file_char;
	char* object_char;
	int start_part;
	int64_t end_length;
	QList<QVariant> uploaded_list;
};

class RunnableProgressBarTask : public QThread
{
	Q_OBJECT
public:
    RunnableProgressBarTask(QObject *parent, QProgressBar* progressBar, MyInt64Mq* mq);
	~RunnableProgressBarTask()
	{
		blog(LOG_INFO, "~progress thread");
	}
    void run();

private:
    QProgressBar* m_ProgressBar = NULL;
	MyInt64Mq* m_mq = NULL;

};

class MyClass :public QThread
{
	Q_OBJECT
public:
	MyClass(QObject *parent, QString filePath, QProgressBar* progressBar, MyInt64Mq* mq);
	MyClass(QObject *parent, QString filePath, QProgressBar* progressBar);
	~MyClass();
	void run();
	bool check_upload_file();
	QSettings* init_upload_local_config(int& threads_num);
	

signals:
	void testSignal(int);

public slots:
void stopThread();
void updateProgressBar();
private:
	/*enum finish_code {
		DONE = 0, HAVE_MORE, COMPELETE_ERROR,
	};*/
	QList<QVariant> getDefaultList()
	{
		QList<QVariant> defaultList;
		defaultList.append(0);
		defaultList.append(10001);
		return defaultList;
	}
private:
	int threads_num = 0;
	QList<QVariant> lists[5];
	QProgressBar* progressBar = NULL;
	QSettings* settings = NULL;
	RunnableProgressBarTask* test = NULL;
	UploadTask* upload_threads[5] = { NULL,NULL,NULL,NULL,NULL };
	MyInt64Mq* mq = NULL;
	int partSize = 5242880;
	QString file_name, file_root_path, file_path;
	char *file_name_char = NULL; char *file_path_char = NULL;
	int64_t total_size;
	bool isResumeUpload;
	int total_part;
	int e_thread_parts;
	int l_thread_parts;
	char *uploadId = NULL;
	bool is_done = false;
	bool is_delete_mq = false;
};
