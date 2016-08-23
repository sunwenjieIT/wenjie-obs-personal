#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"




void multipart_upload_file_from_buffer()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *complete_headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_string_t upload_id;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    oss_list_upload_part_params_t *params = NULL;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content = NULL;
    oss_complete_part_content_t *complete_part_content = NULL;
    int part_num1 = 1;
    int part_num2 = 2;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    complete_headers = aos_table_make(p, 1); 
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "multipart-key.1");
    
    //init mulitipart
    s = oss_init_multipart_upload(options, &bucket, &object, 
                                  &upload_id, headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n");
        aos_pool_destroy(p);
        return;
    }    

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
                                    part_num1, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_init(&buffer);
    make_random_body(p, 10, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num2, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
        aos_pool_destroy(p);
        return;
    }    

    //list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1000;
    aos_list_init(&complete_part_list);
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("List multipart succeeded\n");
    } else {
        printf("List multipart failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_for_each_entry(oss_list_part_content_t, part_content, &params->part_list, node) {
        complete_part_content = oss_create_complete_part_content(p);
        aos_str_set(&complete_part_content->part_number, 
                    part_content->part_number.data);
        aos_str_set(&complete_part_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_part_content->node, &complete_part_list);
    }

    //complete multipart
    apr_table_add(complete_headers, OSS_CONTENT_TYPE, "video/MP2T");
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, complete_headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Complete multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload failed\n");
    }

    aos_pool_destroy(p);
}

void multipart_upload_file_from_file()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *complete_headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_string_t upload_id;
    oss_upload_file_t *upload_file = NULL;
    aos_status_t *s = NULL;
    oss_list_upload_part_params_t *params = NULL;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content = NULL;
    oss_complete_part_content_t *complete_part_content = NULL;
    int part_num = 1;
    int64_t pos = 0;
    int64_t file_length = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    complete_headers = aos_table_make(p, 1);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "multipart-key.2");
    
    //init mulitipart
    s = oss_init_multipart_upload(options, &bucket, &object, 
                                  &upload_id, headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n");
        aos_pool_destroy(p);
        return;
    }

    //upload part from file
    file_length = get_file_size(MULTIPART_UPLOAD_FILE_PATH);
    while(pos < file_length) {
        upload_file = oss_create_upload_file(p);
        aos_str_set(&upload_file->filename, MULTIPART_UPLOAD_FILE_PATH);
        upload_file->file_pos = pos;
        pos += 100 * 1024;
        upload_file->file_last = pos < file_length ? pos : file_length; //200k
        s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
                part_num++, upload_file, &resp_headers);

        if (aos_status_is_ok(s)) {
            printf("Multipart upload part from file succeeded\n");
        } else {
            printf("Multipart upload part from file failed\n");
        }
    }

    //list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1000;
    aos_list_init(&complete_part_list);
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("List multipart succeeded\n");
    } else {
        printf("List multipart failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_for_each_entry(oss_list_part_content_t, part_content, &params->part_list, node) {
        complete_part_content = oss_create_complete_part_content(p);
        aos_str_set(&complete_part_content->part_number, 
                    part_content->part_number.data);
        aos_str_set(&complete_part_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_part_content->node, &complete_part_list);
    }

    //complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, complete_headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Complete multipart upload from file succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload from file failed\n");
    }

    aos_pool_destroy(p);
}

void abort_multipart_upload()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_string_t upload_id;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    
    s = oss_init_multipart_upload(options, &bucket, &object, 
                                  &upload_id, headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n"); 
        aos_pool_destroy(p);
        return;
    }
    
    s = oss_abort_multipart_upload(options, &bucket, &object, &upload_id, 
                                   &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("Abort multipart upload succeeded, upload_id::%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Abort multipart upload failed\n"); 
    }    

    aos_pool_destroy(p);
}

void multipart_object_sample()
{
    multipart_upload_file_from_buffer();
    multipart_upload_file_from_file();
    abort_multipart_upload();
}

void my_multipart_upload()
{
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
	/* 创建并初始化options */
	//options = oss_request_options_create(p);
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	//init_options(options);
	/* 初始化参数 */
	headers = aos_table_make(p, 1);
	aos_str_set(&bucket, "wenjie-backet");
	aos_str_set(&object, "test10.xls");
	//aos_str_set(&upload_id, "76E295F65D274DDFA032881E03C89536");
	/* 初始化分片上传，获取一个上传ID */
	printf("Copy upload id succeeded, upload_id:%.*s\n",
		upload_id.len, upload_id.data);
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
	/*printf("%d,:%s",upload_id.len, upload_id.data);
	aos_string_t copy_upload_id;
	aos_str_set(&copy_upload_id, "360A20F379FC46AB97A5E9E342EB21E1");
	printf("Copy upload id succeeded, upload_id:%.*s\n",
		copy_upload_id.len, copy_upload_id.data);*/

	/* 上传分片*/
	upload_file = oss_create_upload_file(p);
	char UPLOAD_FILE[] = "F:/download_all/Chrom_download/suiteitem_SHTRQC.xls";
	//char UPLOAD_FILE[] = "F:/download_all/Chrom_download/obs-studio-0.15.2.zip";
	aos_str_set(&upload_file->filename, UPLOAD_FILE);
	int64_t total_size = get_file_size(UPLOAD_FILE);
	upload_file->file_pos = 0;
	upload_file->file_last = 200 * 1024;//200k
	do 
	{
		if (part_num1 == 1) {
			upload_file->file_pos = upload_file->file_last;
			upload_file->file_last = total_size - upload_file->file_last > 200 * 1024 ? upload_file->file_last + 200 * 1024 : total_size;
			part_num1++;
			continue;
		}
		
		s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
			part_num1, upload_file, &resp_headers);
		if (aos_status_is_ok(s)) {
			printf("Multipart upload from file success %d \n", s->req_id);
			upload_file->file_pos = upload_file->file_last;
			upload_file->file_last = total_size - upload_file->file_last > 200 * 1024 ? upload_file->file_last + 200 * 1024 : total_size;
			part_num1++;
			break;
			// failed test
			//upload_file->file_last = get_file_size("");
			//upload_file->file_last += 200 * 1024;
		}
		else {
			printf("Multipart upload from file failed\n");
			//break;
			continue;
		}
	} while (upload_file->file_pos < total_size);
	

	/* 完成分片上传*/
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
		printf("Complete multipart upload from file succeeded, upload_id:%d",
			upload_id.data);
	}
	else {
		printf("Complete multipart upload from file failed\n");
	}
	/* 释放资源*/
	aos_pool_destroy(p);
	

}

void easyMultipart_upload() 
{
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	oss_request_options_t *options = NULL;
	aos_status_t *s = NULL;
	int part_size = 5 * 1024 *1024;
	aos_string_t upload_id;
	aos_string_t filepath;
	int is_cname = 0;
	aos_pool_create(&p, NULL);
	
	/* 创建并初始化options */
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname);
	//init_options(options);
	aos_str_set(&bucket, "wenjie-backet");
	aos_str_set(&object, "easy4.zip");
	aos_str_null(&upload_id);
	//aos_str_set(&upload_id, "9B79B397977341C7AE0740E115A52F1F");//断点续传
	aos_str_set(&filepath, "F:/download_all/Chrom_download/QT5testfile.zip");
	
	/* 分片上传 */
	s = oss_upload_file(options, &bucket, &object, &upload_id, &filepath,
	                    part_size, NULL);
	
	/* 判断是否上传成功 */
	if (aos_status_is_ok(s)) {
		printf("upload file succeeded\n");

	}
	else {
		printf("upload file failed\n");

	}
	printf("Complete multipart upload from file succeeded, upload_id:%.*s\n",
		upload_id.len, upload_id.data);
	
	/* 释放资源*/
	aos_pool_destroy(p);

}

