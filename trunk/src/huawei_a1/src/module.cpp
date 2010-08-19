#include <memory>
#include "tlog.hpp"
#include "module.hpp"
#include "hw_asn1.hpp"

//
// ��Ϊ������ʽ
//
enum /* enum_HW_BILLFILE_TYPE */
{
    HW_BFT_INVALID,  // ��Ч�Ļ���
    HW_BFT_ASN1      // ASN1 ��ʽ
};

enum
{
    HW_iGWB_INVALID,
    HW_iGWB_1ST         // ��һ�ݻ�����ʽ
};


//
// ���廰��ģ������ָ��
//
std::auto_ptr<BillModule> glb_BillModulePtr(new HuaweiModule);

//
// ���캯��
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
// ��¡����
//
BillModule* HuaweiModule::billmodClone() const
{
    return new HuaweiModule;
}

//
// �ļ������������Ӷ�ASN.1�ļ���
//
bool HuaweiModule::billmodParseFile(const char* file_name)
{
    if(!BillModule::billmodParseFile(file_name))
    {
        return false;
    }

    //
    // �жϻ�����ʽ
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
// ��ȡ��һ������
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
        // ASN1 ��ʽ
        //
        
        // �����iGWB��ʽ������ǰ��4���ֽ�
        if(hwmod_iGWBType == HW_iGWB_1ST)
        {
            billmodStream().seekg(4, std::ios::cur);
        }

        // ��ȡ Tag
        int loc_Tag;
        if(!hwasn1GetTag(billmodStream(), loc_Tag))
        {
            return false;
        }

        // ��ȡ ����
        size_t loc_Len;
        if(!hwasn1GetLen(billmodStream(), loc_Len))
        {
            return false;
        }

        if(loc_Tag == 0xA0 || loc_Tag == 0xA1 || loc_Tag == 0xA5 || loc_Tag == 0xBF64)
        {
            // ��ȡ���ݣ����� istringstream
            std::string loc_Data;
            if(!hwasn1GetCont(billmodStream(), loc_Len, loc_Data))
            {
                return false;
            }

            std::istringstream loc_IStr(loc_Data);

            if(loc_Tag == 0xA0)         // ���л���
            {
                billmodCurBillType(HW_BT_A1_MOC);
                return hwmod_A1MocBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xA1)    // ���л���
            {
                billmodCurBillType(HW_BT_A1_MTC);
                return hwmod_A1MtcBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xA5)    // ��ӻ���
            {
                billmodCurBillType(HW_BT_A1_TRAN);
                return hwmod_A1TransitBill.billbaseReadData(loc_IStr);
            }
            else if(loc_Tag == 0xBF64)  // ǰת����
            {
                billmodCurBillType(HW_BT_A1_CFW);
                return hwmod_A1CfwBill.billbaseReadData(loc_IStr);
            }
        }
        else
        {
            //
            // ����������л��������л�����ǰת��������ӻ�����Ŀǰ��������
            //
            billmodStream().seekg(loc_Len, std::ios::cur);

            billmodCurBillType(INVAILD_BILL_TYPE);
            return true;
        }
    }

    return false;
}

//
// ����ASN1��ʽ������
//
// ����ֵ����Ϊ������ʽ���μ� : enum_HW_BILL_TYPE��
//
int HuaweiModule::hwbillAnalyseType()
{
    std::istream& loc_FStream = billmodStream();

    //
    // ��¼��ǰ�ļ�ָ��λ��
    //
    size_t loc_OldPos = loc_FStream.tellg();

    //
    // ����Ƿ��� iGWB ��ʽ����
    //
    // ������������ͷ 33 00 01 00 01 00 00 00
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
    // ����� iGWB ��ʽ�����ж������Ƿ���ASN.1
    //
    if(hwmod_iGWBType == HW_iGWB_1ST)
    {
        // ��ȡ��һ����������(����������Tag��Length��ռ�ֽڳ��ȣ�
        StaticLenValue<4> loc_BillLenObj;
        loc_FStream >> loc_BillLenObj;
        unsigned long long loc_BillLen = 0;
        if(!UIntFormat::uintfmtReadValue(true, loc_BillLenObj.valRawData(), loc_BillLenObj.valRawDataLen(), loc_BillLen))
        {
            LOG_STR_LN("��ȡ��������ʧ�ܣ�");
            return HW_BFT_INVALID;
        }

        // ��¼�ļ�ָ��λ��
        size_t loc_Pos1 = loc_FStream.tellg();

        // ��ȡ��һ��������Tag
        int loc_Asn1Tag;
        if(!hwasn1GetTag(loc_FStream, loc_Asn1Tag))
        {
            LOG_STR_LN("��ȡ����Tagʧ�ܣ�");
            return HW_BFT_INVALID;
        }

        // ��ȡ��һ��������Length
        size_t loc_Asn1Len; 
        if(!hwasn1GetLen(loc_FStream, loc_Asn1Len))
        {
            LOG_STR_LN("��ȡ����Lengthʧ�ܣ�");
            return HW_BFT_INVALID;
        }

        // ��¼�ļ�ָ��λ��
        size_t loc_Pos2 = loc_FStream.tellg();

        if(loc_BillLen - loc_Asn1Len == loc_Pos2 - loc_Pos1)
        {
            // ���ļ�ָ�����õ���һ��������ʼλ��
            loc_FStream.seekg(loc_OldPos+32, std::ios::beg);
            return HW_BFT_ASN1;
        }
    }
    else
    {
        // �ָ��ļ�ָ��λ��
        loc_FStream.seekg(loc_OldPos, std::ios::beg);

        //
        // �ж��Ƿ���� ASN1 �ļ���ʽ
        //
        int loc_Tag = 0;
        size_t loc_Len = 0;
        bool loc_Ok = false;
        do
        {
            /* ----------------------------------------------------
                ������½ṹ����������㣬����Ϊ����ASN.1
                30(xxx) : �ļ�����  *
                {
                    A0(10) : �ļ�ͷ   *
                    {
                        80(9) : ??    *
                        81(1) : ??
                    }
                    A2(0) : ͷ����
                    A1(0x11E) : ����
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

            // ȫ��Tag == 0x30
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0x30)
            {
                LOG_STR_LN("ȫ��Tag != 0x30");
                break;
            }

            // ȫ�ĳ���
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("ȫ�ĳ��ȶ�ȡʧ��");
                break;
            }

            // ͷTag == 0xA0
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0xA0)
            {
                LOG_STR_LN("ͷTag != 0xA0");
                break;
            }

            // ͷ����
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("ͷ���ȶ�ȡʧ��");
                break;
            }

            // ͷ������
            loc_FStream.seekg(loc_Len, std::ios::cur);

            // ����Tag == 0xA1
            if(!hwasn1GetTag(loc_FStream, loc_Tag) || loc_Tag != 0xA1)
            {
                LOG_STR_LN("����Tag != 0xA1");
                break;
            }

            // ����Len 
            if(!hwasn1GetLen(loc_FStream, loc_Len))
            {
                LOG_STR_LN("���ĳ��ȶ�ȡʧ��");
                break;
            }

            loc_Ok = true;
        } while(0);

        // ASN.1 ��ʽ
        if(loc_Ok == true)
        {
            return HW_BFT_ASN1;
        }
    }
    
    //
    // ����ASN1��ʽ���ָ��ļ�ָ��λ��
    //
    loc_FStream.seekg(loc_OldPos, std::ios::beg);

    return HW_BFT_INVALID;
}

