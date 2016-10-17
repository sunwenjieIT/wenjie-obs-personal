#include "YDBUpdate.h"
#include "ui_YDBUpdate.h"

#include "util/base.h"
#include <util/util.hpp>
#include <libff/ff-util.h>
#include "window-basic-main.hpp"
#include <QMessageBox>

#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
//#include <QNetworkReply>
//#include <QNetworkAccessManager>



YDBUpdate::YDBUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::YDBUpdate)
{
    ui->setupUi(this);
	this->setWindowFlags(Qt::WindowCloseButtonHint);
	ui->pushButton->setVisible(false);
	if (updateCheckThread) {
		updateCheckThread->wait();
		delete updateCheckThread;
	}


	QByteArray postArray;

	postArray.append("&softKey=com.zbsd.obs");
	postArray.append("&deviceOS=windows");


	manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	QNetworkRequest request(QUrl("http://localhost:20000/System/latestVersion"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, postArray.size());
	manager->post(request, postArray);
	
	//MyClassNet* test = new MyClassNet;
	


}

void YDBUpdate::replyFinished(QNetworkReply *reply) {
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

		for (int i = 0; i < local_list.size(); i++){
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
					ui->gridLayout_3->addWidget(label, num, 0, 1, 3);
					num++;
					
				}
				//ui->label_3->setText(message);
				download_url = retObj.value("softUpdateUrl").toString();
				ui->pushButton->setVisible(true);
				break;
			}
		}

	}

}

void YDBUpdate::on_pushButton_clicked() {
	QDesktopServices::openUrl(QUrl(download_url));
}

#define VERSION_ENTRY "windows"
void YDBUpdate::updateFileFinished(const QString &text, const QString &error) {
	//ui->actionCheckForUpdates->setEnabled(true);

	if (text.isEmpty()) {
		blog(LOG_WARNING, "Update check failed: %s", QT_TO_UTF8(error));
		return;
	}

	obs_data_t *returnData = obs_data_create_from_json(QT_TO_UTF8(text));
	obs_data_t *versionData = obs_data_get_obj(returnData, VERSION_ENTRY);
	const char *description = obs_data_get_string(returnData,
		"description");
	const char *download = obs_data_get_string(versionData, "download");

	if (returnData && versionData && description && download) {
		long major = obs_data_get_int(versionData, "major");
		long minor = obs_data_get_int(versionData, "minor");
		long patch = obs_data_get_int(versionData, "patch");
		long version = MAKE_SEMANTIC_VERSION(major, minor, patch);

		blog(LOG_INFO, "Update check: last known remote version "
			"is %ld.%ld.%ld",
			major, minor, patch);

		if (version > LIBOBS_API_VER) {
			QString     str = QTStr("UpdateAvailable.Text");
			QMessageBox messageBox(this);

			str = str.arg(QString::number(major),
				QString::number(minor),
				QString::number(patch),
				download);

			messageBox.setWindowTitle(QTStr("UpdateAvailable"));
			messageBox.setTextFormat(Qt::RichText);
			messageBox.setText(str);
			messageBox.setInformativeText(QT_UTF8(description));
			messageBox.exec();

			long long t = (long long)time(nullptr);
			config_set_int(App()->GlobalConfig(), "General",
				"LastUpdateCheck", t);
			config_save_safe(App()->GlobalConfig(), "tmp", nullptr);
		}
	}
	else {
		blog(LOG_WARNING, "Bad JSON file received from server");
	}

	obs_data_release(versionData);
	obs_data_release(returnData);
}

YDBUpdate::~YDBUpdate()
{
    delete ui;
	delete manager;
}
