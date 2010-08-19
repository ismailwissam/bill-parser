#include <memory>
#include "tlog.hpp"
#include "module.hpp"
#include "hw_asn1.hpp"

//
// 华为话单格式
//
enum /* enum_HW_BILLFILE_TYPE */
{
    HW_BFT_INVALID,  // 无效的话单
    HW_BFT_ASN1      // ASN1 格式
};

enum
{
    HW_iGWB_INVALID,
    HW_iGWB_1ST         // 第一份话单格式
};


//
// 定义话单模块智能指针
//
std::auto_ptr<BillModule> glb_BillModulePtr(new HuaweiModule);

//
// 构造函数
//
HuaweiModule::HuaweiModule()
{
    FUNC_TRACE_LOG;

    billmodAddBill(&hwmod_A1MocBill);
    billmodAddBill(&hwmod_A1MtcBill);
    billmodAddBill(&hwmod_A1CfwBill);
    billmodAddBill(&hwmod_A1TransitBill);

    hwmod_iGWBType = HW_iGWB_INVALID;
}

//
// 克隆自身
//
BillModule* HuaweiModule::billmodClone() const
{
    return new HuaweiModule;
}

//
// 文件解析处理，增加对ASN.1的兼容
//
bool HuaweiModule::billmodParseFile(const char* file_name)
{
    if(!BillModule::billmodParseFile(file_name))
    {
        return false;
    }

    //
    // 判断话单格式
    //
    int loc_Type = hwbillAnalyseType();

    if(loc_Type == HW_BFT_INVALID)
    {
        return false;
    }

    hwmod_BillType = loc_Type;
    return true;
}

//
// 读取下一条话单
//
bool HuaweiModule::billmodFetchNextBill()
{
    if(billmodStream().bad() || billmodStream().eof())
    {
        return false;
    }

    if(hwmod_BillType == HW_BFT_ASN1)
    {
        //
        // ASN1 格式
        //
        
        // 如果是iGWB格式，跳过前面4个字节
        if(hwmod_iGWBType == HW_iGWB_1ST)
        {
            billmodStream().seekg(4, std::ios::cur);
        }

        // 读取 Tag
        int loc_Tag;
        if(!hwasn1GetTag(billmodStream(), loc_Tag))
        {
            return false;
        }

        // 读取 长度
        size_t loc_Len;
        if(!hwasn1GetLen(billmodStream(), loc_Len))
        {
            return false;
        }

        if(loc_Tag == 0xA0 || loc_Tag == 0xA1 || loc_Tag == 0xA5 || loc_Tag == 0xBF64)
        {
            // 读取内容，生成 istringstream
            std::string loc_Data;
            if(!hwasn1GetCont(billmodStream(), loc_Len, loc_Data))
            {
                return false;
            }

            std::istringstream loc_IStr(loc_Data);

            if(loc_Tag == 0xA0)         // 主叫话单
            {
                billmodCurBillType(HW_BT_A1_MOC);
                return hwmod_A1MocBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xA1)    // 被叫话单
            {
                billmodCurBillType(HW_BT_A1_MTC);
                return hwmod_A1MtcBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xA5)    // 汇接话单
            {
                billmodCurBillType(HW_BT_A1_TRAN);
                return hwmod_A1TransitBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xBF64)  // 前转话单
            {
                billmodCurBillType(HW_BT_A1_CFW);
                return hwmod_A1CfwBill.billbaseReadData(loc_IStr);
            }
        }
        else
        {
            //
            // 如果不是主叫话单、被叫话单、前转话单、汇接话单，目前不做处理
            //
            billmodStream().seekg(loc_Len, std::ios::cur);

            billmodCurBillType(INVAILD_BILL_TYPE);
            return true;
        }
    }

    return false;
}

//
// 按照ASN1格式处理话单
//
// 返回值：华为话单格式（参见 : enum_HW_BILL_TYPE）
//
int HuaweiModule::hwbillAnalyseType()
{
    std::istream& loc_FStream = billmodStream();

    //
    // 记录当前文件指针位置
    //
    size_t loc_OldPos = loc_FStream.tellg();

    //
    // 检查是否是 iGWB 格式话单
    //
    // 满足条件，开头 33 00 01 00 01 00 00 00
    //
    unsigned char loc_iGWBHead[32];
    loc_FStream.read((char*)loc_iGWBHead, 32);

    if(loc_iGWBHead[0] == 0x33 && loc_iGWBHead[1] == 0x00
            && loc_iGWBHead[2] == 0x01 && loc_iGWBHead[3] == 0x00
            && loc_iGWBHead[4] == 0x01 && loc_iGWBHead[5] == 0x00
            && loc_iGWBHead[6] == 0x00 && loc_iGWBHead[7] == 0x00)
    {
        hwmod_iGWBType = HW_iGWB_1ST;
    }

    //
    // 如果是 iGWB 格式，则判断内容是否是ASN.1
    //
    if(hwmod_iGWBType == HW_iGWB_1ST)
    {
        // 读取第一个话单长度(包含话单的Tag和Length所占字节长度）
        StaticLenValue<4> loc_BillLenObj;
        loc_FStream >> loc_BillLenObj;
        unsigned long long loc_BillLen = 0;
        if(!UIntFormat::uintfmtReadValue(true, loc_BillLenObj.valRawData(), loc_BillLenObj.valRawDataLen(), loc_BillLen))
        {
            LOG_STR_LN("读取话单长度失败！");
            return HW_BFT_INVALID;
        }

        // 记录文件指针位置
        size_t loc_Pos1 = loc_FStream.tellg();

        // 读取第一个话单的Tag
        int loc_Asn1Tag;
        if(!hwasn1GetTag(loc_FStream, loc_Asn1Tag))
        {
            LOG_STR_LN("读取话单Tag失败！");
            return HW_BFT_INVALID;
        }

        // 读取第一个话单的Length
        size_t loc_Asn1Len; 
        if(!hwasn1GetLen(loc_FStream, loc_Asn1Len))
        {
            LOG_STR_LN("读取话单Length失败！");
            return HW_BFT_INVALID;
        }

        // 记录文件指针位置
        size_t loc_Pos2 = loc_FStream.tellg();

        if(loc_BillLen - loc_Asn1Len == loc_Pos2 - loc_Pos1)
        {
            // 将文件指针设置到第一个话单起始位置
            loc_FStream.seekg(loc_OldPos+32, std::ios::beg);
            return HW_BFT_ASN1;
        }
    }
    else
    {
        // 恢复文件指针位置
        loc_FStream.seekg(loc_OldPos, std::ios::beg);

        //
        // 判断是否符合 ASN1 文件格式
        //
        int loc_Tag = 0;
        size_t loc_Len = 0;
        bool loc_Ok = false;
        do
        {
            /* ----------------------------------------------------
                检查如下结构，如果不满足，则认为不是ASN.1
                30(xxx) : 文件类型  *
                {
                    A0(10) : 文件头   *
                    {
                        80(9) : ??    *
                        81(1) : ??
                    }
                    A2(0) : 头结束
                    A1(0x11E) : 正文
                    {
                        A0(0x11A) : 
                        {
                            80(1) : 0
                            81(8) : 460007000102000
                            83(8) : 198613720012000
                            84(8) : 198613720012000
                            85(7) : 1A13720012100
                            ...
                        }
                        ...
                    }
                }
             ---------------------------------------------------- */

            // 全文Tag == 0x30
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0x30)
            {
                LOG_STR_LN("全文Tag != 0x30");
                break;
            }

            // 全文长度
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("全文长度读取失败");
                break;
            }

            // 头Tag == 0xA0
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0xA0)
            {
                LOG_STR_LN("头Tag != 0xA0");
                break;
            }

            // 头长度
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("头长度读取失败");
                break;
            }

            // 头的内容
            loc_FStream.seekg(loc_Len, std::ios::cur);

            // 正文Tag == 0xA1
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0xA1)
            {
                LOG_STR_LN("正文Tag != 0xA1");
                break;
            }

            // 正文Len 
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("正文长度读取失败");
                break;
            }

            loc_Ok = true;
        } while(0);

        // ASN.1 格式
        if(loc_Ok == true)
        {
            return HW_BFT_ASN1;
        }
    }
    
    //
    // 不是ASN1格式，恢复文件指针位置
    //
    loc_FStream.seekg(loc_OldPos, std::ios::beg);

    return HW_BFT_INVALID;
}

