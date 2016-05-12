/*
 * leancloud_rsp.c
 *
 *  Created on: May 1, 2015
 *      Author: jk
 */
#include <stdlib.h>
#include <string.h>

#include "leancloud_rsp.h"
#include "leancloud_req.h"
#include "cJSON.h"
#include "log.h"
#include "env.h"
#include "db.h"
#include "objectID_leancloud.h"

size_t leancloud_onSaveGPS(void *contents, size_t size, size_t nmemb, void *userdata)
{
	userdata = userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse GPS response: %s", rsp);
	}
	else
	{
		LOG_DEBUG("get save GPS response: %s", rsp);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onSaveDID(void *contents, size_t size, size_t nmemb, void *userdata)
{
	const char* imei = (const char*) userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse DID response: %s", rsp);
	}
	else
	{
		LOG_INFO("get save DID(%s) response: %s", imei, rsp);
		db_updateOBJIsPosted(imei);

		cJSON* objectID = cJSON_GetObjectItem(json, "objectId");
		if(!objectID)
		{
			LOG_ERROR("can't get objectId in leancloud_onSaveDID");
			return 0;
		}

		int rc = objectID_add_HashAndDb(imei, objectID->valuestring);
		if(rc)
		{
			LOG_ERROR("can't add objectId into hash and db");
			return 0;
		}
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onSaveItinerary(void *contents, size_t size, size_t nmemb, void *userdata)
{
	userdata = userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse Itinerary response: %s", rsp);
	}
	else
	{
		LOG_DEBUG("get save Itinerary response: %s", rsp);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onSaveSimInfo(void *contents, size_t size, size_t nmemb, void *userdata)
{
	userdata = userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse SimInfo response: %s", rsp);
	}
	else
	{
		LOG_DEBUG("get save SimInfo response: %s", rsp);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onGetObjectIDWithImei(void *contents, size_t size, size_t nmemb, void *userdata)
{
	userdata = userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error objectID with imei response: %s", rsp);
	}
	else
	{
		LOG_DEBUG("get objectID with imei response: %s", rsp);

		cJSON* results = cJSON_GetObjectItem(json, "results");
		if(!results)
		{
			LOG_ERROR("can't get results in leancloud_onGetObjectIDWithImei");
			return 0;
		}

		cJSON* result = cJSON_GetArrayItem(results, 0);
		if(!result)
		{
			LOG_ERROR("can't get result in leancloud_onGetObjectIDWithImei");
			return 0;
		}

		cJSON* IMEI = cJSON_GetObjectItem(result, "IMEI");
		if(!IMEI)
		{
			LOG_ERROR("can't get IMEI in leancloud_onGetObjectIDWithImei");
			return 0;
		}

		cJSON* objectID = cJSON_GetObjectItem(result, "objectId");
		if(!objectID)
		{
			LOG_ERROR("can't get objectId in leancloud_onGetObjectIDWithImei");
			return 0;
		}

		int rc = objectID_add_HashAndDb(IMEI->valuestring, objectID->valuestring);
		if(rc)
		{
			LOG_ERROR("can't add objectId into hash and db");
			return 0;
		}
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onRev(void *contents, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size * nmemb;
	MemroyBuf *mem = (MemroyBuf *)userdata;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL)
	{
		LOG_ERROR("not enough memory (realloc returned NULL)");
		return 0;
	}

	LOG_DEBUG("receive %zu bytes from leancloud", realsize);

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}
