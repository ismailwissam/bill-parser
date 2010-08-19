
#include "format.hpp"
#include "hw_asn1.hpp"
#include <stdio.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  HuaweiIsdnFmt
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HuaweiIsdnFmt::fmtPerlStr()
{
	//
	// 华为电话号码格式长度是固定的14位
	//
	if(fmt_Data == NULL || fmt_Len != 14)
	{
		return NULL;
	}

	const unsigned char* loc_Data = (const unsigned char*)fmt_Data;

	// 读取 号码计划
	hwisdnfmt_IsdnNpi = loc_Data[0] & 0x0f;

	// 读取 号码类型
	hwisdnfmt_IsdnNai = (loc_Data[0] >> 4) & 0x07;

	// 读取 号码长度
	hwisdnfmt_IsdnLen = loc_Data[1] & 0x1f;

	// 读取号码长度
    if(BCDFormat::bcdfmtReadValue(true, true, true, 0, fmt_Data+2, 12, hwisdnfmt_Isdn))
    {
        return hwisdnfmt_Isdn.c_str();
    }

	return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  HuaweiDateFmt
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HuaweiDateFmt::fmtPerlStr()
{
	//
	// 华为日期格式长度有6/7位两种，区别在于年份用1位还是2位
	//
	if(fmt_Data == NULL || (fmt_Len != 6 && fmt_Len != 7))
	{
		return NULL;
	}

    unsigned long long loc_Year;
    unsigned long long loc_Mon;
    unsigned long long loc_Day;
    unsigned long long loc_Hour;
    unsigned long long loc_Min;
    unsigned long long loc_Sec;
    
    if(fmt_Len == 6)
    {
        UIntFormat::uintfmtReadValue(true, fmt_Data, 1, loc_Year);
        loc_Year += 2000;
        UIntFormat::uintfmtReadValue(true, fmt_Data+1, 1, loc_Mon);
        UIntFormat::uintfmtReadValue(true, fmt_Data+2, 1, loc_Day);
        UIntFormat::uintfmtReadValue(true, fmt_Data+3, 1, loc_Hour);
        UIntFormat::uintfmtReadValue(true, fmt_Data+4, 1, loc_Min);
        UIntFormat::uintfmtReadValue(true, fmt_Data+5, 1, loc_Sec);
    }
    else
    {
        UIntFormat::uintfmtReadValue(true, fmt_Data, 2, loc_Year);
        UIntFormat::uintfmtReadValue(true, fmt_Data+2, 1, loc_Mon);
        UIntFormat::uintfmtReadValue(true, fmt_Data+3, 1, loc_Day);
        UIntFormat::uintfmtReadValue(true, fmt_Data+4, 1, loc_Hour);
        UIntFormat::uintfmtReadValue(true, fmt_Data+5, 1, loc_Min);
        UIntFormat::uintfmtReadValue(true, fmt_Data+6, 1, loc_Sec);
    }


    //
    // (unsigned) long long 类型不能传递给 sprintf / printf 等可变参数函数
    //
    sprintf(hwdatefmt_Buff, "%04d-%02d-%02d %02d:%02d:%02d",
            (unsigned int)loc_Year, (unsigned int)loc_Mon, (unsigned int)loc_Day,
            (unsigned int)loc_Hour, (unsigned int)loc_Min, (unsigned int)loc_Sec);

    return hwdatefmt_Buff;
}

// ---------------------------------- ASN.1 ----------------------------------
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  华为ASN1普通号码格式
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HwA1IsdnFmt::fmtPerlStr()
{
	if(fmt_Data == NULL)
	{
		return NULL;
	}

    std::string loc_TempStr;
    if(BCDFormat::bcdfmtReadValue(true, false, false, 0, fmt_Data, fmt_Len, loc_TempStr))
    {
        size_t loc_Pos = loc_TempStr.find_last_not_of('F');
        if(loc_Pos == std::string::npos)
        {
            loc_Pos = loc_TempStr.size();
        }
        else
        {
            loc_Pos += 1;
        }

        hwa1fmt_Isdn.assign(loc_TempStr, 0, loc_Pos);
        return hwa1fmt_Isdn.c_str();
    }

	return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  华为ASN1电话号码格式
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HwA1NumberFmt::fmtPerlStr()
{
	if(fmt_Data == NULL)
	{
		return NULL;
	}

	const unsigned char* loc_Data = (const unsigned char*)fmt_Data;

    // 获取号码计划
    hwa1fmt_NumberNpi = loc_Data[0] & 0x0F;

    // 获取号码类型
    hwa1fmt_NumberNai = (loc_Data[0] >> 4) & 0x07;
    
    // 获取号码
    std::string loc_TempStr;
    if(BCDFormat::bcdfmtReadValue(true, false, true, 0, fmt_Data+1, fmt_Len-1, loc_TempStr))
    {
        size_t loc_Pos = loc_TempStr.find_last_not_of('F');
        if(loc_Pos == std::string::npos)
        {
            loc_Pos = loc_TempStr.size();
        }
        else
        {
            loc_Pos += 1;
        }

        hwa1fmt_Number.assign(loc_TempStr, 0, loc_Pos);
        return hwa1fmt_Number.c_str();
    }

	return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  华为 ASN1 包含ASN1格式字符串
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HwA1PackedStrFmt::fmtPerlStr()
{
	if(fmt_Data == NULL)
	{
		return NULL;
	}

    std::string loc_TempStr(fmt_Data, fmt_Len);
    std::istringstream loc_IStream(loc_TempStr);

    int loc_Tag;
    size_t loc_Len;
    if(!hwasn1GetTag(loc_IStream, loc_Tag)
            || !hwasn1GetLen(loc_IStream, loc_Len)
            || !hwasn1GetCont(loc_IStream, loc_Len, hwa1fmt_Str))
    {
        return NULL;
    }

    return hwa1fmt_Str.c_str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  华为 ASN1 Lac-ci
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HwA1LacCiFmt::fmtPerlStr()
{
	if(fmt_Data == NULL)
	{
		return NULL;
	}

    std::string loc_TempStr(fmt_Data, fmt_Len);
    std::istringstream loc_IStream(loc_TempStr);

    int loc_Tag;
    size_t loc_Len;
    std::string loc_Lac;
    std::string loc_Ci;
    if(!hwasn1GetTag(loc_IStream, loc_Tag)
            || !hwasn1GetLen(loc_IStream, loc_Len)
            || !hwasn1GetCont(loc_IStream, loc_Len, loc_Lac)
            || !hwasn1GetTag(loc_IStream, loc_Tag)
            || !hwasn1GetLen(loc_IStream, loc_Len)
            || !hwasn1GetCont(loc_IStream, loc_Len, loc_Ci))
    {
        return NULL;
    }

    std::string loc_LacStr;
    std::string loc_CiStr;
    if(BCDFormat::bcdfmtReadValue(true, false, false, 0, loc_Lac.data(), loc_Lac.size(), loc_LacStr)
            && BCDFormat::bcdfmtReadValue(true, false, false, 0, loc_Ci.data(), loc_Ci.size(), loc_CiStr))
    {
        hwa1fmt_LacCi = loc_LacStr;
        hwa1fmt_LacCi += "-";
        hwa1fmt_LacCi += loc_CiStr;

        return hwa1fmt_LacCi.c_str();
    }
    else
    {
        return NULL;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  华为 ASN1 Time
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* HwA1TimeFmt::fmtPerlStr()
{
	if(fmt_Data == NULL || fmt_Len != 9)
	{
		return NULL;
	}

    static const char* _NUM_ = "0123456789      ";

    // --- 年 ---
    hwa1fmt_Time = "20";
    hwa1fmt_Time += _NUM_[(fmt_Data[0]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[0]&0x0f];
    hwa1fmt_Time += '-';
    // --- 月 ---
    hwa1fmt_Time += _NUM_[(fmt_Data[1]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[1]&0x0f];
    hwa1fmt_Time += '-';
    // --- 日 ---
    hwa1fmt_Time += _NUM_[(fmt_Data[2]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[2]&0x0f];
    hwa1fmt_Time += ' ';
    // --- 时 ---
    hwa1fmt_Time += _NUM_[(fmt_Data[3]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[3]&0x0f];
    hwa1fmt_Time += ':';
    // --- 分 ---
    hwa1fmt_Time += _NUM_[(fmt_Data[4]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[4]&0x0f];
    hwa1fmt_Time += ':';
    // --- 秒 ---
    hwa1fmt_Time += _NUM_[(fmt_Data[5]>>4)&0x0f];
    hwa1fmt_Time += _NUM_[fmt_Data[5]&0x0f];

    return hwa1fmt_Time.c_str();
}

