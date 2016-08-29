#pragma once
#include <QList>
#include <QVariant>
#include "util/base.h"

class MyInt64Mq
{
private:
	QList<int64_t> *list;
	int64_t total_progress;

public:
	MyInt64Mq();
	~MyInt64Mq();
	void putMessage(int64_t value);
	int64_t getMessage();
	int size();
	bool isFinish();
	void clear();
};

//Q_DECLARE_METATYPE(MyInt64Mq*);

class MyInt64Mq2
{
private:
	QList<int64_t> *list;


public:
	MyInt64Mq2();
	~MyInt64Mq2();
	void putMessage(int64_t value);
	int64_t getMessage();
	int size();
};