#include <QThread>
#include <QLabel>
#include <QString>
class MyThreadTest : public QThread {
public:
	MyThreadTest(QLabel* label, QString driver) {
		this->label = label;
		this->driver = driver;
	}

	//signals:
	//    void myfinish(const int);

signals:
	void mysignal();
public:
	void run();
private:
	QLabel* label;
	QString driver;
};