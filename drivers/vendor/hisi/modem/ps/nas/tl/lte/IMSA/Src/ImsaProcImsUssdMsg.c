

/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include "ImsaProcImsUssdMsg.h"
#include "ImsaEntity.h"
#include "ImsaPublic.h"
#include "ImsaProcUssdMsg.h"

/*lint -e767*/
#define    THIS_FILE_ID       PS_FILE_ID_IMSAPROCIMSUSSDMSG_C
#define    THIS_NAS_FILE_ID   NAS_FILE_ID_IMSAPROCIMSUSSDMSG_C
/*lint +e767*/

/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#if (FEATURE_ON == FEATURE_IMS)
/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/


/*****************************************************************************
  3 Function
*****************************************************************************/


VOS_VOID IMSA_ImsMsgUssdEvent(VOS_VOID *pUssdEvent)
{
    IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssd = VOS_NULL_PTR;

    pstOutUssd = (IMSA_IMS_OUTPUT_USSD_EVENT_STRU*)pUssdEvent;

    /*lint -e788*/
    switch (pstOutUssd->enOutputUssdReason)
    {
        case IMSA_IMS_OUTPUT_USSD_REASON_OK:
            IMSA_ProcImsMsgUssdReasonOk(pstOutUssd);
            break;
        case IMSA_IMS_OUTPUT_USSD_REASON_NOTIFY_EVENT:
            IMSA_ProcImsMsgUssdReasonNotify(pstOutUssd);
            break;
        case IMSA_IMS_OUTPUT_USSD_REASON_REQUEST_EVENT:
            IMSA_ProcImsMsgUssdReasonRequest(pstOutUssd);
            break;
        case IMSA_IMS_OUTPUT_USSD_REASON_DISCONNECT_EVENT:
            IMSA_ProcImsMsgUssdReasonDisconnect(pstOutUssd);
            break;
        case IMSA_IMS_OUTPUT_USSD_REASON_ERROR:
            IMSA_ProcImsMsgUssdReasonError(pstOutUssd);
            break;
        default:
            IMSA_INFO_LOG("IMSA_ImsMsgParaEvent: reason is err");
            break;

    }/*lint +e788*/
}


VOS_VOID IMSA_ProcImsMsgUssdReasonOk(IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssdEvt)
{
    IMSA_USSD_MANAGER_STRU              *pstUssdManager;

    IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonOk is entered");

    (void)pstOutUssdEvt;
    pstUssdManager = IMSA_USSD_GetEntityAddress();

    if (IMSA_USSD_MO_CONN_STATE == pstUssdManager->enUssdState)
    {

       (VOS_VOID)IMSA_SendSpmUssdDataSndEvt(pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId);
    }
    return;
}


VOS_VOID IMSA_ProcImsMsgUssdReasonNotify(const IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssdEvt)
{
    IMSA_USSD_MANAGER_STRU              *pstUssdManager;
    TAF_SS_USSD_STRING_STRU             stUssdString = {0};

    IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonNotify is entered");

    pstUssdManager = IMSA_USSD_GetEntityAddress();

    if (IMSA_USSD_MT_CONN_STATE == pstUssdManager->enUssdState)
    {
        IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonNotify:state is err");
        return;
    }

    stUssdString.usCnt = pstOutUssdEvt->usMessageLen;
    IMSA_MEM_CPY(                   stUssdString.aucUssdStr,
                                    pstOutUssdEvt->aucMessage,
                                    pstOutUssdEvt->usMessageLen);

    if (IMSA_USSD_MO_CONN_STATE == pstUssdManager->enUssdState)
    {
        /*停止等待网侧响应定时器*/
        IMSA_StopTimer(&(pstUssdManager->stUssdWaitNetRspTimer));

        (VOS_VOID)IMSA_SendSpmUssdNotifyIndEvt(  pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId,
                                    pstOutUssdEvt->encType,
                                    &stUssdString);
    }
    else
    {
        /*USSD在状态IMSA_USSD_IDLE_STATE的处理*/
        /*网侧主动触发流程*/
        (VOS_VOID)IMSA_SendSpmUssdNotifyIndEvt(  0x3fff,
                                        0,
                                    pstOutUssdEvt->encType,
                                    &stUssdString);
    }


    /*启动等待APP响应定时器*/
    IMSA_StartTimer(&(pstUssdManager->stUssdWaitAppRspTimer));
    pstUssdManager->enUssdState = IMSA_USSD_MT_CONN_STATE;
    return;
}


VOS_VOID IMSA_ProcImsMsgUssdReasonRequest(const IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssdEvt)
{
    IMSA_USSD_MANAGER_STRU              *pstUssdManager;
    TAF_SS_USSD_STRING_STRU             stUssdString = {0};

    IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonRequest is entered");

    pstUssdManager = IMSA_USSD_GetEntityAddress();

    if (IMSA_USSD_MT_CONN_STATE == pstUssdManager->enUssdState)
    {
        IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonRequest:state is err");
        return;
    }

    stUssdString.usCnt = pstOutUssdEvt->usMessageLen;
    IMSA_MEM_CPY(                   stUssdString.aucUssdStr,
                                    pstOutUssdEvt->aucMessage,
                                    pstOutUssdEvt->usMessageLen);

    if (IMSA_USSD_MO_CONN_STATE == pstUssdManager->enUssdState)
    {
        /*停止等待网侧响应定时器*/
        IMSA_StopTimer(&(pstUssdManager->stUssdWaitNetRspTimer));

        (VOS_VOID)IMSA_SendSpmUssdReqIndEvt(  pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId,
                                    pstOutUssdEvt->encType,
                                    &stUssdString);
    }
    else
    {
        /*USSD在状态IMSA_USSD_IDLE_STATE的处理*/
        /*网侧主动触发流程*/
        (VOS_VOID)IMSA_SendSpmUssdReqIndEvt(  0x3fff,
                                        0,
                                    pstOutUssdEvt->encType,
                                    &stUssdString);
    }


    /*启动等待APP响应定时器*/
    IMSA_StartTimer(&(pstUssdManager->stUssdWaitAppRspTimer));
    pstUssdManager->enUssdState = IMSA_USSD_MT_CONN_STATE;
    return;
}


VOS_VOID IMSA_ProcImsMsgUssdReasonDisconnect(IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssdEvt)
{
    IMSA_USSD_MANAGER_STRU              *pstUssdManager;

    IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonDisconnect is entered");

    pstUssdManager = IMSA_USSD_GetEntityAddress();
    (void)pstOutUssdEvt;

    if (IMSA_USSD_IDLE_STATE == pstUssdManager->enUssdState)
    {
        IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonDisconnect:state is err");
        return;
    }

    if (IMSA_USSD_MO_CONN_STATE == pstUssdManager->enUssdState)
    {
        /*停止等待网侧响应定时器*/
        IMSA_StopTimer(&(pstUssdManager->stUssdWaitNetRspTimer));

        (VOS_VOID)IMSA_SendSpmUssdRelCompleteIndEvt(  pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId);

    }
    else
    {
        /*USSD在状态IMSA_USSD_MT_CONN_STATE的处理*/
        /*停止等待APP响应定时器*/
        IMSA_StopTimer(&(pstUssdManager->stUssdWaitAppRspTimer));

        (VOS_VOID)IMSA_SendSpmUssdRelCompleteIndEvt( 0x3fff,
                                    0);
    }

    pstUssdManager->enUssdState = IMSA_USSD_IDLE_STATE;

    return;
}
VOS_VOID IMSA_ProcImsMsgUssdReasonError(const IMSA_IMS_OUTPUT_USSD_EVENT_STRU *pstOutUssdEvt)
{
    IMSA_USSD_MANAGER_STRU              *pstUssdManager;

    IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonError is entered");

    pstUssdManager = IMSA_USSD_GetEntityAddress();

    if (IMSA_USSD_MO_CONN_STATE != pstUssdManager->enUssdState)
    {
        IMSA_INFO_LOG("IMSA_ProcImsMsgUssdReasonError: state is err");
        return;
    }

    /*停止等待网侧响应定时器*/
    IMSA_StopTimer(&(pstUssdManager->stUssdWaitNetRspTimer));

    /*IMS本地导致的错误等确认后更新*/
    if ((300 > pstOutUssdEvt->stErrorCode.usSipStatusCode) || (699 < pstOutUssdEvt->stErrorCode.usSipStatusCode))
    {
        /*IMS本地导致的失败*/
        (VOS_VOID)IMSA_SendSpmUssdErrorEvt(  pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId,
                                    pstOutUssdEvt->stErrorCode.usSipStatusCode + TAF_ERR_SS_IMSA_BASE);
    }
    else
    {
        /*网侧导致的失败*/
        /*(VOS_VOID)IMSA_SendSpmUssdReqCnfEvt(   pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId,
                                    (VOS_UINT8)pstOutUssdEvt->stErrorCode.usSipStatusCode);*/
        (VOS_VOID)IMSA_SendSpmUssdErrorEvt(  pstUssdManager->usClientId,
                                    pstUssdManager->ucOpId,
                                    pstOutUssdEvt->stErrorCode.usSipStatusCode + TAF_ERR_SS_IMS_BASE);
    }

    pstUssdManager->enUssdState = IMSA_USSD_IDLE_STATE;

    return;
}


#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
/* end of ImsaProcImsUssdMsg.c */









