#include "MyInt64Mq.h"



MyInt64Mq::MyInt64Mq()
{
	blog(LOG_INFO, "mq init");
	list = new QList<int64_t>;
}


MyInt64Mq::~MyInt64Mq()
{
	blog(LOG_INFO, "mq delete");
	delete list;
}

void MyInt64Mq::putMessage(int64_t value)
{
	blog(LOG_INFO, "mq put message");
	list->append(value);
}
int64_t MyInt64Mq::getMessage()
{
	
	//blog(LOG_INFO, "mq get message");
	int64_t first = 0;
	if (!list->isEmpty())
	{
		first = list->first();
		list->removeFirst();
	}
	return first;
}

int MyInt64Mq::size()
{
	blog(LOG_INFO, "mq size");
	return list->size();
}

bool MyInt64Mq::isFinish()
{
	blog(LOG_INFO, "mq isfinish");
	bool isFinish = false;
	if (!list->isEmpty()) {
		isFinish = list->first() == -1;
	}
	return isFinish;
}
void MyInt64Mq::clear() {
	list->clear();
}
MyInt64Mq2::MyInt64Mq2()
{
	list = new QList<int64_t>;
}


MyInt64Mq2::~MyInt64Mq2()
{
	delete list;
}

void MyInt64Mq2::putMessage(int64_t value)
{
	list->append(value);
}
int64_t MyInt64Mq2::getMessage()
{
	int64_t first = 0;
	if (!list->isEmpty())
	{
		first = list->first();
		list->removeFirst();
	}
	return first;
}
int MyInt64Mq2::size()
{
	return list->size();
}