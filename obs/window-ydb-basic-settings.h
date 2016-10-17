#ifndef WINDOWYDBBASICSETTINGS_H
#define WINDOWYDBBASICSETTINGS_H

#include <QDialog>
#include "qt-wrappers.hpp"
#include "window-basic-main.hpp"
#include <libff/ff-util.h>
#include <util/util.hpp>
//#include "window-basic-settings.cpp"

namespace Ui {
class YDBBasicSettings;
}

class YDBBasicSettings : public QDialog
{
    Q_OBJECT

	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t =
		std::unique_ptr<obs_properties_t, properties_delete_t>;

public:
    explicit YDBBasicSettings(QWidget *parent = 0, OBSSource monitor_capture_source = NULL);
    ~YDBBasicSettings();
	void LoadListValues(QComboBox *widget, obs_property_t *prop,
		int index);
	virtual void accept();
private:
    Ui::YDBBasicSettings *ui;
	properties_t properties;
	OBSBasic* main;
	OBSSource monitor_capture_source;
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
	//bool 
public slots:
void on_simpleOutRecFormat_currentIndexChanged(int idx);
void on_fpsCommon_currentIndexChanged(int idx);
void on_simpleOutStrEncoder_currentIndexChanged(int idx);
void on_simpleOutPreset_currentIndexChanged(int idx);
void on_sampleRate_currentIndexChanged(int idx);
void on_auxAudioDevice1_currentIndexChanged(int idx);
void on_simpleOutputABitrate_currentIndexChanged(int idx);
void SimpleStreamingEncoderChanged();
void SimpleRecordingEncoderChanged();
void on_checkBox_toggled(bool visible);
void on_displaySelect_currentIndexChanged(int idx);
void on_buttonBox_clicked(QAbstractButton* button);
void on_simpleOutputVBitrate_valueChanged(int value);
};

#endif // WINDOWYDBBASICSETTINGS_H
