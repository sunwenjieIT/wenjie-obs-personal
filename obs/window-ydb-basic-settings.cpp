#include "window-ydb-basic-settings.h"
#include "ui_YDBBasicSettings.h"
#include <QDebug>
#include <QComboBox>
#include <initializer_list>
#include <sstream>
#include "audio-encoders.hpp"
#include "QFile"
#include "QDebug"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#include "platform.hpp"
//#include "window-basic-preview.cpp"
//#include "window-basic-main.hpp"
//#include "properties-view.cpp"

using namespace std;
QString qJsonObjectToQString(QJsonObject obj) {
	return QString(QJsonDocument(obj).toJson());
}
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
YDBBasicSettings::YDBBasicSettings(QWidget *parent, OBSSource monitor_capture_source) :
    QDialog(parent),
    ui(new Ui::YDBBasicSettings),
	properties(obs_source_properties(monitor_capture_source), obs_properties_destroy)
{
    ui->setupUi(this);
	ui->comboBox_7->setVisible(false);
	ui->label_7->setVisible(false);

	this->monitor_capture_source = monitor_capture_source;
	monitor_capture_settings = obs_source_get_settings(monitor_capture_source);
	main = qobject_cast<OBSBasic*>(parent);


	//connect(ui->simpleOutputVBitrate, SIGNAL()
	//connect(ui->simpleOutputVBitrate, SIGNAL(valueChanged(int)), this, SLOT(on_simpleOutputVBitrate_changed(int)));

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

	////文件类型
	//const char *format = config_get_string(main->Config(), "AdvOut",
	//	"RecFormat");
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
	//this->properties.reset
	//obs_properties_t* properties = obs_source_properties(monitor_capture_source);
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
}
void YDBBasicSettings::SimpleRecordingEncoderChanged() {

}
void YDBBasicSettings::SimpleStreamingEncoderChanged() {
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
void YDBBasicSettings::on_buttonBox_clicked(QAbstractButton* button) {
	QDialogButtonBox::ButtonRole val = ui->buttonBox->buttonRole(button);
	
	if (val == QDialogButtonBox::ResetRole) {
		if (main->recording_status == OBSBasic::Pausing) {
			return;
		}
		//ui->fpsCommon = 
		ui->simpleOutRecFormat->setCurrentIndex(0);
		ui->fpsCommon->setCurrentIndex(3);
		ui->simpleOutStrEncoder->setCurrentIndex(0);
		ui->simpleOutputABitrate->setCurrentIndex(11);
		ui->auxAudioDevice1->setCurrentIndex(1);
		ui->sampleRate->setCurrentIndex(0);
		ui->checkBox->setChecked(true);
		ui->simpleOutputVBitrate->setValue(500);
	}
}
void YDBBasicSettings::accept() {
	qDebug() << "accept!";
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

		OBSSceneItem scene_item= main->GetCurrentSceneItem();

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
	QDialog::accept();
}

void YDBBasicSettings::on_simpleOutputVBitrate_valueChanged(int value) {
	if (is_init_done && value != old_simpleOutputVBitrate_value) {
		sender()->setProperty("changed", QVariant(true));
		simpleOutPreset_changed = true;
	}
}

void YDBBasicSettings::on_simpleOutRecFormat_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutRecFormat_idx) {
		qDebug() << "on_simpleOutRecFormat_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutRecFormat_changed = true;
	}
}
void YDBBasicSettings::on_simpleOutPreset_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutPreset_idx) {
		sender()->setProperty("changed", QVariant(true));
		simpleOutPreset_changed = true;
	}
}
void YDBBasicSettings::on_sampleRate_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_sampleRate_idx) {
		qDebug() << "on_sampleRate_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		sampleRate_changed = true;
	}
}
void YDBBasicSettings::on_simpleOutStrEncoder_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutStrEncoder_idx) {
		qDebug() << "on_simpleOutStrEncoder_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutStrEncoder_changed = true;
	}
}
void YDBBasicSettings::on_fpsCommon_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_fpsCommon_idx) {
		qDebug() << "on_fpsCommon_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		fpsCommon_changed = true;
	}
}
void YDBBasicSettings::on_auxAudioDevice1_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_auxAudioDevice1_idx) {
		qDebug() << "on_auxAudioDevice1_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		auxAudioDevice1_changed = true;
	}
}
void YDBBasicSettings::on_simpleOutputABitrate_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_simpleOutputABitrate_idx) {
		qDebug() << "on_simpleOutputABitrate_currentIndexChanged " << idx;
		sender()->setProperty("changed", QVariant(true));
		simpleOutputABitrate_changed = true;
	}
}
void YDBBasicSettings::on_checkBox_toggled(bool visible) {
	if (is_init_done && is_capture_cursor != visible) {
		qDebug() << captureCurosr_changed;
		captureCurosr_changed = true;
	}
}
void YDBBasicSettings::on_displaySelect_currentIndexChanged(int idx) {
	if (is_init_done && idx != old_displaySelect_idx) {
		qDebug() << "on_displaySelect_currentIndexChanged";
		sender()->setProperty("changed", QVariant(true));
		displaySelect_changed = true;
	}
}
void YDBBasicSettings::LoadListValues(QComboBox *widget, obs_property_t *prop,
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

void YDBBasicSettings::update_config_local(QString key, QString value) {

	/*wchar_t path_utf16[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);
	char new_path[512] = "/obs-studio/basic/profiles/";

	strcat(new_path, Str("Untitled"));
	QString path1 = QString::fromWCharArray(path_utf16);
	QString path = path1 + new_path + "/basic.ini";
	QSettings* local_ini = new QSettings(path, QSettings::IniFormat);*/
	QString str2 = local_ini->value("SimpleOutput/FilePath").toString();
	local_ini->setValue(key, value);
	local_ini->sync();
	//delete local_ini;
	//delete local_ini;

}
void YDBBasicSettings::update_json_local(QString a, QString b) {
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
	QString obj_qstt = qJsonObjectToQString(obj);
	qDebug() << val;
	file.resize(0);
	file.write(obj_qstt.toUtf8());
	file.close();

}
void YDBBasicSettings::SaveCombo(QComboBox *widget, const char *section,
	const char *value)
{
	if (WidgetChanged(widget))
		config_set_string(main->Config(), section, value,
			QT_TO_UTF8(widget->currentText()));
}
void YDBBasicSettings::SaveComboData(QComboBox *widget, const char *section,
	const char *value)
{
	if (WidgetChanged(widget)) {
		QString str = GetComboData(widget);
		config_set_string(main->Config(), section, value,
			QT_TO_UTF8(str));
	}
}
YDBBasicSettings::~YDBBasicSettings()
{
    delete ui;
	properties.release();
	//obs_data_release(monitor_capture_settings);
	//obs_source_release(monitor_capture_source);
	/*if (monitor_capture_settings)
		obs_data_release(monitor_capture_settings);
	if (monitor_capture_source)
		obs_source_release(monitor_capture_source);*/

	if (isChanged) {
		local_ini->sync();
		delete local_ini;
	}
}
