#ifndef HUAWEI_FORMAT_HPP
#define HUAWEI_FORMAT_HPP

#include "format_base.hpp"

/*!
 * ��Ϊ�����ʽ
 */
class HuaweiIsdnFmt : public FormatBase
{
private:
	// ����ƻ�
	int hwisdnfmt_IsdnNpi;

	// ��������
	int hwisdnfmt_IsdnNai;

	// ���볤��
	int hwisdnfmt_IsdnLen;

	// ����
	std::string hwisdnfmt_Isdn;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 * ��Ϊʱ���ʽ
 */
class HuaweiDateFmt : public FormatBase
{
private:
    // ʱ��
    char hwdatefmt_Buff[32];

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

// ---------------------- ASN.1 -------------------------
/*!
 * ��ΪASN1��ͨ�����ʽ
 */
class HwA1IsdnFmt : public FormatBase
{
private:
    std::string hwa1fmt_Isdn;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 * ��ΪASN1�绰�����ʽ
 */
class HwA1NumberFmt : public FormatBase
{
private:
    //����ƻ�
    int hwa1fmt_NumberNpi;
    //��������
    int hwa1fmt_NumberNai;
    //�绰����
    std::string hwa1fmt_Number;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 * ��Ϊ ASN1 ����ASN1��ʽ�ַ���
 */
class HwA1PackedStrFmt : public FormatBase
{
private:
    std::string hwa1fmt_Str;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 * ��Ϊ ASN1 Lac-ci
 */
class HwA1LacCiFmt : public FormatBase
{
private:
    std::string hwa1fmt_LacCi;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

/*!
 * ��Ϊ ASN1 Time
 */
class HwA1TimeFmt : public FormatBase
{
private:
    std::string hwa1fmt_Time;

public:
    /*!
     * ����perl��ȡ���ַ�����ʽ
     */
    virtual const char* fmtPerlStr();
};

#endif //HUAWEI_FORMAT_HPP

