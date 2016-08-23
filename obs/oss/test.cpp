#include "test.h"
#include <iostream>
//#include "stdafx.h"
//#include <iostream>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

using namespace std;
#define NUM_THREADS 5

#pragma comment(lib,"pthreadVC2.lib") 

test::test()
{
}


test::~test()
{
	
}

struct thread_data
{
	int threadNum;
	int start;
	int end;
	int startPart;
	int endPart;
};

// 线程的运行函数
void* say_hello(void* args)
{
	struct thread_data *my_data;
	my_data = (struct thread_data *)args;
	int po = my_data->start;
	int partSize = 1024 * 1024 * 5;

	int last = po + partSize;

	//cout << "thread:" << my_data->threadNum << " startPart:" << my_data->startPart << " endPart:" << my_data->endPart << " start:" << my_data->start << endl;
	//int max = (my_data->end - my_data->start + partSize - 1) / partSize;

	for (int n = my_data->startPart; n <= my_data->endPart; n++)
	{
		printf("thread %d part %d\n", my_data->threadNum, n);
		po = last;
		last = n + 1 == my_data->endPart ? my_data->end : last + partSize;
	}
	printf("done start %d end %d\n", po, last - partSize);
	//for (int n = my_data->startPart; n <= my_data->endPart; n++)
	//{
	//	//下载逻辑

	//	//成功
	//	printf("thread %d upload %s part success \n", my_data->threadNum, n);
	//	po = last;
	//	last = n == my_data->endPart ? my_data->end : last + partSize;
	//	
	//	//失败
	//	//continue
	//	
	//}

	//do 
	//{
	//	//上传逻辑
	//	//判断成功
	//	po = last;
	//	last = my_data->end
	//} while ();

	//cout << "Hello Runoob！\n" << endl;

	return NULL;
}

int main3()
{
	int totalSize = 1024 * 1024 * 320 - 1;
	int partSize = 1024 * 1024 * 5;
	int threadSize;
	int rc;
	int i;
	pthread_t threads[NUM_THREADS];
	struct thread_data td[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	aos_table_t *headers = NULL;
	aos_table_t *resp_headers = NULL;
	aos_table_t *complete_headers = NULL;
	oss_request_options_t *options = NULL;
	aos_string_t upload_id;
	oss_upload_file_t *upload_file = NULL;
	aos_status_t *s = NULL;
	oss_list_upload_part_params_t *params = NULL;
	aos_list_t complete_part_list;
	oss_list_part_content_t *part_content = NULL;
	oss_complete_part_content_t *complete_part_content = NULL;
	int is_cname = 0;
	int part_num1 = 1;
	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	headers = aos_table_make(p, 1);
	aos_str_set(&bucket, "wenjie-backet");
	aos_str_set(&object, "test8.xls");
	//aos_str_set(&upload_id, "76E295F65D274DDFA032881E03C89536");

	//设置断点续传的upload_id
	s = oss_init_multipart_upload(options, &bucket, &object,
	     &upload_id, headers, &resp_headers);
	/* 判断是否初始化分片上传成功 */
	if (aos_status_is_ok(s)) {
		printf("Init multipart upload succeeded, upload_id:%.*s\n",
			upload_id.len, upload_id.data);
	} else {
		printf("Init multipart upload failed, upload_id:%.*s\n",
			upload_id.len, upload_id.data);
	}

	//threadSize = totalSize / partSize;
	//int margin = totalSize % partSize;
	//int num = (totalSize + partSize - 1) / partSize; //分块 总块数
	//printf("size:%d, int:%d, num:%d\n", threadSize, margin, num);
	//int thread_nums = num / NUM_THREADS; //除最后一个线程外每个线程的任务量
	//int lastThread = thread_nums + num % NUM_THREADS; //最后一个线程的任务量
	//printf("%d\n  %d ", thread_nums, lastThread);

	////int onesize = (totalSize + NUM_THREADS - 1) / NUM_THREADS;
	//////int onesize_margin = totalSize % NUM_THREADS;
	////int oneparts = (onesize + partSize - 1) / partSize;
	////printf("%d  %d", onesize, oneparts);

	//// 初始化并设置线程为可连接的（joinable）
	//pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	//printf("start\n");
	//for (i = 0; i < NUM_THREADS; i++) {
	//	td[i].threadNum = i;
	//	td[i].startPart = i * thread_nums + 1;
	//	td[i].endPart = i + 1 == NUM_THREADS ? i * thread_nums + lastThread : (i + 1) * thread_nums;

	//	td[i].start = (td[i].startPart - 1) * partSize;
	//	td[i].end = i + 1 == NUM_THREADS ? totalSize : td[i].start + partSize * thread_nums;
	//	//cout << "main() : creating thread, " << i << endl;
	//	rc = pthread_create(&threads[i], NULL, say_hello, (void *)&td[i]);
	//	if (rc) {
	//		cout << "Error:unable to create thread," << rc << endl;
	//		exit(-1);
	//	}
	//}
	////WaitForSingleObject()
	//// 删除属性，并等待其他线程
	//pthread_attr_destroy(&attr);
	//for (i = 0; i < NUM_THREADS; i++) {
	//	rc = pthread_join(threads[i], &status);
	//	if (rc) {
	//		cout << "Error:unable to join," << rc << endl;
	//		exit(-1);
	//	}
	//	cout << "Main: completed thread id :" << i;
	//	cout << "  exiting with status :" << status << endl;
	//}

	//cout << "Main: program exiting." << endl;

	//pthread_exit(NULL);

}
