#include "window-ydb-filepath.h"
#include "ui_YDBFilePath.h"





static inline bool WidgetChanged(QWidget *widget)
{
	return widget->property("changed").toBool();
}

YDBFilePath::YDBFilePath(QWidget *parent, OBSBasic* main) :
    QDialog(parent),
    ui(new Ui::YDBFilePath)
{
	this->main = main;
    ui->setupUi(this);
	/*wchar_t path_utf16[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);*/
	const char *path = config_get_string(main->Config(), "SimpleOutput",
		"FilePath");
	ui->filePathEdit->setText(path);
	
}
/************************************************************************/
/* Ñ¡ÔñÂ·¾¶                                                                     */
/************************************************************************/
void YDBFilePath::on_selectPathButton_clicked() {
	QString directory = QFileDialog::getExistingDirectory(this);
	ui->filePathEdit->setText(directory);
	ui->filePathEdit->setProperty("changed", QVariant(true));
}

void YDBFilePath::accept() {
	if (WidgetChanged(ui->filePathEdit)) {
		QString new_file_path = ui->filePathEdit->text();
		config_set_string(main->Config(), "SimpleOutput", "FilePath",
			QT_TO_UTF8(new_file_path));
		
		//qst_path = "C:/Users/wenjie/Videos/Captures";

		QMetaObject::invokeMethod(this->parentWidget(), "update_file_list", Qt::AutoConnection);
		
		update_config_local("SimpleOutput/FilePath", new_file_path);
		
		new_file_path = new_file_path.left(new_file_path.indexOf("/"));
		main->updateDriver(new_file_path);
		main->update_free_space();
	}
	QDialog::accept();
}

void YDBFilePath::update_config_local(QString key, QString value) {
 
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);
	char new_path[512] = "/obs-studio/basic/profiles/";
 
	strcat(new_path, Str("Untitled"));
	QString path1 = QString::fromWCharArray(path_utf16);
	QString path = path1 + new_path + "/basic.ini";
	local_ini = new QSettings(path, QSettings::IniFormat);
	QString str2 = local_ini->value("SimpleOutput/FilePath").toString();
	local_ini->setValue(key, value);
	local_ini->sync();
	delete local_ini;
 
}

YDBFilePath::~YDBFilePath()
{
    delete ui;
	
	//delete path_utf16;
	//_CrtDumpMemoryLeaks();
}

