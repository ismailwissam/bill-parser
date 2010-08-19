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
     * 华为话单对象
     */
    HwA1MocBill     hwmod_A1MocBill;         // Asn1 主叫话单 idx = HW_BT_A1_MOC
    HwA1MtcBill     hwmod_A1MtcBill;         // Asn1 被叫话单 idx = HW_BT_A1_MTC
    HwA1CfwBill     hwmod_A1CfwBill;         // Asn1 前转话单 idx = HW_BT_A1_CFW
    HwA1TransitBill hwmod_A1TransitBill;     // Asn1 汇接话单 idx = HW_BT_A1_TRAN

    /*!
     * 华为话单文件格式
     */
    int hwmod_BillType;
    int hwmod_iGWBType;

public:
    /*!
     * 构造函数
     */
    HuaweiModule();

    /*!
     * 克隆自身
     */
    virtual BillModule* billmodClone() const;

    /*!
     * 文件解析处理，增加对ASN.1的兼容
     */
    virtual bool billmodParseFile(const char* file_name);

    /*!
     * 读取下一条话单
     */
    virtual bool billmodFetchNextBill();

private:
    /*!
     * 按照ASN1格式处理话单
     *
     * 返回值：华为话单格式（参见 : enum_HW_BILL_TYPE）
     */
    int hwbillAnalyseType();
};

#endif // HUAWEI_MODULE_HPP

