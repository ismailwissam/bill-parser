#include "hw_asn1.hpp"
#include <iostream>
#include <stdexcept>

/*!
 * 获得ASN1的Tag
 */
bool hwasn1GetTag(std::istream& input, int& tag)
{
    unsigned char loc_Char;
    int loc_Tag = 0;

    // 读取第一个字符
    loc_Char = input.get();
    loc_Tag |= loc_Char;

    // 如果后 5 位都是 1，则后续N个字节也是Tag
    if((loc_Char & 0x1f) == 0x1f)
    {
        int loc_Len = 1;
        while((loc_Len++) <= 4)
        {
            if(input.bad() || input.eof())
            {
                return false;
            }

            loc_Char = input.get();

            //第一个后续字节的第7位到第1位不能全为0
            if(loc_Len == 2)
            {
                if(!((loc_Char & 0x7f) | 0x00))
                {
                    return false;
                }
            }

            loc_Tag <<= 8;
            loc_Tag |= loc_Char;

            //后续字节除最后一个字节外，其他字节的第8位为1
            //如果第8位为0，则说明是最后一个后续字节
            if((loc_Char & 0x80) != 0x80)
            {
                break;
            }
        }

        // 华为格式里 Tag 不会大于 4
        if(loc_Len > 4)
        {
            return false;
        }
    }

    tag = loc_Tag;
    return input.good();
}

/*!
 * 获得ASN1的Len
 */
bool hwasn1GetLen(std::istream& input, size_t& len)
{
    unsigned char loc_Char;
    size_t loc_Len = 0;

    // 读取第一个字符
    loc_Char = input.get();
    loc_Len |= loc_Char;

    // 如果第一位是1，则后7位表示长度所占的字节数
    if((loc_Char & 0x80) == 0x80)
    {
        loc_Len = 0;
        int loc_LenSize = loc_Char & 0x7f;
        while((loc_LenSize--) > 0)
        {
            if(input.bad() || input.eof())
            {
                return false;
            }

            loc_Char = input.get();
            loc_Len <<= 8;
            loc_Len |= loc_Char;
        }
    }

    len = loc_Len;
    return input.good();
}

/*!
 * 获得ASN1的Cont
 */
bool hwasn1GetCont(std::istream& input, size_t len, std::string& cont)
{
    char* loc_Buff = new char[len];
    input.read(loc_Buff, len);

    // 赋值给 cont
    try
    {
        cont.assign(loc_Buff, len);
    }
    catch(std::length_error & e)
    {
        delete []loc_Buff;
        return false;
    }

    delete []loc_Buff;
    return input.good();
}

