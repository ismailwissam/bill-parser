#include <iostream>
#include "tlog.hpp"
#include "asn1_bill.hpp"
#include "hw_asn1.hpp"

/*!
 * 构造函数
 */
HwA1Bill::HwA1Bill(const char* name) :
    BillBase(name),
    hwa1_BCDZipBEFmt(true, false, true),
    hwa1_DebugFmt(true, true, false),
    hwa1_UIntLEFmt(false)
{
}

/*!
 * 析构函数
 */
HwA1Bill::~HwA1Bill()
{
    for(HW_A1_NODE_LIST::iterator loc_It = hwa1_NodeList.begin();
            loc_It != hwa1_NodeList.end(); ++loc_It)
    {
        // 删除所有的value值
        delete (*loc_It).value;
    }
}

/*!
 * 返回字段名称
 */
const char* HwA1Bill::billbaseItemName(int idx)
{
    if(idx < 0 || idx >= hwa1_NodeList.size())
    {
        return NULL;
    }

    return hwa1_NodeList[idx].name.c_str();
}

/*!
 * 读取话单数据
 * 解析后的结果必须将有效的字段保存在 billbase_NodeMap 中
 */
bool HwA1Bill::billbaseReadData(std::istream& in)
{
    // 清除之前保留的所有话单
    billbaseReset();

    int loc_Tag;
    size_t loc_Len;
    std::string loc_Data;

    HW_A1_TAG_MAP::iterator loc_It;
    int loc_NodeIdx;

    while(1)
    {
        if(!hwasn1GetTag(in, loc_Tag))
        {
            if(in.eof())
            {
                break;
            }
            else
            {
                LOG_STR_LN("读取Tag失败");
                return false;
            }
        }

        if(!hwasn1GetLen(in, loc_Len))
        {
            LOG_STR_LN("读取Len失败");
            return false;
        }

        if(!hwasn1GetCont(in, loc_Len, loc_Data))
        {
            LOG_STR_LN("读取Content失败");
            return false;
        }

        loc_It = hwa1_TagMap.find(loc_Tag);
        if(loc_It == hwa1_TagMap.end())
        {
            // 发现未知Tag
            continue;
        }

        // 获取Tag对应的node 在列表中的索引
        loc_NodeIdx = (*loc_It).second;

        // 设置值
        HwA1Node& loc_Node = hwa1_NodeList[loc_NodeIdx];
        loc_Node.value->dynlenvalSetData(loc_Data);

        // 追加到列表中
        billbaseAddNode(loc_Node.name.c_str(), loc_Node.value, loc_Node.format);

        LOG_STR_LN(loc_Node.name.c_str());
    }

    return true;
}


// ----------------------------------- HwA1MocBill -----------------------------------
//
//                  华为ASN.1 主叫话单格式说明
//
// **********************************************************
// 名称                                          Tag
// **********************************************************
//                     recordType                0x80
//                     servedIMSI                0x81
//                     servedIMEI                0x82
//                   servedMSISDN                0x83
//                  callingNumber                0x84
//                   calledNumber                0x85
//               translatedNumber                0x86
//                connectedNumber                0x87
//                  roamingNumber                0x88
//                recordingEntity                0x89
//               mscIncomingROUTE                0xAA
//               mscOutgoingROUTE                0xAB
//                       location                0xAC
//               changeOfLocation                0xAD
//                   basicService                0xAE
//          transparencyIndicator                0x8F
//                changeOfService                0xB0
//              supplServicesUsed                0xB1
//                  aocParameters                0xB2
//               changeOfAOCParms                0xB3
//                    msClassmark                0x94
//              changeOfClassmark                0xB5
//                      setupTime            0x9F8149
//                    seizureTime                0x96
//                   alertingTime            0x9F814A
//                     answerTime                0x97
//                    releaseTime                0x98
//                   callDuration                0x99
//             radioChanRequested                0x9B
//                  radioChanUsed                0x9C
//              changeOfRadioChan                0xBD
//                   causeForTerm                0x9E
//                    diagnostics              0xBF1F
//                  callReference              0x9F20
//                 sequenceNumber              0x9F21
//              additionalChgInfo              0xBF22
//               recordExtensions              0xBF23
//                 gsm-SCFAddress              0x9F24
//                     serviceKey              0x9F25
//           networkCallReference              0x9F26
//                     mSCAddress              0x9F27
//           cAMELInitCFIndicator              0x9F28
//            defaultCallHandling              0x9F29
//                           fnur              0x9F2D
//                  aiurRequested              0x9F2E
//         speechVersionSupported              0x9F31
//              speechVersionUsed              0x9F32
//          numberOfDPEncountered              0x9F33
//            levelOfCAMELService              0x9F34
//                 freeFormatData              0x9F35
//        cAMELCallLegInformation              0xBF36
//           freeFormatDataAppend              0x9F37
//          defaultCallHandling-2              0x9F38
//               gsm-SCFAddress-2              0x9F39
//                   serviceKey-2              0x9F3A
//               freeFormatData-2              0x9F3B
//         freeFormatDataAppend-2              0x9F3C
//                     systemType              0x9F3D
//                 rateIndication              0x9F3E
//              partialRecordType              0x9F45
//              guaranteedBitRate              0x9F46
//                 maximumBitRate              0x9F47
//                      modemType            0x9F810B
//                     classmark3            0x9F810C
//                   chargedParty            0x9F810D
//           originalCalledNumber            0x9F810E
//                 chargeAreaCode            0x9F8111
//           calledChargeAreaCode            0x9F8112
//             mscOutgoingCircuit            0x9F8126
//                  orgRNCorBSCId            0x9F8127
//                       orgMSCId            0x9F8128
//              callEmlppPriority            0x9F812A
//     callerDefaultEmlppPriority            0x9F812B
//               eaSubscriberInfo            0x9F812E
//                    selectedCIC            0x9F812F
//             optimalRoutingFlag            0x9F8131
//  optimalRoutingLateForwardFlag            0x9F8132
// optimalRoutingEarlyForwardFlag            0x9F8133
//                     portedflag            0x9F8134
//                     calledIMSI            0x9F8135
//                   globalAreaID            0x9F813C
//           changeOfglobalAreaID            0xBF813D
//             subscriberCategory            0x9F813E
//                    firstmccmnc            0x9F8140
//             intermediatemccmnc            0x9F8141
//                     lastmccmnc            0x9F8142
//     cUGOutgoingAccessIndicator            0x9F8143
//               cUGInterlockCode            0x9F8144
//          cUGOutgoingAccessUsed            0x9F8145
//                       cUGIndex            0x9F8146
//              interactionWithIP            0x9F8147
//                  hotBillingTag            0x9F8148
//                 voiceIndicator            0x9F814B
//                      bCategory            0x9F814C
//                       callType            0x9F814D
//         resourceChargeIPnumber            0x9F814E
//                  groupCallType            0x9F8150
//             groupCallReference            0x9F8151
//                       uus1Type            0x9F8152
//                      eCategory            0x9F8157
//                     tariffCode            0x9F8159
//                disconnectparty            0x9F815A
//                    csReference            0x9F815C
//                   csaReference            0x9F815D
//                     camelphase            0x9F815E
//              networkOperatorId            0x9F815F
//              typeOfSubscribers            0x9F8160
//                  audioDataType            0x9F8161
//                       userType            0x9F8163
//                   recordNumber            0x9F8168
//                       zoneCode            0x9F8170
//                        MCTType            0x9F8175
//                           cARP            0x9F8176
// **********************************************************

/*!
 * 构造函数
 */
HwA1MocBill::HwA1MocBill():HwA1Bill("华为ASN.1主叫话单")
{
    /*
     * 创建每个字段对应的值和格式的对象 : hwa1_NodeList
     * 创建每个Tag和字段的对应 : hwa1_TagMap
     */
    int loc_Idx = 0;
    hwa1_NodeList.push_back(HwA1Node("recordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x80] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMSI", new DynLenValue(), &hwa1_IsdnFmt)); hwa1_TagMap[0x81] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMEI", new DynLenValue(), &hwa1_IsdnFmt)); hwa1_TagMap[0x82] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedMSISDN", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x83] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callingNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x84] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x85] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("translatedNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x86] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("connectedNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x87] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("roamingNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x88] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordingEntity", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x89] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTE", new DynLenValue(), &hwa1_PStrFmt)); hwa1_TagMap[0xAA] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTE", new DynLenValue(), &hwa1_PStrFmt)); hwa1_TagMap[0xAB] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("location", new DynLenValue(), &hwa1_LacCiFmt)); hwa1_TagMap[0xAC] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfLocation", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAD] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("basicService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAE] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("transparencyIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x8F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB0] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("supplServicesUsed", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB1] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aocParameters", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB2] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfAOCParms", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB3] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("msClassmark", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x94] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfClassmark", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB5] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("setupTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x9F8149] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("seizureTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x96] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("alertingTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x9F814A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("answerTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x97] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("releaseTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x98] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callDuration", new DynLenValue(), &hwa1_UIntLEFmt)); hwa1_TagMap[0x99] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanRequested", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanUsed", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfRadioChan", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBD] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("causeForTerm", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("diagnostics", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF1F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F20] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("sequenceNumber", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F21] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("additionalChgInfo", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF22] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordExtensions", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF23] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("gsm-SCFAddress", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F24] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("serviceKey", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F25] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkCallReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F26] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mSCAddress", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F27] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cAMELInitCFIndicator", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F28] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("defaultCallHandling", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F29] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("fnur", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F2D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aiurRequested", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F2E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionSupported", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F31] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionUsed", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F32] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("numberOfDPEncountered", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F33] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("levelOfCAMELService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F34] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatData", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F35] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cAMELCallLegInformation", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF36] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatDataAppend", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F37] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("defaultCallHandling-2", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F38] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("gsm-SCFAddress-2", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F39] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("serviceKey-2", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F3A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatData-2", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F3B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatDataAppend-2", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F3C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("systemType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F3D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("rateIndication", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F3E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("partialRecordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F45] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("guaranteedBitRate", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F46] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("maximumBitRate", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F47] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("modemType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F810B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("classmark3", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F810C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargedParty", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F810D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("originalCalledNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F810E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargeAreaCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8111] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledChargeAreaCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8112] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingCircuit", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8126] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgRNCorBSCId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8127] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgMSCId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8128] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callEmlppPriority", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callerDefaultEmlppPriority", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eaSubscriberInfo", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("selectedCIC", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8131] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingLateForwardFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8132] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingEarlyForwardFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8133] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("portedflag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8134] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledIMSI", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8135] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("globalAreaID", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F813C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfglobalAreaID", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF813D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("subscriberCategory", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F813E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("firstmccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8140] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("intermediatemccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8141] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("lastmccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8142] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8143] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGInterlockCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8144] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessUsed", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8145] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGIndex", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8146] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("interactionWithIP", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8147] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("hotBillingTag", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8148] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("voiceIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("bCategory", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("resourceChargeIPnumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F814E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8150] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8151] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("uus1Type", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8152] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eCategory", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8157] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("tariffCode", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8159] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("disconnectparty", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F815A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F815C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csaReference", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F815D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("camelphase", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F815E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkOperatorId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F815F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("typeOfSubscribers", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8160] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("audioDataType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8161] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("userType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8163] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordNumber", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8168] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("zoneCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8170] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("MCTType", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8175] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cARP", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8176] = loc_Idx++;
}

// ----------------------------------- HwA1MtcBill -----------------------------------
//
//                  华为ASN.1 被叫话单格式说明
//
// **********************************************************
// 名称                                          Tag
// **********************************************************
//                    recordType                0x80
//                    servedIMSI                0x81
//                    servedIMEI                0x82
//                  servedMSISDN                0x83
//                 callingNumber                0x84
//               connectedNumber                0x85
//               recordingEntity                0x86
//              mscIncomingROUTE                0xA7
//              mscOutgoingROUTE                0xA8
//                      location                0xA9
//              changeOfLocation                0xAA
//                  basicService                0xAB
//         transparencyIndicator                0x8C
//               changeOfService                0xAD
//             supplServicesUsed                0xAE
//                 aocParameters                0xAF
//              changeOfAOCParms                0xB0
//                   msClassmark                0x91
//             changeOfClassmark                0xB2
//                     setupTime            0x9F814B
//                   seizureTime                0x93
//                  alertingTime            0x9F814C
//                    answerTime                0x94
//                   releaseTime                0x95
//                  callDuration                0x96
//            radioChanRequested                0x98
//                 radioChanUsed                0x99
//             changeOfRadioChan                0xBA
//                  causeForTerm                0x9B
//                   diagnostics                0xBC
//                 callReference                0x9D
//                sequenceNumber                0x9E
//             additionalChgInfo              0xBF1F
//              recordExtensions              0xBF20
//          networkCallReference              0x9F21
//                    mSCAddress              0x9F22
//                          fnur              0x9F26
//                 aiurRequested              0x9F27
//        speechVersionSupported              0x9F2A
//             speechVersionUsed              0x9F2B
//                gsm-SCFAddress              0x9F2C
//                    serviceKey              0x9F2D
//                    systemType              0x9F2E
//                rateIndication              0x9F2F
//             partialRecordType              0x9F36
//             guaranteedBitRate              0x9F37
//                maximumBitRate              0x9F38
//        initialCallAttemptFlag            0x9F8109
//              ussdCallBackFlag            0x9F810A
//                     modemType            0x9F810B
//                    classmark3            0x9F810C
//                  chargedParty            0x9F810D
//          originalCalledNumber            0x9F810E
//                chargeAreaCode            0x9F8111
//          calledChargeAreaCode            0x9F8112
//           defaultCallHandling            0x9F8116
//                freeFormatData            0x9F8117
//          freeFormatDataAppend            0x9F8118
//         numberOfDPEncountered            0x9F8119
//           levelOfCAMELService            0x9F811A
//                 roamingNumber            0x9F8120
//            mscIncomingCircuit            0x9F8126
//                 orgRNCorBSCId            0x9F8127
//                      orgMSCId            0x9F8128
//             callEmlppPriority            0x9F812A
//    calledDefaultEmlppPriority            0x9F812B
//              eaSubscriberInfo            0x9F812E
//                   selectedCIC            0x9F812F
//            optimalRoutingFlag            0x9F8131
//                    portedflag            0x9F8134
//                  globalAreaID            0x9F813C
//          changeOfglobalAreaID            0xBF813D
//            subscriberCategory            0x9F813E
//                   firstmccmnc            0x9F8140
//            intermediatemccmnc            0x9F8141
//                    lastmccmnc            0x9F8142
//    cUGOutgoingAccessIndicator            0x9F8143
//              cUGInterlockCode            0x9F8144
//         cUGIncomingAccessUsed            0x9F8145
//                      cUGIndex            0x9F8146
//                 hotBillingTag            0x9F8148
//             redirectingnumber            0x9F8149
//            redirectingcounter            0x9F814A
//                  calledNumber            0x9F814D
//                voiceIndicator            0x9F814E
//                     bCategory            0x9F814F
//                      callType            0x9F8150
//                 groupCallType            0x9F8153
//            groupCallReference            0x9F8154
//                      uus1Type            0x9F8155
//                     eCategory            0x9F8157
//                    tariffCode            0x9F8159
//               disconnectparty            0x9F815A
//                   csReference            0x9F815C
//                  csaReference            0x9F815D
//             networkOperatorId            0x9F815F
//             typeOfSubscribers            0x9F8160
//                 audioDataType            0x9F8161
//                      userType            0x9F8163
//                  recordNumber            0x9F8168
//              translatedNumber            0x9F8175
//                      zoneCode            0x9F8170
//                          cARP            0x9F8176
// **********************************************************
//
/*!
 * 构造函数
 */
HwA1MtcBill::HwA1MtcBill():HwA1Bill("华为ASN.1被叫话单")
{
    /*
     * 创建每个字段对应的值和格式的对象 : hwa1_NodeList
     * 创建每个Tag和字段的对应 : hwa1_TagMap
     */
    int loc_Idx = 0;
    hwa1_NodeList.push_back(HwA1Node("recordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x80] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMSI", new DynLenValue(), &hwa1_IsdnFmt)); hwa1_TagMap[0x81] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMEI", new DynLenValue(), &hwa1_IsdnFmt)); hwa1_TagMap[0x82] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedMSISDN", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x83] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callingNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x84] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("connectedNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x85] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordingEntity", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x86] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTE", new DynLenValue(), &hwa1_PStrFmt)); hwa1_TagMap[0xA7] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTE", new DynLenValue(), &hwa1_PStrFmt)); hwa1_TagMap[0xA8] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("location", new DynLenValue(), &hwa1_LacCiFmt)); hwa1_TagMap[0xA9] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfLocation", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAA] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("basicService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAB] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("transparencyIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x8C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAD] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("supplServicesUsed", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAE] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aocParameters", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xAF] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfAOCParms", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB0] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("msClassmark", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x91] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfClassmark", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xB2] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("setupTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x9F814B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("seizureTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x93] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("alertingTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x9F814C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("answerTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x94] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("releaseTime", new DynLenValue(), &hwa1_TimeFmt)); hwa1_TagMap[0x95] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callDuration", new DynLenValue(), &hwa1_UIntLEFmt)); hwa1_TagMap[0x96] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanRequested", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x98] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanUsed", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x99] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfRadioChan", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBA] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("causeForTerm", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("diagnostics", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBC] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("sequenceNumber", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("additionalChgInfo", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF1F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordExtensions", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF20] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkCallReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F21] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mSCAddress", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F22] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("fnur", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F26] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aiurRequested", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F27] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionSupported", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F2A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionUsed", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F2B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("gsm-SCFAddress", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F2C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("serviceKey", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F2D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("systemType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F2E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("rateIndication", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F2F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("partialRecordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F36] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("guaranteedBitRate", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F37] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("maximumBitRate", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F38] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("initialCallAttemptFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8109] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("ussdCallBackFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F810A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("modemType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F810B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("classmark3", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F810C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargedParty", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F810D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("originalCalledNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F810E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargeAreaCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8111] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledChargeAreaCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8112] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("defaultCallHandling", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8116] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatData", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8117] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatDataAppend", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8118] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("numberOfDPEncountered", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8119] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("levelOfCAMELService", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F811A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("roamingNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F8120] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscIncomingCircuit", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8126] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgRNCorBSCId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8127] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgMSCId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8128] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callEmlppPriority", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledDefaultEmlppPriority", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eaSubscriberInfo", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("selectedCIC", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F812F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingFlag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8131] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("portedflag", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8134] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("globalAreaID", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F813C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfglobalAreaID", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0xBF813D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("subscriberCategory", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F813E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("firstmccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8140] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("intermediatemccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8141] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("lastmccmnc", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8142] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8143] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGInterlockCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8144] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGIncomingAccessUsed", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8145] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGIndex", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8146] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("hotBillingTag", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8148] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("redirectingnumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F8149] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("redirectingcounter", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F814D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("voiceIndicator", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("bCategory", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F814F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8150] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8153] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8154] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("uus1Type", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8155] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eCategory", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8157] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("tariffCode", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8159] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("disconnectparty", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F815A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csReference", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F815C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csaReference", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F815D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkOperatorId", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F815F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("typeOfSubscribers", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8160] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("audioDataType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8161] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("userType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8163] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordNumber", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8168] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("translatedNumber", new DynLenValue(), &hwa1_NumberFmt)); hwa1_TagMap[0x9F8175] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("zoneCode", new DynLenValue(), &hwa1_DebugFmt)); hwa1_TagMap[0x9F8170] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cARP", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x9F8176] = loc_Idx++;
}

// ----------------------------------- HwA1TansitBill -----------------------------------
//
//                  华为ASN.1 汇接话单格式说明
//
// **********************************************************
//    名称                             Tag
// **********************************************************
//    recordType                       0x80         
//    recordingEntity                  0x81         
//    mscIncomingROUTE                 0xA2         
//    mscOutgoingROUTE                 0xA3         
//    callingNumber                    0x84         
//    calledNumber                     0x85         
//    isdnBasicService                 0x86         
//    seizureTime                      0x87         
//    answerTime                       0x88         
//    releaseTime                      0x89         
//    callDuration                     0x8A         
//    causeForTerm                     0x8C         
//    diagnostics                      0xAD         
//    callReference                    0x8E         
//    sequenceNumber                   0x8F         
//    recordExtensions                 0xB0         
//    partialRecordType                0x97         
//    basicService                     0xBF8102     
//    additionalChgInfo                0xBF8105     
//    ussdCallBackFlag                 0x9F810A     
//    originalCalledNumber             0x9F810E     
//    chargeAreaCode                   0x9F8111     
//    rateIndication                   0x9F811F     
//    roamingNumber                    0x9F8120     
//    mscOutgoingCircuit               0x9F8126     
//    mscIncomingCircuit               0x9F8127     
//    orgMSCId                         0x9F8128     
//    callEmlppPriority                0x9F812A     
//    eaSubscriberInfo                 0x9F812E     
//    selectedCIC                      0x9F812F     
//    callerportedflag                 0x9F8134     
//    subscriberCategory               0x9F813E     
//    cUGOutgoingAccessIndicator       0x9F8143     
//    cUGInterlockCode                 0x9F8144     
//    cUGIncomingAccessUsed            0x9F8145     
//    mscIncomingRouteAttribute        0x9F8146     
//    mscOutgoingRouteAttribute        0x9F8147     
//    networkCallReference             0x9F8148     
//    setupTime                        0x9F8149     
//    alertingTime                     0x9F814A     
//    voiceIndicator                   0x9F814B     
//    bCategory                        0x9F814C     
//    callType                         0x9F814D     
//    chargePulseNum                   0x9F8150     
//    disconnectparty                  0x9F815A     
//    chargePulseNumforITXTXA          0x9F815B     
//    networkOperatorId                0x9F815F     
//    audioDataType                    0x9F8161     
//    recordNumber                     0x9F8168     
//    partyRelCause                    0xBF816C     
//    chargeLevel                      0x9F816D     
//    locationNum                      0x9F816E     
//    locationNumberNai                0x9F8171     
//    translatedNumber                 0x9F8175     
//    cmnFlag                          0x9F817B     
//    icidvalue                        0x9F817C     
//    origioi                          0x9F817D     
//    termioi                          0x9F817E     
//    calledportedflag                 0x9F817F     
//    locationroutingnumber            0x9F8200     
//    intermediateChargingInd          0x9F8202     
//    mscOutgoingROUTENumber           0x9F8205     
//    mscIncomingROUTENumber           0x9F8206     
//    drcCallId                        0x9F820A     
//    drcCallRN                        0x9F820B     
//    wpsCallFlag                      0x9F820F     
//    redirectingNumber                0x9F8210     
//    redirectingCounter               0x9F8211     
//    officeName                       0x9F821D     
// **********************************************************
//
/*!
 * 构造函数
 */
HwA1TransitBill::HwA1TransitBill():HwA1Bill("华为ASN.1汇接话单")
{
    /*
     * 创建每个字段对应的值和格式的对象 : hwa1_NodeList
     * 创建每个Tag和字段的对应 : hwa1_TagMap
     */
    int loc_Idx = 0;
    hwa1_NodeList.push_back(HwA1Node("recordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x80] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("recordingEntity", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x81] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTE", new DynLenValue(), &hwa1_PStrFmt));   hwa1_TagMap[0xA2] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTE", new DynLenValue(), &hwa1_PStrFmt));   hwa1_TagMap[0xA3] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callingNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x84] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("calledNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x85] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("isdnBasicService", new DynLenValue(), &hwa1_PStrFmt));   hwa1_TagMap[0x86] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("seizureTime", new DynLenValue(), &hwa1_TimeFmt));   hwa1_TagMap[0x87] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("answerTime", new DynLenValue(), &hwa1_TimeFmt));   hwa1_TagMap[0x88] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("releaseTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x89] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callDuration", new DynLenValue(), &hwa1_UIntLEFmt));  hwa1_TagMap[0x8A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("causeForTerm", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x8C] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("diagnostics", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xAD] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x8E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("sequenceNumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x8F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("recordExtensions", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xB0] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("partialRecordType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x97] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("basicService", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF8102] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("additionalChgInfo", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF8105] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("ussdCallBackFlag", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0x9F810A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("originalCalledNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x9F810E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("chargeAreaCode", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0x9F8111] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("rateIndication", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0x9F811F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("roamingNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x9F8120] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingCircuit", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8126] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscIncomingCircuit", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8127] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("orgMSCId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8128] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callEmlppPriority", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("eaSubscriberInfo", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("selectedCIC", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callerportedflag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8134] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("subscriberCategory", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F813E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessIndicator", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8143] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("cUGInterlockCode", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8144] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("cUGIncomingAccessUsed", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8145] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscIncomingRouteAttribute", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8146] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingRouteAttribute", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8147] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("networkCallReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8148] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("setupTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F8149] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("alertingTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F814A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("voiceIndicator", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814B] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("bCategory", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814C] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("callType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814D] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("chargePulseNum", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8150] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("disconnectparty", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("chargePulseNumforITXTXA", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815B] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("networkOperatorId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F815F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("audioDataType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8161] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("recordNumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8168] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("partyRelCause", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF816C] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("chargeLevel", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F816D] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("locationNum", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F816E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("locationNumberNai", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8171] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("translatedNumber", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x9F8175] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("cmnFlag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F817B] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("icidvalue", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F817C] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("origioi", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F817D] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("termioi", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F817E] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("calledportedflag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F817F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("locationroutingnumber", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8200] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("intermediateChargingInd", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8202] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTENumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8205] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTENumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8206] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("drcCallId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F820A] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("drcCallRN", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F820B] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("wpsCallFlag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F820F] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("redirectingNumber", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8210] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("redirectingCounter", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8211] = loc_Idx++;     
    hwa1_NodeList.push_back(HwA1Node("officeName", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F821D] = loc_Idx++;     
}


// ----------------------------------- HwA1CfwBill -----------------------------------
//
//                  华为ASN.1 前转话单格式说明
//
// **********************************************************
//    名称                                Tag
// **********************************************************
//    recordType                          0x80         
//    servedIMSI                          0x81         
//    servedIMEI                          0x82         
//    servedMSISDN                        0x83         
//    callingNumber                       0x84         
//    calledNumber                        0x85         
//    translatedNumber                    0x86         
//    connectedNumber                     0x87         
//    roamingNumber                       0x88         
//    recordingEntity                     0x89         
//    mscIncomingROUTE                    0xAA         
//    mscOutgoingROUTE                    0xAB         
//    location                            0xAC         
//    changeOfLocation                    0xAD         
//    basicService                        0xAE         
//    transparencyIndicator               0x8F         
//    changeOfService                     0xB0         
//    supplServicesUsed                   0xB1         
//    aocParameters                       0xB2         
//    changeOfAOCParms                    0xB3         
//    msClassmark                         0x94         
//    changeOfClassmark                   0xB5         
//    seizureTime                         0x96         
//    answerTime                          0x97         
//    releaseTime                         0x98         
//    callDuration                        0x99         
//    radioChanRequested                  0x9B         
//    radioChanUsed                       0x9C         
//    changeOfRadioChan                   0xBD         
//    causeForTerm                        0x9E         
//    diagnostics                         0xBF1F       
//    callReference                       0x9F20       
//    sequenceNumber                      0x9F21       
//    additionalChgInfo                   0xBF22       
//    recordExtensions                    0xBF23       
//    gsm_SCFAddress                      0x9F24       
//    serviceKey                          0x9F25       
//    networkCallReference                0x9F26       
//    mSCAddress                          0x9F27       
//    cAMELInitCFIndicator                0x9F28       
//    defaultCallHandling                 0x9F29       
//    fnur                                0x9F2D       
//    aiurRequested                       0x9F2E       
//    speechVersionSupported              0x9F31       
//    speechVersionUsed                   0x9F32       
//    numberOfDPEncountered               0x9F33       
//    levelOfCAMELService                 0x9F34       
//    freeFormatData                      0x9F35       
//    cAMELCallLegInformation             0xBF36       
//    freeFormatDataAppend                0x9F37       
//    defaultCallHandling_2               0x9F38       
//    gsm_SCFAddress_2                    0x9F39       
//    serviceKey_2                        0x9F3A       
//    freeFormatData_2                    0x9F3B       
//    freeFormatDataAppend_2              0x9F3C       
//    systemType                          0x9F3D       
//    rateIndication                      0x9F3E       
//    partialRecordType                   0x9F45       
//    guaranteedBitRate                   0x9F46       
//    maximumBitRate                      0x9F47       
//    modemType                           0x9F810B     
//    classmark3                          0x9F810C     
//    chargedParty                        0x9F810D     
//    originalCalledNumber                0x9F810E     
//    chargeAreaCode                      0x9F8111     
//    calledChargeAreaCode                0x9F8112     
//    mscOutgoingCircuit                  0x9F8126     
//    orgRNCorBSCId                       0x9F8127     
//    orgMSCId                            0x9F8128     
//    callEmlppPriority                   0x9F812A     
//    callerDefaultEmlppPriority          0x9F812B     
//    eaSubscriberInfo                    0x9F812E     
//    selectedCIC                         0x9F812F     
//    optimalRoutingFlag                  0x9F8131     
//    optimalRoutingLateForwardFlag       0x9F8132     
//    optimalRoutingEarlyForwardFlag      0x9F8133     
//    callerportedflag                    0x9F8134     
//    calledIMSI	                      0x9F8135     
//    globalAreaID                        0x9F813C     
//    changeOfglobalAreaID                0xBF813D     
//    subscriberCategory                  0x9F813E     
//    firstmccmnc                         0x9F8140     
//    intermediatemccmnc                  0x9F8141     
//    lastmccmnc                          0x9F8142     
//    cUGOutgoingAccessIndicator          0x9F8143     
//    cUGInterlockCode                    0x9F8144     
//    cUGOutgoingAccessUsed               0x9F8145     
//    cUGIndex                            0x9F8146     
//    interactionWithIP                   0x9F8147     
//    hotBillingTag                       0x9F8148     
//    setupTime                           0x9F8149     
//    alertingTime                        0x9F814A     
//    voiceIndicator                      0x9F814B     
//    bCategory                           0x9F814C     
//    callType                            0x9F814D     
//    cAMELDestinationNumber              0x9F814F     
//    groupCallType                       0x9F8150     
//    groupCallReference                  0x9F8151     
//    uus1Type                            0x9F8152     
//    eCategory                           0x9F8157     
//    tariffCode                          0x9F8159     
//    disconnectparty                     0x9F815A     
//    chargePulseNum                      0x9F815B     
//    csReference                         0x9F815C     
//    csaReference                        0x9F815D     
//    camelphase                          0x9F815E     
//    networkOperatorId                   0x9F815F     
//    typeOfSubscribers                   0x9F8160     
//    audioDataType                       0x9F8161     
//    userType                            0x9F8163     
//    recordNumber                        0x9F8168     
//    partyRelCause                       0xBF816C     
//    chargeLevel                         0x9F816D     
//    locationNum                         0x9F816E     
//    zoneCode                            0x9F8170     
//    locationNumberNai                   0x9F8171     
//    dtmf_indicator                      0x9F8172     
//    b_ch_number                         0x9F8173     
//    cARP                                0x9F8176     
//    channelmode                         0x9F8179     
//    channel                             0x9F817A     
//    specialBillPrefix                   0x9F817C     
//    calledportedflag                    0x9F817F     
//    locationroutingnumber               0x9F8200     
//    routingcategory                     0x9F8201     
//    intermediateChargingInd             0x9F8202     
//    calledIMEI                          0x9F8204     
//    mscOutgoingROUTENumber              0x9F8205     
//    mscIncomingROUTENumber              0x9F8206     
//    roDefaultCallHandling               0x9F8207     
//    roLinkFailureTime                   0x9F8208     
//    lastSuccCCRTime                     0x9F8209     
//    drcCallId                           0x9F820A     
//    drcCallRN                           0x9F820B     
//    wpsCallFlag                         0x9F820F     
//    redirectingCounter                  0x9F8211     
//    voBBUserFlag                        0x9F8215     
//    chargePulses                        0x9F8216     
//    inapFciBillingInfo                  0xBF8217     
//    followMeInd                         0x9F8219     
//    invokeOfLCLS                        0xBF821A     
//    officeName                          0x9F821D     
// **********************************************************
//
/*!
 * 构造函数
 */
HwA1CfwBill::HwA1CfwBill():HwA1Bill("华为ASN.1前转话单")
{
    /*
     * 创建每个字段对应的值和格式的对象 : hwa1_NodeList
     * 创建每个Tag和字段的对应 : hwa1_TagMap
     */
    int loc_Idx = 0;
    hwa1_NodeList.push_back(HwA1Node("recordType", new DynLenValue(), &hwa1_UIntBEFmt)); hwa1_TagMap[0x80] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMSI", new DynLenValue(), &hwa1_IsdnFmt));   hwa1_TagMap[0x81] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedIMEI", new DynLenValue(), &hwa1_IsdnFmt));   hwa1_TagMap[0x82] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("servedMSISDN", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x83] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callingNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x84] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x85] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("translatedNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x86] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("connectedNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x87] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("roamingNumber", new DynLenValue(), &hwa1_NumberFmt));   hwa1_TagMap[0x88] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordingEntity", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x89] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTE", new DynLenValue(), &hwa1_PStrFmt));  hwa1_TagMap[0xAA] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTE", new DynLenValue(), &hwa1_PStrFmt));  hwa1_TagMap[0xAB] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("location", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xAC] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfLocation", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xAD] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("basicService", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xAE] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("transparencyIndicator", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x8F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfService", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xB0] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("supplServicesUsed", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xB1] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aocParameters", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xB2] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfAOCParms", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0xB3] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("msClassmark", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0x94] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfClassmark", new DynLenValue(), &hwa1_DebugFmt));   hwa1_TagMap[0xB5] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("seizureTime", new DynLenValue(), &hwa1_TimeFmt));   hwa1_TagMap[0x96] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("answerTime", new DynLenValue(), &hwa1_TimeFmt));   hwa1_TagMap[0x97] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("releaseTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x98] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callDuration", new DynLenValue(), &hwa1_UIntLEFmt));  hwa1_TagMap[0x99] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanRequested", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("radioChanUsed", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfRadioChan", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBD] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("causeForTerm", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("diagnostics", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF1F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F20] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("sequenceNumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F21] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("additionalChgInfo", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF22] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordExtensions", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF23] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("gsm-SCFAddress", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x9F24] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("serviceKey", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F25] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkCallReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F26] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mSCAddress", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x9F27] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cAMELInitCFIndicator", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F28] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("defaultCallHandling", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F29] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("fnur", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F2D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("aiurRequested", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F2E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionSupported", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F31] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("speechVersionUsed", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F32] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("numberOfDPEncountered", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F33] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("levelOfCAMELService", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F34] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatData", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F35] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cAMELCallLegInformation", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF36] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatDataAppend", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F37] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("defaultCallHandling-2", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F38] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("gsm-SCFAddress-2", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x9F39] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("serviceKey-2", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F3A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatData-2", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F3B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("freeFormatDataAppend-2", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F3C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("systemType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F3D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("rateIndication", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F3E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("partialRecordType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F45] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("guaranteedBitRate", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F46] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("maximumBitRate", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F47] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("modemType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F810B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("classmark3", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F810C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargedParty", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F810D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("originalCalledNumber", new DynLenValue(), &hwa1_NumberFmt));  hwa1_TagMap[0x9F810E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargeAreaCode", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8111] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledChargeAreaCode", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8112] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingCircuit", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8126] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgRNCorBSCId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8127] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("orgMSCId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8128] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callEmlppPriority", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callerDefaultEmlppPriority", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eaSubscriberInfo", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("selectedCIC", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F812F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingFlag", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8131] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingLateForwardFlag", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8132] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("optimalRoutingEarlyForwardFlag", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8133] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callerportedflag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8134] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledIMSI	", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8135] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("globalAreaID", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F813C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("changeOfglobalAreaID", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF813D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("subscriberCategory", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F813E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("firstmccmnc", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8140] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("intermediatemccmnc", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8141] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("lastmccmnc", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8142] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessIndicator", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8143] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGInterlockCode", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8144] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGOutgoingAccessUsed", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8145] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cUGIndex", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8146] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("interactionWithIP", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8147] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("hotBillingTag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8148] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("setupTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F8149] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("alertingTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F814A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("voiceIndicator", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("bCategory", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("callType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F814D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cAMELDestinationNumber", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F814F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8150] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("groupCallReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8151] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("uus1Type", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8152] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("eCategory", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8157] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("tariffCode", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8159] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("disconnectparty", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargePulseNum", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csReference", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F815C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("csaReference", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("camelphase", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F815E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("networkOperatorId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F815F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("typeOfSubscribers", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8160] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("audioDataType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8161] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("userType", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8163] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("recordNumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8168] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("partyRelCause", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF816C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargeLevel", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F816D] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("locationNum", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F816E] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("zoneCode", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8170] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("locationNumberNai", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8171] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("dtmf-indicator", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8172] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("b-ch-number", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8173] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("cARP", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8176] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("channelmode", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8179] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("channel", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F817A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("specialBillPrefix", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F817C] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledportedflag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F817F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("locationroutingnumber", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8200] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("routingcategory", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8201] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("intermediateChargingInd", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8202] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("calledIMEI", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8204] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscOutgoingROUTENumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8205] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("mscIncomingROUTENumber", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8206] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("roDefaultCallHandling", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8207] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("roLinkFailureTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F8208] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("lastSuccCCRTime", new DynLenValue(), &hwa1_TimeFmt));  hwa1_TagMap[0x9F8209] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("drcCallId", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F820A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("drcCallRN", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F820B] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("wpsCallFlag", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F820F] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("redirectingCounter", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8211] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("voBBUserFlag", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F8215] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("chargePulses", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8216] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("inapFciBillingInfo", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF8217] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("followMeInd", new DynLenValue(), &hwa1_UIntBEFmt));  hwa1_TagMap[0x9F8219] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("invokeOfLCLS", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0xBF821A] = loc_Idx++;
    hwa1_NodeList.push_back(HwA1Node("officeName", new DynLenValue(), &hwa1_DebugFmt));  hwa1_TagMap[0x9F821D] = loc_Idx++;
}


