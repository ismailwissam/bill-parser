#ifndef HUAWEI_ASN1_BILL_HPP
#define HUAWEI_ASN1_BILL_HPP

#include "bill_base.hpp"
#include "value.hpp"
#include "format.hpp"

//
// 华为话单节点
//
class HwA1Node
{
    public:
        std::string name;
        DynLenValue* value;
        FormatBase* format;

        HwA1Node(const char* n, DynLenValue* v, FormatBase* f)
            : name(n), value(v), format(f)
        {
        }
};

typedef std::vector<HwA1Node> HW_A1_NODE_LIST;
typedef std::map<int, int> HW_A1_TAG_MAP;

/*!
 * 华为 ASN.1 话单
 */
class HwA1Bill : public BillBase
{
    protected:
        //
        // 输出格式
        //
        BCDFormat  hwa1_DebugFmt;       // Debug 用的格式
        UIntFormat hwa1_UIntBEFmt;      // 无符号整数，Big-Endian编码格式
        UIntFormat hwa1_UIntLEFmt;      // 无符号整数，Little-Endian编码格式
        BCDFormat  hwa1_BCDZipBEFmt;    // BCD 压缩格式高位在后格式
        HwA1IsdnFmt hwa1_IsdnFmt;	    // 普通号码格式
        HwA1NumberFmt hwa1_NumberFmt;   // Asn.1号码格式
        HwA1PackedStrFmt hwa1_PStrFmt;  // Asn.1包的字符串
        HwA1LacCiFmt hwa1_LacCiFmt;     // lac-ci 格式
        HwA1TimeFmt hwa1_TimeFmt;       // 日期类型

        HW_A1_NODE_LIST hwa1_NodeList;  // 华为节点列表
        HW_A1_TAG_MAP   hwa1_TagMap;    // Tag -> 节点 的映射表

    public:
        /*!
         * 构造函数
         */
        HwA1Bill(const char* name);

        /*!
         * 析构函数
         */
        virtual ~HwA1Bill();

        /*!
         * 返回字段名称
         */
        virtual const char* billbaseItemName(int idx);

        /*!
         * 读取话单数据
         * 解析后的结果必须将有效的字段保存在 billbase_NodeMap 中
         */
        virtual bool billbaseReadData(std::istream& in);
};


//
// 华为 ASN.1 主叫话单
//
class HwA1MocBill : public HwA1Bill
{
    public:
        /*!
         * 构造函数
         */
        HwA1MocBill();
};

//
// 华为 ASN.1 被叫话单
//
class HwA1MtcBill : public HwA1Bill
{
    public:
        /*!
         * 构造函数
         */
        HwA1MtcBill();
};

//
// 华为 ASN.1 汇接话单 
//
class HwA1TransitBill : public HwA1Bill
{
    public:
        /*
         * 构造函数
         */
        HwA1TransitBill();
};

//
// 华为 ASN.1 前转话单 
//
class HwA1CfwBill : public HwA1Bill
{
    public:
        /*
         * 构造函数
         */
        HwA1CfwBill();
};


#endif // HUAWEI_ASN1_MOC_BILL_HPP

