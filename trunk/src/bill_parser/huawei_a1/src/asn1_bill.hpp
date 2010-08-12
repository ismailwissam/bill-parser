#ifndef HUAWEI_ASN1_BILL_HPP
#define HUAWEI_ASN1_BILL_HPP

#include "bill_base.hpp"
#include "value.hpp"
#include "format.hpp"

//
// ��Ϊ�����ڵ�
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
 * ��Ϊ ASN.1 ����
 */
class HwA1Bill : public BillBase
{
    protected:
        //
        // �����ʽ
        //
        BCDFormat  hwa1_DebugFmt;       // Debug �õĸ�ʽ
        UIntFormat hwa1_UIntBEFmt;      // �޷���������Big-Endian�����ʽ
        UIntFormat hwa1_UIntLEFmt;      // �޷���������Little-Endian�����ʽ
        BCDFormat  hwa1_BCDZipBEFmt;    // BCD ѹ����ʽ��λ�ں��ʽ
        HwA1IsdnFmt hwa1_IsdnFmt;	    // ��ͨ�����ʽ
        HwA1NumberFmt hwa1_NumberFmt;   // Asn.1�����ʽ
        HwA1PackedStrFmt hwa1_PStrFmt;  // Asn.1�����ַ���
        HwA1LacCiFmt hwa1_LacCiFmt;     // lac-ci ��ʽ
        HwA1TimeFmt hwa1_TimeFmt;       // ��������

        HW_A1_NODE_LIST hwa1_NodeList;  // ��Ϊ�ڵ��б�
        HW_A1_TAG_MAP   hwa1_TagMap;    // Tag -> �ڵ� ��ӳ���

    public:
        /*!
         * ���캯��
         */
        HwA1Bill(const char* name);

        /*!
         * ��������
         */
        virtual ~HwA1Bill();

        /*!
         * �����ֶ�����
         */
        virtual const char* billbaseItemName(int idx);

        /*!
         * ��ȡ��������
         * ������Ľ�����뽫��Ч���ֶα����� billbase_NodeMap ��
         */
        virtual bool billbaseReadData(std::istream& in);
};


//
// ��Ϊ ASN.1 ���л���
//
class HwA1MocBill : public HwA1Bill
{
    public:
        /*!
         * ���캯��
         */
        HwA1MocBill();
};

//
// ��Ϊ ASN.1 ���л���
//
class HwA1MtcBill : public HwA1Bill
{
    public:
        /*!
         * ���캯��
         */
        HwA1MtcBill();
};

//
// ��Ϊ ASN.1 ��ӻ��� 
//
class HwA1TransitBill : public HwA1Bill
{
    public:
        /*
         * ���캯��
         */
        HwA1TransitBill();
};

//
// ��Ϊ ASN.1 ǰת���� 
//
class HwA1CfwBill : public HwA1Bill
{
    public:
        /*
         * ���캯��
         */
        HwA1CfwBill();
};


#endif // HUAWEI_ASN1_MOC_BILL_HPP

