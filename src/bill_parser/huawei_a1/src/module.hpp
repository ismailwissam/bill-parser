#ifndef HUAWEI_MODULE_HPP
#define HUAWEI_MODULE_HPP

#include "module_base.hpp"
#include "asn1_bill.hpp"

class HuaweiModule : public BillModule
{
private:
    enum
    {
        HW_BT_A1_MOC = 0,
        HW_BT_A1_MTC,
        HW_BT_A1_CFW,
        HW_BT_A1_TRAN
    };

    /*!
     * ��Ϊ��������
     */
    HwA1MocBill     hwmod_A1MocBill;         // Asn1 ���л��� idx = HW_BT_A1_MOC
    HwA1MtcBill     hwmod_A1MtcBill;         // Asn1 ���л��� idx = HW_BT_A1_MTC
    HwA1CfwBill     hwmod_A1CfwBill;         // Asn1 ǰת���� idx = HW_BT_A1_CFW
    HwA1TransitBill hwmod_A1TransitBill;     // Asn1 ��ӻ��� idx = HW_BT_A1_TRAN

    /*!
     * ��Ϊ�����ļ���ʽ
     */
    int hwmod_BillType;
    int hwmod_iGWBType;

public:
    /*!
     * ���캯��
     */
    HuaweiModule();

    /*!
     * ��¡����
     */
    virtual BillModule* billmodClone() const;

    /*!
     * �ļ������������Ӷ�ASN.1�ļ���
     */
    virtual bool billmodParseFile(const char* file_name);

    /*!
     * ��ȡ��һ������
     */
    virtual bool billmodFetchNextBill();

private:
    /*!
     * ����ASN1��ʽ������
     *
     * ����ֵ����Ϊ������ʽ���μ� : enum_HW_BILL_TYPE��
     */
    int hwbillAnalyseType();
};

#endif // HUAWEI_MODULE_HPP

