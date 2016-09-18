#include "YDBRefreshSpace.h"
#include <windows.h> 

void MyThreadTest::run() {
	QString test = label->text();
	//qDebug() << test;
	QString abc;

	while (true)
	{

		LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();

		ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;

		if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
		{
			return;
		}
		quint64 free_mb = (quint64)(liTotalFreeBytes.QuadPart / 1024 / 1024);//MB
																				//int temp = free_mb * 100 / 1024;
		double free_gb = double(free_mb * 100 / 1024) / 100;

		label->setText(QString::number(free_gb) + "G");
	}

		//return (quint64)liTotalFreeBytes.QuadPart / 1024 / 1024 / 1024;
}