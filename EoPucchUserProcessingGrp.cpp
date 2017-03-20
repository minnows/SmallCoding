// Copyright 2017 Nokia. All rights reserved.

#include "EoPucchUserProcessingGrp.hpp"

#include <glo_def.h>
#include <LtefeMsg.h>

#include "Common/ArmAndDspSharedData.h"
#include "Common/Definitions/EEventGroupIndex.h"
#include "Common/Definitions/EPhyCellIndex.h"
#include "Common/Definitions/EQueueIndex.h"
#include "Common/Definitions/SystemLog.h"
#include "Common/EventUtils.hpp"
#include "Common/EventUtils/SwEvent.hpp"
#include "Common/LteFeUlInterface.hpp"
#include "Common/memory_barrier.hpp"
#include "Common/Profiling.h"
#include "Common/StartUp/PhyEventGroups.hpp"
#include "Common/StartUp/PhyHandles.h"
#include "Common/UlPhyInternalEventIds.h"
#include "Common/UlPhyMetadata.h"
#include "Config/MemorySections.h"
#include "EoPucchMeasCombiningGrp.hpp"
#include "PucchBuffers.hpp"
#include "UlPhyControl/ResourceManager/PucchCommon.hpp"


namespace ulphy {
namespace pucch {

// FIXME where does value 18 come from?
#define PUCCH_EVENT_GROUP_MAX_NOTIF (18)

PucchUserGrpControl      gPucchUserGrpControl [EPhyCellIndex_MaxNum][SYS_SUBFRAMES_PER_FRAME][SYS_SLOTS_PER_SUBFRAME]   DATA_EXT_APP_INIT;
SUserProcGrpBufferEvents gUeProcGrpEventsTable[EPhyCellIndex_MaxNum][MAX_EVENT_GROUP_WINDOW_SIZE]                       DATA_EXT_APP_NO_INIT;

void ResetNotifyTable(em_notif_t *notifTbl, const uint32_t notifNum)
{
    for (uint32_t i = 0; i < notifNum; i++)
    {
        checked_em_free( notifTbl[i].event );
    }
}

ERetStatus ClearEventGroupPlan(em_event_group_t group)
{
    em_notif_t notifTbl[PUCCH_EVENT_GROUP_MAX_NOTIF];

    // If the group is not free, create new one and delete the previous group.
    if (!em_event_group_is_ready(group))
    {
        uint32_t notifNum = em_event_group_get_notif (group, PUCCH_EVENT_GROUP_MAX_NOTIF, notifTbl);
        if (EM_OK == em_event_group_abort(group))
        {
            ResetNotifyTable(notifTbl, notifNum);
        }
        else
        {
            ERR_LOG("em_event_group_abort() failed at an un-ready group("EM_EVT_GRP_FMT")", group);
            return ERetStatus_Failed;
        }
    }
    return ERetStatus_Success;
}

ERetStatus PrepareUserProcessingGroup(const int32_t            f1UsersGroupLoopCnt,
                                      const int32_t            f2UsersGroupLoopCnt,
                                      const int32_t            totalPowGrpLoopCnt,
                                      SUlPhyMetadata           &metadata,
                                      SUserProcGrpBufferEvents *eventsTable)
{
    //initialization before goto
    em_event_group_t sendGroup;
    em_event_group_t userProcEventGroup;
    em_status_t      status;

    uint16_t cellIdx         = metadata.cellIdx;
    uint16_t timeWindow      = metadata.timeWindowIdx;
    uint16_t subframe        = metadata.subframe;
    uint32_t numEventsToWait = slotsPerSubframe * GetSharedPucchCellParamsPtr((EPhyCellIndex)cellIdx)->numOfAntennaPairs[subframe];
    em_notif_t notifEvts[maxNotifNum];
    uint32_t notifNum = 0;
    uint32_t notifIdx = 0;

    //F1 notification
    em_queue_t sendQueue = GetPhyQueueHandle(EQueueIndex_PucchSQ2, cellIdx);
    if (EM_QUEUE_UNDEF == sendQueue)
    {
        ERR_LOG("GetPhyQueueHandle failed");
        goto ERR_EXIT_1;
    }

    for (int32_t f1EvtIdx = 0; f1EvtIdx < f1UsersGroupLoopCnt; f1EvtIdx++)
    {
        SwEvent swEventF1;
        // First segment: Npe
        if (not swEventF1.Alloc(ULPHY_EVENT_ID_PUCCH_F1_PROCESSING, sizeof(NpeCombinedBuffers)))
        {
            ERR_LOG("sw event allocation failed for ULPHY_EVENT_ID_PUCCH_F1_PROCESSING (1st segment)");
            goto ERR_EXIT_1;
        }
        // Second segment: f1EventInput
        if (not swEventF1.AddSegment(sizeof(F1EventInput)))
        {
            ERR_LOG("sw event segment add failed for ULPHY_EVENT_ID_PUCCH_F1_PROCESSING (2nd segment)");
            goto ERR_EXIT_1;
        }

        metadata.segmentIdx = f1EvtIdx;

        swEventF1.SetMetadata(metadata);

        notifIdx = f1EvtIdx;
        // set event group table
        notifEvts[notifIdx].event  = swEventF1.GetEventPtr();
        notifEvts[notifIdx].queue  = sendQueue;
        notifEvts[notifIdx].egroup = EM_EVENT_GROUP_UNDEF;
        eventsTable->pucchF1Evt[f1EvtIdx] = swEventF1.GetEventPtr();
        notifNum++;
    }

    //F2 notification
    sendQueue = GetPhyQueueHandle(EQueueIndex_PucchSQ3, cellIdx);
    if (EM_QUEUE_UNDEF == sendQueue)
    {
        ERR_LOG("GetPhyQueueHandle failed");
        goto ERR_EXIT_1;
    }

    for (int32_t f2EvtIdx = 0; f2EvtIdx < f2UsersGroupLoopCnt; f2EvtIdx++)
    {
        SwEvent swEventF2;
        // First segment: Npe
        if (not swEventF2.Alloc(ULPHY_EVENT_ID_PUCCH_F2_PROCESSING, sizeof(NpeCombinedBuffers)))
        {
            ERR_LOG("sw event allocation failed for ULPHY_EVENT_ID_PUCCH_F2_PROCESSING (1st segment)");
            goto ERR_EXIT_1;
        }
        // Second segment: f2EventInput
        if (not swEventF2.AddSegment(sizeof(F2EventInput)))
        {
            ERR_LOG("sw event segment add failed for ULPHY_EVENT_ID_PUCCH_F2_PROCESSING (2nd segment)");
            goto ERR_EXIT_1;
        }

        metadata.segmentIdx = f2EvtIdx;

        swEventF2.SetMetadata(metadata);

        notifIdx = f2EvtIdx + f1UsersGroupLoopCnt;
        // set event group table
        notifEvts[notifIdx].event  = swEventF2.GetEventPtr();
        notifEvts[notifIdx].queue  = sendQueue;
        notifEvts[notifIdx].egroup = EM_EVENT_GROUP_UNDEF;
        eventsTable->pucchF2Evt[f2EvtIdx] = swEventF2.GetEventPtr();
        notifNum++;
    }

    //Total power, NpeWeighted notification
    sendQueue = GetPhyQueueHandle(EQueueIndex_Pucch_MeasCombining, cellIdx);

    if (EM_QUEUE_UNDEF == sendQueue)
    {
        ERR_LOG("GetPhyQueueHandle failed");
        goto ERR_EXIT_1;
    }
    sendGroup = GetPhyEventGroupHandle(EEventGroupIndex_PucchMeasCombining,
                                       static_cast<EPhyCellIndex>(cellIdx),
                                       timeWindow);
    if (EM_EVENT_GROUP_UNDEF == sendGroup)
    {
        ERR_LOG("GetPhyEventGroupHandle failed");
        goto ERR_EXIT_1;
    }

    for (int32_t powerEvtIdx = 0; powerEvtIdx < totalPowGrpLoopCnt; powerEvtIdx++)
    {
        SwEvent swEventUPOut;

        if (not swEventUPOut.Alloc(ULPHY_EVENT_ID_PUCCH_USER_PROCESSING_OUT, sizeof(UserProcOutputPrbInfo)))
        {
            ERR_LOG("sw event allocation failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING_OUT");
            goto ERR_EXIT_1;
        }

        notifIdx = powerEvtIdx + f1UsersGroupLoopCnt + f2UsersGroupLoopCnt;
        // set event group table
        notifEvts[notifIdx].event  = swEventUPOut.GetEventPtr();
        notifEvts[notifIdx].queue  = sendQueue;
        notifEvts[notifIdx].egroup = sendGroup;
        eventsTable->pucchPrbInfoEvt[powerEvtIdx] = swEventUPOut.GetEventPtr();
        notifNum++;
    }

    userProcEventGroup = GetPhyEventGroupHandle(EEventGroupIndex_PucchUserProcessing,
                                                (EPhyCellIndex)cellIdx,
                                                timeWindow);
    if (EM_EVENT_GROUP_UNDEF == userProcEventGroup)
    {
        ERR_LOG("GetPhyEventGroupHandle failed");
        goto ERR_EXIT_1;
    }
    if (ERetStatus_Failed == ClearEventGroupPlan(userProcEventGroup))
    {
        ERR_LOG("ClearEventGroupPlan failed");
        goto ERR_EXIT_1;
    }

    status = em_event_group_apply(userProcEventGroup,
                                  numEventsToWait,
                                  notifNum,
                                  notifEvts);
    if (EM_OK != status)
    {
        ERR_LOG("PucchUserProcessingGrp: Failed to apply User Processing event group");
        goto ERR_EXIT_1;
    }
    return ERetStatus_Success;

ERR_EXIT_1:
    ResetNotifyTable(notifEvts, notifNum);
    return ERetStatus_Failed;
}

bool CreateProcessingResultsEvent(SwEvent              &event,
                                  const SUlPhyMetadata &metadata)
{
    uint32_t cellIdx = static_cast<uint32_t>(metadata.cellIdx);
    uint32_t timeWindow = static_cast<uint32_t>(metadata.timeWindowIdx);

    // First segment with SW event header: PucchF1Combine
    bool createOk = event.Alloc(ULPHY_EVENT_ID_PUCCH_PROCESSING_RESULTS,
                                sizeof(g_pucchFmt1Output[cellIdx][timeWindow].payload),
                                &g_pucchFmt1Output[cellIdx][timeWindow]);
    if (not createOk)
    {
        ERR_LOG("sw event segment allocation failed for ULPHY_EVENT_ID_PUCCH_PROCESSING_RESULTS");
        return false;
    }

    // Second segment: PucchF2Combine
    if (not event.AddSegment(sizeof(PucchF2Combine), &g_pucchFmt2Output[cellIdx][timeWindow]))
    {
        ERR_LOG("sw event segment add failed for ULPHY_EVENT_ID_PUCCH_PROCESSING_RESULTS (2nd segment)");
        return false;
    }

    // Third segment: PucchPrbInfoCombine
    if (not event.AddSegment(sizeof(PucchPrbInfoCombine), &g_pucchPrbInfoOutput[cellIdx][timeWindow]))
    {
        ERR_LOG("sw event segment add failed for ULPHY_EVENT_ID_PUCCH_PROCESSING_RESULTS (3rd segment)");
        return false;
    }

    event.SetMetadata(metadata);
    return true;
}

ERetStatus PrepareMeasCombiningGrp(const int32_t        f1UsersGroupLoopCnt,
                                   const int32_t        f2UsersGroupLoopCnt,
                                   const int32_t        totalPowGrpLoopCnt,
                                   const SUlPhyMetadata &metadata)
{
    em_status_t status;

    uint32_t numEventsToWait = f1UsersGroupLoopCnt + f2UsersGroupLoopCnt + totalPowGrpLoopCnt;
    uint16_t cellIdx         = metadata.cellIdx;
    uint16_t timeWindow      = metadata.timeWindowIdx;
    const uint32_t notifNum  = 1;

    SwEvent notifyEvent;
    if (not CreateProcessingResultsEvent(notifyEvent, metadata))
    {
        return ERetStatus_Failed;
    }

    g_PucchFormat12NotifEvts[cellIdx][timeWindow].event = notifyEvent.GetEventPtr();
    g_PucchFormat12NotifEvts[cellIdx][timeWindow].queue = GetPhyQueueHandle(EQueueIndex_Pucch_ResultProcessing,
                                                                            cellIdx);
    g_PucchFormat12NotifEvts[cellIdx][timeWindow].egroup = EM_EVENT_GROUP_UNDEF;
    em_notif_t *notifEvts = &g_PucchFormat12NotifEvts[cellIdx][timeWindow];

    em_event_group_t combiningEventGroup;
    combiningEventGroup = GetPhyEventGroupHandle(EEventGroupIndex_PucchMeasCombining,
                                        (EPhyCellIndex)cellIdx,
                                        timeWindow);
    if (EM_EVENT_GROUP_UNDEF == combiningEventGroup)
    {
        ERR_LOG("GetPhyEventGroupHandle failed");
        goto ERR_EXIT_1;
    }
    if (ERetStatus_Failed == ClearEventGroupPlan(combiningEventGroup))
    {
        ERR_LOG("ClearEventGroupPlan failed");
        goto ERR_EXIT_1;
    }
    status = em_event_group_apply(combiningEventGroup,
                                  numEventsToWait,
                                  notifNum,
                                  notifEvts);
    if (EM_OK != status)
    {
        ERR_LOG("PucchUserProcessing: Failed to apply MeasCombining event group");
        goto ERR_EXIT_1;
    }
    return ERetStatus_Success;

ERR_EXIT_1:
    ResetNotifyTable(&g_PucchFormat12NotifEvts[cellIdx][timeWindow], notifNum);
    return ERetStatus_Failed;
}

void ClearUserGrpControl(PucchUserGrpControl *pucchUserGrpControl, const int32_t antIdxStart)
{
    for (int32_t antIdx = 0; antIdx < SYS_RX_TWO_ANTENNAS; antIdx++)
    {
        pucchUserGrpControl->event_slotReadyIndAnt[antIdxStart + antIdx] = EM_EVENT_UNDEF;
    }
}

void FreeUserGrpControl(PucchUserGrpControl *pucchUserGrpControl, const int32_t antIdxStart)
{
    for (int32_t antIdx = 0; antIdx < SYS_RX_TWO_ANTENNAS; antIdx++)
    {
        if (EM_EVENT_UNDEF != pucchUserGrpControl->event_slotReadyIndAnt[antIdxStart + antIdx])
            em_free(pucchUserGrpControl->event_slotReadyIndAnt[antIdxStart + antIdx]);
    }
}


em_status_t ResetEoPucchUserProcessingGrp(void* eo_ctx, em_eo_t eo)
{
    for (int32_t cellIdx = 0; cellIdx < EPhyCellIndex_MaxNum; cellIdx++)
    {
        for (int32_t subframeIdx = 0; subframeIdx < SYS_SUBFRAMES_PER_FRAME; subframeIdx++)
        {
            for (int32_t slotInSfr = 0; slotInSfr < SYS_SLOTS_PER_SUBFRAME; slotInSfr++)
            {
                for (int32_t antIdx = 0; antIdx < MAX_ANT_NUM_SYS; antIdx++)
                {
                    gPucchUserGrpControl[cellIdx][subframeIdx][slotInSfr].event_slotReadyIndAnt[antIdx] = EM_EVENT_UNDEF;
                }
            }
        }
    }
    return EM_OK;
}

void FreeAndClearUserGrpControl(PucchUserGrpControl *controlTable, const int32_t antIdxStart)
{
    FreeUserGrpControl(controlTable, antIdxStart);
    ClearUserGrpControl(controlTable, antIdxStart);
}

static void SendPucchUserProcessingEvent(PucchUserGrpControl *pucchUserGrpControl,
                                         const int32_t       antIdxStart,
                                         SUlPhyMetadata      &metaData)
{
    int32_t            slotInFrame         = metaData.slotIdx + slotsPerSubframe*metaData.subframe;
    SSharedPucchParams *pucchCellParamsPtr = GetSharedPucchCellParamsPtr((EPhyCellIndex)metaData.cellIdx);
    void               *src                = pucchCellParamsPtr->pucchBaseSeqShifted + (slotInFrame)*carriersPerPrb*symbolsPerSlotAligned;

    // PUCCH UserProcessing 2ant data segment
    SwEvent swEventUP;
    if (not swEventUP.Set(pucchUserGrpControl->event_slotReadyIndAnt[antIdxStart]))
    {
        ERR_LOG("sw event set failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING");
        FreeAndClearUserGrpControl(pucchUserGrpControl, antIdxStart);
        return;
    }

    SwEvent swEventAnt2;
    if (not swEventAnt2.Set(pucchUserGrpControl->event_slotReadyIndAnt[antIdxStart + 1]))
    {
        ERR_LOG("sw event set failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING (2nd antenna)");
        FreeAndClearUserGrpControl(pucchUserGrpControl, antIdxStart);
        return;
    }

    if (not swEventUP.Combine(swEventAnt2)) // combine 2nd segment
    {
        ERR_LOG("sw event combining failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING");
        return;
    }

    if (not swEventUP.AddSegment(sizeof(SingleSlotBaseSeqContainer))) // 3rd segment
    {
        ERR_LOG("sw event segment add failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING (3rd segment)");
        return;
    }

    void *pucchBaseSeqPtr = swEventUP.GetSegmentPayload(2, ESwEventHeader_None);
    memcpy_barrier(pucchBaseSeqPtr, src, sizeof(SingleSlotBaseSeqContainer));

    swEventUP.SetId(ULPHY_EVENT_ID_PUCCH_USER_PROCESSING);

    swEventUP.SetMetadata(metaData);

    em_queue_t sndQ = GetPhyQueueHandle(EQueueIndex_PucchSQ1, 0);

    if (EM_QUEUE_UNDEF == sndQ)
    {
        ERR_LOG("GetPhyQueueHandle failed");
        swEventUP.Free();
        return;
    }

    em_event_group_t sndGrp = GetPhyEventGroupHandle(EEventGroupIndex_PucchUserProcessing,
                                                     static_cast<EPhyCellIndex>(metaData.cellIdx),
                                                     metaData.timeWindowIdx);
    if (EM_EVENT_GROUP_UNDEF == sndGrp)
    {
        ERR_LOG("GetPhyEventGroupHandle failed");
        swEventUP.Free();
        return;
    }

    if (not swEventUP.SendGroup(sndQ, sndGrp))
    {
        ERR_LOG("sw event SendGroup failed for ULPHY_EVENT_ID_PUCCH_USER_PROCESSING");
        return;
    }
    pucchUserGrpControl->numOfAntennasSent += SYS_RX_TWO_ANTENNAS;
}

static ERetStatus PreprocessUserInfo(SwEvent          swEvent,
                                     SUlPhyMetadata   pucchMetadata,
                                     TtiStorage       *tti)
{
    uint32_t cellIdx       = pucchMetadata.cellIdx;
    uint32_t timeWindowIdx = pucchMetadata.timeWindowIdx;
    uint32_t slotIdx       = pucchMetadata.slotIdx;
    uint32_t antIdx        = pucchMetadata.antIdx;
    uint32_t subframe      = pucchMetadata.subframe;
    uint32_t antIdxStart   = antIdx & 0xFE;

    SUserProcGrpBufferEvents *eventsTable  = &gUeProcGrpEventsTable[cellIdx][timeWindowIdx];
    PucchUserGrpControl *controlTable = &gPucchUserGrpControl[cellIdx][subframe][slotIdx];
    // PUCCH slot event from LTEFE/SW channelizer
    controlTable->event_slotReadyIndAnt[antIdx] = swEvent.GetEventPtr();

    INF_LOG("UserProcessing Grp DataReceived cellIdx %d subframe %d slot %d antIdx %d",
            cellIdx, subframe, slotIdx, antIdx);

    if ((0 == antIdx) and (0 == slotIdx))
    {
        UlPhyPucchReceiveReq& reqData = tti->requests[cellIdx].pucchRReq;
        AlgoContext*          algoContext     = &reqData.algoContext;

        int32_t               numOfAntennas   = algoContext->antCount;

        SSharedPucchParams*   pucchCellParamsPtr = GetSharedPucchCellParamsPtr((EPhyCellIndex)cellIdx);

        controlTable->numOfAntennas = numOfAntennas;

        const int32_t         numOfF1Users = GetTotalF1(algoContext->dspInstCount);
        const int32_t         numOfF2Users = GetTotalF2(algoContext->dspInstCount);
        const int32_t         f1UsersGroupLoopCnt = (numOfF1Users == 0 ? 0 : ((numOfF1Users-1)/maxF1InstPerEvent + 1));
        const int32_t         f2UsersGroupLoopCnt = (numOfF2Users == 0 ? 0 : ((numOfF2Users-1)/maxF2InstPerEvent + 1));
        const int32_t         totalPowGrpLoopCnt = slotsPerSubframe*pucchCellParamsPtr->numOfAntennaPairs[pucchMetadata.subframe];

        if (ERetStatus_Success != PrepareMeasCombiningGrp(f1UsersGroupLoopCnt, f2UsersGroupLoopCnt, totalPowGrpLoopCnt, pucchMetadata))
        {
            ERR_LOG("PrepareMeasCombiningGrp failed");
            FreeAndClearUserGrpControl(controlTable, antIdxStart);
            return ERetStatus_Failed;
        }
        if (ERetStatus_Success != PrepareUserProcessingGroup(f1UsersGroupLoopCnt, f2UsersGroupLoopCnt, totalPowGrpLoopCnt, pucchMetadata, eventsTable))
        {
            ERR_LOG("PrepareUserProcessingGroup failed");
            FreeAndClearUserGrpControl(controlTable, antIdxStart);
            return ERetStatus_Failed;
        }
    }

    if (controlTable->event_slotReadyIndAnt[antIdxStart] and
        controlTable->event_slotReadyIndAnt[antIdxStart + 1])
    {
        SendPucchUserProcessingEvent(controlTable, antIdxStart, pucchMetadata);
        ClearUserGrpControl(controlTable, antIdxStart);
    }
    if ((controlTable->numOfAntennas > 0) and
        (controlTable->numOfAntennasSent == controlTable->numOfAntennas))
    {
        controlTable->numOfAntennas = 0;
        controlTable->numOfAntennasSent = 0;
    }
    return ERetStatus_Success;
}

void EoPucchUserProcessingGrpReceiveEvent(void * /*eo_ctx*/,
                                          em_event_t event,
                                          em_event_type_t type,
                                          em_queue_t,
                                          void * /*q_ctx*/)
{
    PROFILE_THIS_FUNCTION;

    SwEvent receivedEvent;
    if (not receivedEvent.Set(event))
    {
        WRN_LOG("Unexpected event received with type 0x%X, not an sw event", em_get_event_type(event));
        em_free(event);
        return;
    }

    if (receivedEvent != LTEFE_PUCCH_IND_MSG)
    {
        WRN_LOG("Unexpected sw event received (0x%X)", receivedEvent.GetId());
        receivedEvent.Free();
        return;
    }
    DBG_LOG("received sw event (0x%X)", receivedEvent.GetId());
    
    SUlPhyMetadata pucchMetadata;
    const ltefe_ul_pucch_event_context *context = receivedEvent.GetContext<ltefe_ul_pucch_event_context>();
    LteFeUlInterface::ConvertPucchLteFeEventContextToMetadata(context, &pucchMetadata);

    TtiStorage  *tti = GetTtiStorage(pucchMetadata.subframe);
    if ((GLO_TRUE != tti->thisTTIHasData) or
        (GLO_TRUE != tti->requests[pucchMetadata.cellIdx].pucchAvailable))
    {
        INF_LOG("No TTI schedule info. cellIdx %d subframe %d TTI data.(pucchAvailable) %d.(%d)",
                 pucchMetadata.cellIdx, 
                 pucchMetadata.subframe, 
                 tti->thisTTIHasData, 
                 tti->requests[pucchMetadata.cellIdx].pucchAvailable);
        em_free(event);
    }
    else
    {
        PreprocessUserInfo(receivedEvent, pucchMetadata, tti);
    }
}

} }
