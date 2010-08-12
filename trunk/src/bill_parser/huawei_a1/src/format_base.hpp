#ifndef FORMAT_BASE_HPP
#define FORMAT_BASE_HPP

#include <sstream>
#include "value_base.hpp"

/*!
 * ���ݸ�ʽ������
 */
class FormatBase
{
protected:
    /*!
     * ����ָ��
     */
    const char* fmt_Data;

    /*!
     * ���ݳ���
     */
    size_t fmt_Len;

public:
    /*!
     * ���캯��
     */
    FormatBase();

    /*!
     * �������ݶ���ָ��
     */
    void fmtSetValue(const char* data, size_t len);

    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 *  �޷������͸�ʽ
 */
class UIntFormat : public FormatBase
{
private:
    /*!
     * ���Ϊtrue�����Ǹ�λ��ǰ����λ�ں�
     * ���Ϊfalse�����ǵ�λ��ǰ����λ�ں�
     */
    bool uint_IsBigEnd;

    /*!
     * ������ֵ���еĻ�����
     */
    std::ostringstream uint_Stream;

public:
    /*!
     * ���캯��
     * @param value �������ָ��
     * @param is_bigend �Ƿ��Ǹ�λ��ǰ��Ĭ��Ϊ true
     */
    UIntFormat(bool is_bigend = true);
    
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();

    /*!
     * ���޷������͸�ʽ��ȡ����
     */
    static bool uintfmtReadValue(bool is_bigend, const char* data,
            size_t len, unsigned long long& val);
};

/*!
 *  �з������͸�ʽ
 */
class IntFormat : public FormatBase
{
private:
    /*!
     * ���Ϊtrue�����Ǹ�λ��ǰ����λ�ں�
     * ���Ϊfalse�����ǵ�λ��ǰ����λ�ں�
     */
    bool int_IsBigEnd;

    /*!
     * ������ֵ���еĻ�����
     */
    std::ostringstream int_Stream;

public:
    /*!
     * ���캯��
     * @param value �������ָ��
     * @param is_bigend �Ƿ��Ǹ�λ��ǰ��Ĭ��Ϊ true
     */
    IntFormat(bool is_bigend = true);
    
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();

    /*!
     * ���޷������͸�ʽ��ȡ����
     */
    static bool intfmtReadValue(bool is_bigend, const char* data,
            size_t len, long long& val);
};

/*!
 *  ʮ�����Ƹ�ʽ
 */
class BCDFormat : public FormatBase
{
private:
    /*!
     * ���Ϊtrue������ѹ����BCD��
     * ���Ϊfalse�����Ƿ�ѹ����BCD��
     */
    bool bcd_IsZip;

    /*!
     * ���Ϊtrue�����Ǹ�λ�ں��BCD��
     * ���Ϊfalse�����Ǹ�λ��ǰ��BCD��
     */
    bool bcd_IsBigEnd;

    /*!
     * ���Ϊtrue�����ַ���ֻ�������ݣ�������������0~9�����֣���ֹͣ
     * ���Ϊfalse�����ַ����԰���0~9A~F���κ��ַ�
     */
    bool bcd_AllNum;

    /*!
     * ��õ�BCD���ݴ�
     */
    std::string bcd_Value;

public:
    /*!
     * ���캯��
     */
    BCDFormat(bool is_zip = true, bool is_bigend = true, bool all_num = true);

    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();

    /*!
     * ��BCD��ʽ��ȡ����
     * @param is_zip    ԭʼ������ѹ����ʽ��BCD
     * @param is_bigend ԭʼ�����Ǹ�λ�ں��BCD
     * @param offset    ����ǰ���һ���������֣���������is_zip��־��Ӱ��
     * @param data      ԭʼ���ݵ��׵�ַ
     * @param len       ԭʼ���ݵĳ���
     * @param val       ԭʼ����ֵ
     */
    static bool bcdfmtReadValue(bool is_zip, bool is_bigend, bool all_num,
            size_t offset, const char* data, size_t len, std::string& val);
};

/*!
 *  �ַ���
 */
class StrFormat : public FormatBase
{
private:
    /*!
     * ��õ�BCD���ݴ�
     */
    std::string str_Value;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

#include "format_base.inl"

#endif // FORMAT_BASE_HPP

