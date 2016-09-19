#include "YDBUpdate.h"
#include "ui_YDBUpdate.h"

YDBUpdate::YDBUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::YDBUpdate)
{
    ui->setupUi(this);
	this->setWindowFlags(Qt::WindowCloseButtonHint);
}

YDBUpdate::~YDBUpdate()
{
    delete ui;
}
