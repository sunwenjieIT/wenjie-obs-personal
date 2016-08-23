#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"


#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#define NUM_THREADS 5

#pragma comment(lib,"pthreadVC2.lib") 

extern void append_object_sample();
extern void delete_object_sample();
extern void put_object_sample();
extern void get_object_sample();
extern void head_object_sample();
extern void multipart_object_sample();
extern void put_object_from_buffer();
extern void my_multipart_upload();
extern void easyMultipart_upload();
//extern void multi_thread_multipart_upload();

int main3(int argc, char *argv[])
{
    if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
        exit(1);
    }

    /*put_object_sample();
    append_object_sample();
    get_object_sample();
    head_object_sample();
    multipart_object_sample();
    delete_object_sample();
*/

	//multi_thread_multipart_upload();
	//my_multipart_upload();
	easyMultipart_upload();

    aos_http_io_deinitialize();

    system("pause");

    return 0;
}

struct thread_data
{
	oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;
	int threadNum;
	int start;
	int end;
	int startPart;
	int endPart;
};

void* say_hello(void* args)
{
	oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;

	struct thread_data *my_data;
	my_data = (struct thread_data *)args;
	bucket = my_data->bucket;
	object = my_data->object;
	upload_id = my_data->upload_id;
	resp_headers = my_data->resp_headers;
	options = my_data->options;
	upload_file = my_data->upload_file;

	int po = my_data->start;
	int partSize = 1024 * 1024 * 5;
	aos_status_t *s = NULL;
	int last = po + partSize;

	//cout << "thread:" << my_data->threadNum << " startPart:" << my_data->startPart << " endPart:" << my_data->endPart << " start:" << my_data->start << endl;
	//int max = (my_data->end - my_data->start + partSize - 1) / partSize;
	upload_file->file_pos = my_data->start;//起始偏移量
	upload_file->file_last = my_data->endPart - my_data->startPart == 0 ? my_data->end : my_data->start + partSize;//第一个分块偏移量
	printf("thread %d start upload", my_data->threadNum);
	for (int n = my_data->startPart; n <= my_data->endPart; n++)
	{
		
		s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
			n, upload_file, &resp_headers);
		if (aos_status_is_ok(s)) 
		{
			printf("Multipart upload from file success thread %d part %d\n", my_data->threadNum, n);
			upload_file->file_pos = upload_file->file_last;
			
			upload_file->file_last = n + 1 == my_data->endPart ? my_data->end : upload_file->file_last + partSize;
			//po = last;
		}
		else
		{
			printf("Multipart upload from file failed\n");
			n--;
			continue;
		}
	}
	printf("done start %d end %d now %d\n", my_data->start, my_data->end, upload_file->file_pos);

	pthread_exit(NULL);
	/************************************************************************/
	/* oss_upload_file_t *upload_file;
	oss_request_options_t *options;
	aos_table_t *resp_headers;
	aos_string_t upload_id;
	aos_string_t bucket;
	aos_string_t object;                                                                     */
	/************************************************************************/
	free(upload_file);
	free(options);
	free(resp_headers);
	free(&upload_id);
	free(&bucket);
	free(&object);
	free(my_data);
	return NULL;
}

void main()
{
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
		exit(1);
	}
	aos_pool_t *plist[NUM_THREADS];
	oss_request_options_t *optionslist[NUM_THREADS];

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
	aos_str_set(&bucket, "wenjie-backet");
	aos_str_set(&object, "thread10.mp4");
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/ActivePerl_5.16.2.3010812913.msi";
	char UPLOAD_FILE[] = "F:/download_all/Chrom_download/one piece715-dmxz.zerodm.com.mp4";
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/ftpserver-1.0.6.zip";

	int64_t totalSize = get_file_size(UPLOAD_FILE);
	if (totalSize == -1)
	{
		printf("文件不存在!");
		return;
	}

	aos_pool_create(&p, NULL);
	/* 创建并初始化options */
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	//init_options(options);
	/* 初始化参数 */
	headers = aos_table_make(p, 1);
	

	
	
	//aos_str_set(&upload_id, "76E295F65D274DDFA032881E03C89536");

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

	//分片上传开始
	//upload_file = oss_create_upload_file(p);
	
	//aos_str_set(&upload_file->filename, UPLOAD_FILE);
	
	//upload_file->file_pos = 0;
	//upload_file->file_last = 200 * 1024;//200k
	
	int threads_num;
	

	//int totalSize = 1024 * 1024 * 320 - 1;
	int partSize = 1024 * 1024 * 5;
	long int threadSize;
	int rc;
	int i;
	
	pthread_attr_t attr;
	void *status;

	threadSize = totalSize / partSize;
	long int margin = totalSize % partSize;
	int num = (totalSize + partSize - 1) / partSize; //分块 总块数
	printf("%d\n", num);
	threads_num = num > 5 ? 5 : num;
	pthread_t threads[NUM_THREADS];
	struct thread_data td[NUM_THREADS];
	printf("%d\n", num);
	printf("%d\n", margin);
	printf("size:%I64d, int:%d, num:%d\n", totalSize, margin, num);
	printf("%d\n", num);
	int thread_nums = num / threads_num; //除最后一个线程外每个线程的任务量
	int lastThread = thread_nums + num % threads_num; //最后一个线程的任务量
	printf("thread_nums %d\n lastThread %d ", thread_nums, lastThread);
	printf("test\n");

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	printf("start\n");
	for (i = 0; i < threads_num; i++) {
		td[i].threadNum = i;
		td[i].startPart = i * thread_nums + 1;
		td[i].endPart = i + 1 == threads_num ? i * thread_nums + lastThread : (i + 1) * thread_nums;
		
		/* 创建并初始化options */
		aos_pool_create(&plist[i], NULL);
		optionslist[i] = oss_request_options_create(plist[i]);
		init_sample_request_options(optionslist[i], is_cname);
		td[i].options = optionslist[i];
		
		//td[i].options = options;
		
		td[i].upload_id = upload_id;
		td[i].bucket = bucket;
		td[i].object = object;
		
		//upload_file = oss_create_upload_file(p);
		upload_file = oss_create_upload_file(plist[i]);
		
		aos_str_set(&upload_file->filename, UPLOAD_FILE);

		td[i].upload_file = upload_file;
		td[i].resp_headers = resp_headers;
		td[i].start = (td[i].startPart - 1) * partSize;
		td[i].end = i + 1 == threads_num ? totalSize : td[i].start + partSize * thread_nums;
		//cout << "main() : creating thread, " << i << endl;
		rc = pthread_create(&threads[i], NULL, say_hello, (void *)&td[i]);
		if (rc) {
			printf("Error:unable to create thread,%d\n",rc);
			exit(-1);
		}
	}
	//WaitForSingleObject()
	// 删除属性，并等待其他线程
	pthread_attr_destroy(&attr);
	for (i = 0; i < threads_num; i++) {
		rc = pthread_join(threads[i], &status);
		if (rc) {
			printf("Error:unable to join, : %d\n", rc);
			exit(-1);
		}
		printf("Main: completed thread id : %d\n", i);
		printf("  exiting with status : %d\n", status);
	}
	

	/* 完成分片上传，代码参考后面章节，这里省略*/
	params = oss_create_list_upload_part_params(p);
	params->max_ret = 1000;
	aos_list_init(&complete_part_list);
	s = oss_list_upload_part(options, &bucket, &object, &upload_id, params
		, &resp_headers);
	if (aos_status_is_ok(s)) {
		printf("List multipart secceeded %d \n", s->req_id);
	}
	else {
		printf("List multipart failed");
	}

	aos_list_for_each_entry(oss_list_part_content_t, part_content, &params->part_list, node) {
		complete_part_content = oss_create_complete_part_content(p);
		aos_str_set(&complete_part_content->part_number,
			part_content->part_number.data);
		aos_str_set(&complete_part_content->etag, part_content->etag.data);
		aos_list_add_tail(&complete_part_content->node, &complete_part_list);

	}

	s = oss_complete_multipart_upload(options, &bucket, &object,
		&upload_id, &complete_part_list, complete_headers, &resp_headers);

	if (aos_status_is_ok(s)) {
		printf("Complete multipart upload from file succeeded, upload_id:%.*s\n",
			upload_id.len, upload_id.data);
		/*printf("Complete multipart upload from file succeeded, upload_id:%d",
			upload_id.data);*/
	}
	else {
		printf("Complete multipart upload from file failed\n");
	}
	aos_http_io_deinitialize();
	printf("finish upload\n");
	pthread_exit(NULL);
}