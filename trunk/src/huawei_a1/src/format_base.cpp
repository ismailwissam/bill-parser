
#include "format_base.hpp"

const char* BCDSTR = "0123456789ABCDEF";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  FormatBase
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* FormatBase::fmtPerlStr()
{
	if(fmt_Data == NULL)
	{
		return NULL;
	}

    std::string loc_Str(fmt_Data, fmt_Len);
    return loc_Str.c_str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  UIntFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* UIntFormat::fmtPerlStr()
{
    unsigned long long loc_Value;
    if(uintfmtReadValue(uint_IsBigEnd, fmt_Data, fmt_Len, loc_Value))
    {
        uint_Stream.str("");
        uint_Stream << loc_Value;
        return uint_Stream.str().c_str();
    }
    else
    {
        return NULL;
    }
}

bool UIntFormat::uintfmtReadValue(bool is_bigend, const char* data, size_t len, unsigned long long& val)
{
    if(data == NULL || len <= 0)
    {
        return false;
    }

    const unsigned char* loc_Data = (const unsigned char*)data;

    val = 0;
    if(is_bigend)
    { 
        // 高位在后
        for(int loc_I = len - 1; loc_I >= 0; --loc_I)
        {
            val <<= 8;
            val |= loc_Data[loc_I];
        }
    }
    else
    {
        // 高位在前
        for(int loc_I = 0; loc_I < len; ++loc_I)
        {
            val <<= 8;
            val |= loc_Data[loc_I];
        }
    }

    return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  IntFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* IntFormat::fmtPerlStr()
{
    long long loc_Value;
    if(intfmtReadValue(int_IsBigEnd, fmt_Data, fmt_Len, loc_Value))
    {
        int_Stream.str("");
        int_Stream << loc_Value;
        return int_Stream.str().c_str();
    }

    return NULL;
}

bool IntFormat::intfmtReadValue(bool is_bigend, const char* data, size_t len, long long& val)
{
    if(data == NULL || len <= 0)
    {
        return false;
    }

    const unsigned char* loc_Data = (const unsigned char*)data;

    //
    // 判断是否有符号位置
    //
    // 1. 首先获得高位的第一个字符
    //
    unsigned char loc_FirstChar;
    if(is_bigend)
    {
        loc_FirstChar = loc_Data[len - 1];
    }
    else
    {
        loc_FirstChar = loc_Data[0];
    }

    //
    // 2. 判断最高位是否是 1 ；如果是1 则为负数，否则是整数
    //
    if(loc_FirstChar & 0x80)
    {
        // 负数
        val = 0xffffffffffffffffLL;
    }
    else
    {
        // 正数
        val = 0;
    }

    if(is_bigend)
    { 
        // 高位在后
        for(int loc_I = len - 1; loc_I >= 0; --loc_I)
        {
            val <<= 8;
            val |= loc_Data[loc_I];
        }
    }
    else
    {
        // 高位在前
        for(int loc_I = 0; loc_I < len; ++loc_I)
        {
            val <<= 8;
            val |= loc_Data[loc_I];
        }
    }

    return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  BCDFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* BCDFormat::fmtPerlStr()
{
    if(bcdfmtReadValue(bcd_IsZip, bcd_IsBigEnd,
			bcd_AllNum, 0, fmt_Data, fmt_Len, bcd_Value))
    {
        return bcd_Value.c_str();
    }

    return NULL;
}

bool BCDFormat::bcdfmtReadValue(bool is_zip, bool is_bigend, bool all_num, 
        size_t offset, const char* data, size_t len, std::string& val)
{
    if(data == NULL || len <= 0 || offset >= len)
    {
        return false;
    }

    size_t loc_Len = (is_zip) ? len*2 : len;

    val.reserve(loc_Len);
    val.clear();

    unsigned char loc_Char;
    for(size_t loc_I = offset; loc_I < loc_Len; ++loc_I)
    {
        if(is_zip)
        {
            loc_Char = data[loc_I/2];
            if(loc_I%2 == 0)
            {
                loc_Char = (is_bigend) ? loc_Char >> 4: loc_Char;
            }
            else
            {
                loc_Char = (is_bigend) ? loc_Char : loc_Char >> 4;
            }
        }
        else
        {
            loc_Char = data[loc_I];
        }

        loc_Char &= 0x0f;

        if(all_num && loc_Char > 9)
        {
            return true;
        }

        val += BCDSTR[loc_Char];
    }

    return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  StrFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* StrFormat::fmtPerlStr()
{
    str_Value.assign(fmt_Data, fmt_Len);
    return str_Value.c_str();
}

