#ifndef YDBUPDATE_H
#define YDBUPDATE_H

#include <QDialog>

namespace Ui {
class YDBUpdate;
}

class YDBUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit YDBUpdate(QWidget *parent = 0);
    ~YDBUpdate();

private:
    Ui::YDBUpdate *ui;
};

#endif // YDBUPDATE_H
