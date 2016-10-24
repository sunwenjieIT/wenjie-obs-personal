#include "volume-control.hpp"
#include "qt-wrappers.hpp"
#include "mute-checkbox.hpp"
#include "slider-absoluteset-style.hpp"
#include <util/platform.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QSlider>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <string>
#include <math.h>

using namespace std;

void VolControl::OBSVolumeChanged(void *data, float db)
{
	Q_UNUSED(db);
	VolControl *volControl = static_cast<VolControl*>(data);

	QMetaObject::invokeMethod(volControl, "VolumeChanged");
}

void VolControl::OBSVolumeLevel(void *data, float level, float mag,
			float peak, float muted)
{
	VolControl *volControl = static_cast<VolControl*>(data);

	QMetaObject::invokeMethod(volControl, "VolumeLevel",
		Q_ARG(float, mag),
		Q_ARG(float, level),
		Q_ARG(float, peak),
		Q_ARG(bool,  muted));
}

void VolControl::OBSVolumeMuted(void *data, calldata_t *calldata)
{
	VolControl *volControl = static_cast<VolControl*>(data);
	bool muted = calldata_bool(calldata, "muted");

	QMetaObject::invokeMethod(volControl, "VolumeMuted",
			Q_ARG(bool, muted));
}

void VolControl::VolumeChanged()
{
	slider->blockSignals(true);
	//slider->setValue((int)(obs_fader_get_deflection(obs_fader) * 100.0f));
	slider->setValue((int)(obs_fader_get_deflection(obs_fader) * 150.0f));
	slider->blockSignals(false);
	
	updateText();
}

void VolControl::VolumeLevel(float mag, float peak, float peakHold, bool muted)
{
	if (muted) {
		mag = 0.0f;
		peak = 0.0f;
		peakHold = 0.0f;
	}

	volMeter->setLevels(mag, peak, peakHold);
}

void VolControl::VolumeMuted(bool muted)
{
	if (mute->isChecked() != muted)
		mute->setChecked(muted);
}

void VolControl::SetMuted(bool checked)
{
	obs_source_set_muted(source, checked);
}

void VolControl::SliderChanged(int vol)
{
	obs_fader_set_deflection(obs_fader, float(vol) * 0.01f);
	updateText();
}

void VolControl::updateText()
{
	volLabel->setText(QString::number(obs_fader_get_db(obs_fader), 'f', 1)
			.append(" dB"));
}

QString VolControl::GetName() const
{
	return nameLabel->text();
}

void VolControl::SetName(const QString &newName)
{
	nameLabel->setText(newName);
}

void VolControl::EmitConfigClicked()
{
	emit ConfigClicked();
}

VolControl::VolControl(OBSSource source_, bool showConfig, bool showDetail)
	: source        (source_),
	  levelTotal    (0.0f),
	  levelCount    (0.0f),
	  obs_fader     (obs_fader_create(OBS_FADER_CUBIC)),
	  obs_volmeter  (obs_volmeter_create(OBS_FADER_LOG))
{

	QHBoxLayout *volLayout  = new QHBoxLayout();
	QVBoxLayout *mainLayout = new QVBoxLayout();
	QHBoxLayout *textLayout = new QHBoxLayout();
	QHBoxLayout *botLayout  = new QHBoxLayout();

	nameLabel = new QLabel();
	volLabel  = new QLabel();
	volMeter  = new VolumeMeter();
	mute      = new MuteCheckBox();
	slider    = new QSlider(Qt::Horizontal);

	QFont font = nameLabel->font();
	font.setPointSize(font.pointSize()-1);

	nameLabel->setText(obs_source_get_name(source));
	nameLabel->setFont(font);
	volLabel->setFont(font);
	slider->setMinimum(0);
	slider->setMaximum(150);
	//slider->setMaximum(100);

//	slider->setMaximumHeight(13);

	textLayout->setContentsMargins(0, 0, 0, 0);
	textLayout->addWidget(nameLabel);
	textLayout->addWidget(volLabel);
	textLayout->setAlignment(nameLabel, Qt::AlignLeft);
	textLayout->setAlignment(volLabel,  Qt::AlignRight);

	mute->setChecked(obs_source_muted(source));

	volLayout->addWidget(slider);
	volLayout->addWidget(mute);
	volLayout->setSpacing(5);

	botLayout->setContentsMargins(0, 0, 0, 0);
	botLayout->setSpacing(0);
	botLayout->addLayout(volLayout);

	if (showConfig) {
		config = new QPushButton(this);
		config->setProperty("themeID", "configIconSmall");
		config->setFlat(true);
		config->setSizePolicy(QSizePolicy::Maximum,
				QSizePolicy::Maximum);
		config->setMaximumSize(22, 22);
		config->setAutoDefault(false);

		connect(config, &QAbstractButton::clicked,
				this, &VolControl::EmitConfigClicked);

		botLayout->addWidget(config);
	}

	mainLayout->setContentsMargins(4, 4, 4, 4);
	mainLayout->setSpacing(2);
	mainLayout->addItem(textLayout);
	mainLayout->addWidget(volMeter);
	mainLayout->addItem(botLayout);

	setLayout(mainLayout);

	obs_fader_add_callback(obs_fader, OBSVolumeChanged, this);
	obs_volmeter_add_callback(obs_volmeter, OBSVolumeLevel, this);

	signal_handler_connect(obs_source_get_signal_handler(source),
			"mute", OBSVolumeMuted, this);

	QWidget::connect(slider, SIGNAL(valueChanged(int)),
			this, SLOT(SliderChanged(int)));
	QWidget::connect(mute, SIGNAL(clicked(bool)),
			this, SLOT(SetMuted(bool)));

	obs_fader_attach_source(obs_fader, source);
	obs_volmeter_attach_source(obs_volmeter, source);

	slider->setStyle(new SliderAbsoluteSetStyle(slider->style()));

	/* Call volume changed once to init the slider position and label */
	VolumeChanged();
	
	if (!showDetail) {
		nameLabel->setVisible(false);
		mute->setVisible(false);
		mute->setChecked(false);
		slider->setVisible(false);
		
		volLabel->setVisible(false);
		
		//volMeter->setVisible(false);
		//volMeter->setMinimumWidth(70);
		//volMeter->setMaximumWidth(70);

		volMeter->setMinimumWidth(150);
		volMeter->setMaximumWidth(150);
		QColor blackColor("black");

		
		//QColor color = volMeter->getBkColor();
		//QColor testColor;
	}
}

VolControl::~VolControl()
{
	obs_fader_remove_callback(obs_fader, OBSVolumeChanged, this);
	obs_volmeter_remove_callback(obs_volmeter, OBSVolumeLevel, this);

	signal_handler_disconnect(obs_source_get_signal_handler(source),
			"mute", OBSVolumeMuted, this);

	obs_fader_destroy(obs_fader);
	obs_volmeter_destroy(obs_volmeter);
}

QColor VolumeMeter::getBkColor() const
{
	return bkColor;
}

void VolumeMeter::setBkColor(QColor c)
{
	bkColor = c;
}

QColor VolumeMeter::getMagColor() const
{
	return magColor;
}

void VolumeMeter::setMagColor(QColor c)
{
	magColor = c;
}

QColor VolumeMeter::getPeakColor() const
{
	return peakColor;
}

void VolumeMeter::setPeakColor(QColor c)
{
	peakColor = c;
}

QColor VolumeMeter::getPeakHoldColor() const
{
	return peakHoldColor;
}

void VolumeMeter::setPeakHoldColor(QColor c)
{
	peakHoldColor = c;
}


VolumeMeter::VolumeMeter(QWidget *parent)
			: QWidget(parent)
{
	//setMinimumSize(2, 3);
	setMinimumSize(1, 3);

	//Default meter color settings, they only show if there is no stylesheet, do not remove.

	//bkColor.setRgb(0xDD, 0xDD, 0xDD);
	//magColor.setRgb(0x20, 0x7D, 0x17);
	//peakColor.setRgb(0x3E, 0xF1, 0x2B);
	
	bkColor.setRgb(0x70, 0xFE, 0xD6);	//背景颜色
	magColor.setRgb(0xFF, 0xFF, 0xFF);	//中间部分
	peakColor.setRgb(0xFF, 0xFF, 0xFF);	//顶部渐变部分
	
	peakHoldColor.setRgb(0xFF, 0x00, 0x00);
	resetTimer = new QTimer(this);
	connect(resetTimer, SIGNAL(timeout()), this, SLOT(resetState()));

	resetState();
}

void VolumeMeter::resetState(void)
{
	setLevels(0.0f, 0.0f, 0.0f);
	if (resetTimer->isActive())
		resetTimer->stop();
}

void VolumeMeter::setLevels(float nmag, float npeak, float npeakHold)
{
	mag      = nmag;
	peak     = npeak;
	peakHold = npeakHold;

	update();

	if (resetTimer->isActive())
		resetTimer->stop();
	resetTimer->start(1000);
}

void VolumeMeter::paintEvent(QPaintEvent *event)
{
	UNUSED_PARAMETER(event);

	QPainter painter(this);
	QLinearGradient gradient;

	int width  = size().width();
	int height = size().height();

	int scaledMag      = int((float)width * mag);
	int scaledPeak     = int((float)width * peak);
	int scaledPeakHold = int((float)width * peakHold);

	QColor my_bkcolor(112, 255, 212);
	QColor my_peakcolor("white");
	QColor my_peakholdcolor(250,250,250);

	gradient.setStart(qreal(scaledMag), 0);
	gradient.setFinalStop(qreal(scaledPeak), 0);
	gradient.setColorAt(0, my_peakcolor);
	gradient.setColorAt(1, my_peakholdcolor);


	// RMS
	painter.fillRect(0, 0, 
			scaledMag, height,
			my_peakcolor);
	//magColor);

	// RMS - Peak gradient
	painter.fillRect(scaledMag, 0,
			scaledPeak - scaledMag + 1, height,
			QBrush(gradient));

	// Background
	painter.fillRect(scaledPeak, 0,
			width - scaledPeak, height,
			my_bkcolor);

	// Peak hold
	if (peakHold == 1.0f)
		scaledPeakHold--;

	painter.setPen(peakHoldColor);
	painter.drawLine(scaledPeakHold, 0,
		scaledPeakHold, height);

}
