#ifndef UPLOADFILEDIALOG_H
#define UPLOADFILEDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <qfile.h>


namespace Ui {
class UploadFileDialog;
}


struct thread_data2
{
	/*oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;*/


	int threadNum;
	int start;
	int end;
	int startPart;
	int endPart;
	QStringList uploadedPartList;
	QSettings *iniSetting;
};
struct testabc
{
	int64_t totalsize;
	int b;
	int c;
};
class UploadFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadFileDialog(QWidget *parent = 0);
    ~UploadFileDialog();

public slots:
	void on_uploadButton_clicked();
	void on_selectFileButton_clicked();
	void on_iniButton_clicked();
	void on_pushButton_clicked();

public:
	QString file_name, file_root_path, file_path;
	char *file_name_char, *file_path_char;
	QSettings *uploadIni = NULL;
	int partSize = 5242880; //1024 * 1024 * 5	5MB
	int64_t total_size;
	bool isResumeUpload;
	int total_part;
	int e_thread_parts;
	int l_thread_parts;
	char *uploadId;
	//QStringl
	struct testabc tabc[5];
	struct thread_data2 td2[5];
	QList<QVariant> defaultList;

public:
	void easyUpload();

private:
    Ui::UploadFileDialog *ui;
};

#endif // UPLOADFILEDIALOG_H
