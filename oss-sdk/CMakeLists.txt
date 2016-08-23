#cmake_minimum_required(VERSION 2.8.12)
project (oss-sdk)


#AUX_SOURCE_DIRECTORY(. SRC_LIST)
#ADD_EXECUTABLE(oss-sdk ${SRC_LIST})


#include_directories(${SRC_LIST})

set(SDK_SIMPLE_SOURCES 
	oss_sample_util.c 
	oss_config.c  
    oss_put_object_sample.c 
	oss_append_object_sample.c 
	oss_get_object_sample.c
    oss_head_object_sample.c 
	oss_delete_object_sample.c 
	oss_multipart_upload_sample.c)

set(SDK_SIMPLE_HEADERS
	oss_config.h
	oss_sample_util.h)
	
set(SDK_DIR  D:/third_party/include/sdk)
set(LINK_DIR F:/data/JavaSoft/Source/Repos/aliyun_oss_c_sdk_v3.0.0/Release)

set(APR_INC_DIR  D:/third_party/include/apr)
set(APRUTIL_INC_DIR  D:/third_party/include/aprutil)
set(CURL_INC_DIR  D:/third_party/include/curl)
set(MXML_INC_DIR  D:/third_party/include/mxml)

#set(LINK_DIR F:/data/JavaSoft/Source/Repos/aliyun_oss_c_sdk_v3.0.0/Release)

#set(SOURCE_FILES main.cpp)
#[[include_directories (${APR_INCLUDE_DIR})
#include_directories (${APR_UTIL_INCLUDE_DIR})
#include_directories (${MINIXML_INCLUDE_DIR})
#include_directories (${CURL_INCLUDE_DIR})
#include_directories ("${OSS_C_SDK_INCLUDE_DIR}")]]
#include_directories ("${CMAKE_SOURCE_DIR}/oss_c_sdk")

include_directories(${SDK_DIR})
include_directories(${APR_INC_DIR})
include_directories(${APRUTIL_INC_DIR})
include_directories(${CURL_INC_DIR})
include_directories(${MXML_INC_DIR})

link_directories(${LINK_DIR})

add_library(oss-sdk SHARED ${SDK_SIMPLE_SOURCES} ${SDK_SIMPLE_HEADERS})
#ADD_EXECUTABLE(test ${SRC_LIST})
target_link_libraries(oss-sdk
		curl
		mxml1
		libapr-1
		libaprutil-1
		oss_c_sdk
		ws2_32)

#ADD_LIBRARY(oss-sdk-simple SHARED ${SRC_LIST})

#set_target_properties(oss-sdk PROPERTIES
#	OUTPUT_NAME obs
#	VERSION "0"
#	SOVERSION "0")
		
#find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
#find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
#find_library(MINIXML_LIBRARY mxml)
#find_library(CURL_LIBRARY curl)
#find_library(OSS_C_SDK_LIBRARY oss_c_sdk_lib)
#find_library(PTHREAD_LIBRARY pthread)

#find_library(RT_LIBRARY rt)
#find_library(M_LIBRARY m)

#function(_TARGET_SAMPLE_LIBRARIES SAMPLE_BIN_NAME SOURCE_FILES)
#	 add_executable(${SAMPLE_BIN_NAME} ${SOURCE_FILES})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${OSS_C_SDK})
#	 target_link_libraries(${SAMPLE_BIN_NAME} oss_c_sdk)
	 #[[target_link_libraries(${SAMPLE_BIN_NAME} ${APR_UTIL_LIBRARY})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${APR_LIBRARY})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${MINIXML_LIBRARY})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${CURL_LIBRARY})]]
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${PTHREAD_LIBRARY})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${RT_LIBRARY})
#	 target_link_libraries(${SAMPLE_BIN_NAME} ${M_LIBRARY})
#endfunction()

#_TARGET_SAMPLE_LIBRARIES(oss_c_sdk_sample "${OSS_SAMPLE_SOURCE_FILES}")
