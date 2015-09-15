/*
 * msg_proc_simcom.c
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */


#include <string.h>
#include <netinet/in.h>
#include <malloc.h>

#include "msg_proc_app.h"
#include "msg_proc_simcom.h"
#include "protocol.h"
#include "log.h"
#include "cb_ctx_simcom.h"
#include "object.h"
#include "msg_simcom.h"

typedef int (*MSG_PROC)(const void *msg, SIMCOM_CTX *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int handle_one_msg(const void *msg, SIMCOM_CTX *ctx);
static int simcom_login(const void *msg, SIMCOM_CTX *ctx);
static int simcom_gps(const void *msg, SIMCOM_CTX *ctx);
static int simcom_cell(const void *msg, SIMCOM_CTX *ctx);
static int simcom_ping(const void *msg, SIMCOM_CTX *ctx);
static int simcom_alarm(const void *msg, SIMCOM_CTX *ctx);


static MSG_PROC_MAP msgProcs[] =
{
        {CMD_LOGIN, simcom_login},
        {CMD_GPS,   simcom_gps},
        {CMD_CELL,  simcom_cell},
        {CMD_PING,  simcom_ping},
        {CMD_ALARM, simcom_alarm},
};

int handle_simcom_msg(const char *m, size_t msgLen, void *arg)
{
    const MSG_HEADER *msg = (MSG_HEADER *)m;

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }
    size_t leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN)
    {
        char *status = (char *)(msg->signature);
        if((status[0] == 0x55) && (status[1] == 0xAA)) //TODO:
        {
            LOG_ERROR("receive message header signature error:%x%x", msg->signature);
            return -1;
        }
        handle_one_msg(msg, (SIMCOM_CTX *)arg);
        leftLen = leftLen - MSG_HEADER_LEN - ntohs(msg->length);
        msg = (MSG_HEADER *)(m + msgLen - leftLen);
    }
}


int handle_one_msg(const void *m, SIMCOM_CTX *ctx)
{
    const MSG_HEADER *msg = (MSG_HEADER *)m;
    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg, ctx);
            }
            else
            {
                LOG_ERROR("Message %d not processed", msg->cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown message cmd(%d)", msg->cmd);
    return -1;
}


static int simcom_msg_send(void *msg, size_t len, SIMCOM_CTX *ctx)
{
    if (!ctx)
    {
        return -1;
    }

    SIMCOM_MSG_SEND pfn = ctx->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        return -1;
    }

    pfn(ctx->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_msg_cmd(msg), len);
    LOG_HEX(msg, len);

    free(msg);

    return 0;
}

static int simcom_login(const void *msg, SIMCOM_CTX *ctx)
{
    const MSG_LOGIN_REQ* req = msg;

    OBJECT * obj = ctx->obj;
    
    const char *imei = get_IMEI(req->IMEI); 

    if (!obj)
    {
        LOG_DEBUG("mc IMEI(%s) login", imei);

        obj = obj_get(imei);

        if (!obj)
        {
            LOG_INFO("the first time of simcom IMEI(%s)'s login", imei);

            obj = obj_new();

            memcpy(obj->IMEI, imei, IMEI_LENGTH + 1);
            memcpy(obj->DID, imei, strlen(req->IMEI) + 1);

            obj_add(obj);
        }

        ctx->obj = obj;
    }
    else
    {
        LOG_DEBUG("simcom IMEI(%s) already login", imei);
    }

    MSG_LOGIN_RSP *rsp = alloc_simcom_rspMsg(msg);
    if (rsp)
    {
        simcom_msg_send(rsp, sizeof(MSG_LOGIN_RSP), ctx);
    }
    else
    {
        //TODO: LOG_ERROR
    }
    //TODO : session mqtt mysql
    return 0;
}

static int simcom_gps(const void *msg, SIMCOM_CTX *ctx)
{
    const MSG_GPS* req = msg;

    if (!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }

    if (req->header.length < sizeof(MSG_GPS) - MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    LOG_INFO("GPS: lat(%f), lng(%f)", req->gps.latitude, req->gps.longitude);

    OBJECT * obj = ctx->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }
    //no response message needed

    if (obj->lat == ntohl(req->gps.latitude)
        && obj->lon == ntohl(req->gps.longitude))
    {
        LOG_INFO("No need to save data to leancloud");
        app_sendGpsMsg2App(ctx);
        return 0;
    }

    //update local object
    time_t rawtime;
    time(&rawtime);
    obj->timestamp = rawtime;
    obj->isGPSlocated = 0x01;
    obj->lat = ntohl(req->gps.latitude);
    obj->lon = ntohl(req->gps.longitude);

    app_sendGpsMsg2App(ctx);

    //stop upload data to yeelink
    //yeelink_saveGPS(obj, ctx);
    //TODO:save to DB

    return 0;
}

int simcom_cell(const void *msg, SIMCOM_CTX *ctx)
{
    return 0;
}

static int simcom_ping(const void *msg, SIMCOM_CTX *ctx)
{
    return 0;
}

static int simcom_alarm(const void *msg, SIMCOM_CTX *ctx)
{
    return 0;
}
