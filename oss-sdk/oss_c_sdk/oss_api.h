#ifndef LIBOSS_API_H
#define LIBOSS_API_H

#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_define.h"
#include "oss_util.h"

OSS_CPP_START

/*
 * @brief  create oss bucket
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_create_bucket(const oss_request_options_t *options,
                                const aos_string_t *bucket,
                                oss_acl_e oss_acl,
                                aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                aos_table_t **resp_headers);

/*
 * @brief  put oss bucket acl
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 oss_acl_e oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief  get oss bucket acl
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_acl(const oss_request_options_t *options,
                                 const aos_string_t *bucket,
                                 aos_string_t *oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief  put oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   lifecycle_rule_list the oss bucket lifecycle list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/*
 * @brief  get oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  lifecycle_rule_list the oss bucket lifecycle list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket_lifecycle(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          aos_table_t **resp_headers);

/*
 * @brief  list oss objects
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   params        input params for list object request,
                              including prefix, marker, delimiter, max_ret
 * @param[out]  params        output params for list object response,
                              including truncated, next_marker, obje list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              oss_list_object_params_t *params, 
                              aos_table_t **resp_headers);

/*
 * @brief  put oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_buffer(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_list_t *buffer, 
                                         aos_table_t *headers,
                                         aos_table_t **resp_headers);

/*
 * @brief  put oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   filename            the filename to put
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_file(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object, 
                                       const aos_string_t *filename,
                                       aos_table_t *headers, 
                                       aos_table_t **resp_headers);

/*
 * @brief  get oss object to buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  buffer              the buffer containing object content
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_buffer(const oss_request_options_t *options, 
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object,
                                       aos_table_t *headers, 
                                       aos_table_t *params,
                                       aos_list_t *buffer, 
                                       aos_table_t **resp_headers);

/*
 * @brief  get oss object to file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  filename            the filename storing object content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_file(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     const aos_string_t *object,
                                     aos_table_t *headers, 
                                     aos_table_t *params,
                                     aos_string_t *filename, 
                                     aos_table_t **resp_headers);

/*
 * @brief  head oss object
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   object           the oss object name
 * @param[in]   headers          the headers for request
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_head_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *object,
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/*
 * @brief  delete oss object
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_object(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                const aos_string_t *object, 
                                aos_table_t **resp_headers);

/*
 * @brief  delete oss objects
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object_list         the oss object list name
 * @param[in]   is_quiet            is quiet or verbose
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  deleted_object_list deleted object list
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_objects(const oss_request_options_t *options,
                                 const aos_string_t *bucket, 
                                 aos_list_t *object_list, 
                                 int is_quiet,
                                 aos_table_t **resp_headers, 
                                 aos_list_t *deleted_object_list);

/*
 * @brief  delete oss objects by prefix
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   prefix              prefix of delete objects
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_objects_by_prefix(oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *prefix);

/*
 * @brief  copy oss objects
 * @param[in]   options             the oss request options
 * @param[in]   source_bucket       the oss source bucket name
 * @param[in]   object_list         the oss source object list name
 * @param[in]   dest_bucket         the oss dest bucket name
 * @param[in]   dest_list           the oss dest object list name
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_copy_object(const oss_request_options_t *options, 
                              const aos_string_t *source_bucket, 
                              const aos_string_t *source_object, 
                              const aos_string_t *dest_bucket, 
                              const aos_string_t *dest_object, 
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/*
 * @brief  append oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_append_object_from_buffer(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            int64_t position,
                                            aos_list_t *buffer, 
                                            aos_table_t *headers, 
                                            aos_table_t **resp_headers);

/*
 * @brief  append oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   append_file         the file containing appending content 
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_append_object_from_file(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          int64_t position,
                                          const aos_string_t *append_file, 
                                          aos_table_t *headers, 
                                          aos_table_t **resp_headers);

/*
 * @brief  gen signed url for oss object api
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   expires             the end expire time for signed url
 * @param[in]   req                 the aos http request
 * @return  signed url, non-NULL success, NULL failure
 */
char *oss_gen_signed_url(const oss_request_options_t *options, 
                         const aos_string_t *bucket,
                         const aos_string_t *object, 
                         int64_t expires, 
                         aos_http_request_t *req);

/*
 * @brief  oss put object from buffer using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_buffer_by_url(const oss_request_options_t *options,
                                                const aos_string_t *signed_url, 
                                                aos_list_t *buffer, 
                                                aos_table_t *headers,
                                                aos_table_t **resp_headers);

/*
 * @brief  oss put object from file using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   filename            the filename containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_file_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_string_t *filename, 
                                              aos_table_t *headers,
                                              aos_table_t **resp_headers);

/*
 * @brief  oss get object to buffer using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_buffer_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_table_t *headers,
                                              aos_table_t *params,
                                              aos_list_t *buffer,
                                              aos_table_t **resp_headers);

/*
 * @brief  oss get object to file using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   filename            the filename containing object content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_file_by_url(const oss_request_options_t *options,
                                            const aos_string_t *signed_url,
                                            aos_table_t *headers, 
                                            aos_table_t *params,
                                            aos_string_t *filename,
                                            aos_table_t **resp_headers);

/*
 * @brief  oss head object using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_head_object_by_url(const oss_request_options_t *options,
                                     const aos_string_t *signed_url, 
                                     aos_table_t *headers, 
                                     aos_table_t **resp_headers);

/*
 * @brief  oss init multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_init_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object, 
                                        aos_string_t *upload_id,
                                        aos_table_t *headers,
                                        aos_table_t **resp_headers);

/*
 * @brief  oss upload part from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   buffer              the buffer containing upload part content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_from_buffer(const oss_request_options_t *options, 
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          const aos_string_t *upload_id, 
                                          int part_num, 
                                          aos_list_t *buffer, 
                                          aos_table_t **resp_headers);

/*
 * @brief  oss upload part from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   upload_file         the file containing upload part content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_from_file(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object,
                                        const aos_string_t *upload_id, 
                                        int part_num, 
                                        oss_upload_file_t *upload_file,
                                        aos_table_t **resp_headers);

/*
 * @brief  oss abort multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_abort_multipart_upload(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_string_t *upload_id, 
                                         aos_table_t **resp_headers);


/*
 * @brief  oss complete multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_list           the uploaded part list to complete
 * @param[in]   headers             the headers for request          
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_complete_multipart_upload(const oss_request_options_t *options, 
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            const aos_string_t *upload_id, 
                                            aos_list_t *part_list, 
                                            aos_table_t *headers,
                                            aos_table_t **resp_headers);

/*
 * @brief  oss list upload part with specific upload_id for object
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   params              the input list upload part parameters,
                                    incluing part_number_marker, max_ret
 * @param[out]  params              the output params,
                                    including next_part_number_marker, part_list, truncated
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_upload_part(const oss_request_options_t *options, 
                                   const aos_string_t *bucket, 
                                   const aos_string_t *object, 
                                   const aos_string_t *upload_id, 
                                   oss_list_upload_part_params_t *params, 
                                   aos_table_t **resp_headers);

/*
 * @brief  oss list multipart upload for bucket
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   params              the input list multipart upload parameters
 * @param[out]  params              the output params including next_key_marker, next_upload_id_markert, upload_list etc
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        oss_list_multipart_upload_params_t *params, 
                                        aos_table_t **resp_headers);

/*
 * @brief  oss copy large object using upload part copy
 * @param[in]   options             the oss request options
 * @param[in]   paramsthe           upload part copy parameters
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_copy(const oss_request_options_t *options,
                                   oss_upload_part_copy_params_t *params, 
                                   aos_table_t *headers, 
                                   aos_table_t **resp_headers);

/*
 * @brief  oss upload file using multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   filename            the filename containing object content
 * @param[in]   part_size           the part size for multipart upload
 * @param[in]   headers             the headers for request
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_file(oss_request_options_t *options,
                              const aos_string_t *bucket, 
                              const aos_string_t *object, 
                              aos_string_t *upload_id,
                              aos_string_t *filename, 
                              int64_t part_size,
                              aos_table_t *headers);

OSS_CPP_END

#endif
