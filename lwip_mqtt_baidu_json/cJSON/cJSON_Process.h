#ifndef _CJSON_PROCESS_H_
#define _CJSON_PROCESS_H_
#include "cJSON.h"
#include "stdint.h"


#define   NAME          "name"     
#define   NUMBER        "number"  
#define   BOOL          "bool" 

#define   DEFAULT_NAME          "fire"     
#define   DEFAULT_NUMBER        123.3  
#define   DEFAULT_BOOL          0 

#define   UPDATE_SUCCESS       1 
#define   UPDATE_FAIL          0

cJSON* cJSON_Data_Init(void);
uint8_t cJSON_Update(const cJSON * const object,const char * const string,void * d);
void Proscess(void* data);
#endif

