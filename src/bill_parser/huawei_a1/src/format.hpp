#ifndef HUAWEI_FORMAT_HPP
#define HUAWEI_FORMAT_HPP

#include "format_base.hpp"

/*!
 * 华为号码格式
 */
class HuaweiIsdnFmt : public FormatBase
{
private:
	// 号码计划
	int hwisdnfmt_IsdnNpi;

	// 号码类型
	int hwisdnfmt_IsdnNai;

	// 号码长度
	int hwisdnfmt_IsdnLen;

	// 号码
	std::string hwisdnfmt_Isdn;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 * 华为时间格式
 */
class HuaweiDateFmt : public FormatBase
{
private:
    // 时间
    char hwdatefmt_Buff[32];

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

// ---------------------- ASN.1 -------------------------
/*!
 * 华为ASN1普通号码格式
 */
class HwA1IsdnFmt : public FormatBase
{
private:
    std::string hwa1fmt_Isdn;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 * 华为ASN1电话号码格式
 */
class HwA1NumberFmt : public FormatBase
{
private:
    //号码计划
    int hwa1fmt_NumberNpi;
    //号码类型
    int hwa1fmt_NumberNai;
    //电话号码
    std::string hwa1fmt_Number;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 * 华为 ASN1 包含ASN1格式字符串
 */
class HwA1PackedStrFmt : public FormatBase
{
private:
    std::string hwa1fmt_Str;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 * 华为 ASN1 Lac-ci
 */
class HwA1LacCiFmt : public FormatBase
{
private:
    std::string hwa1fmt_LacCi;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 * 华为 ASN1 Time
 */
class HwA1TimeFmt : public FormatBase
{
private:
    std::string hwa1fmt_Time;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

#endif //HUAWEI_FORMAT_HPP

