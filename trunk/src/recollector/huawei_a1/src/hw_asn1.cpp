#include "hw_asn1.hpp"
#include <iostream>
#include <stdexcept>

/*!
 * ���ASN1��Tag
 */
bool hwasn1GetTag(std::istream& input, int& tag)
{
    unsigned char loc_Char;
    int loc_Tag = 0;

    // ��ȡ��һ���ַ�
    loc_Char = input.get();
    loc_Tag |= loc_Char;

    // ����� 5 λ���� 1�������N���ֽ�Ҳ��Tag
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

            //��һ�������ֽڵĵ�7λ����1λ����ȫΪ0
            if(loc_Len == 2)
            {
                if(!((loc_Char & 0x7f) | 0x00))
                {
                    return false;
                }
            }

            loc_Tag <<= 8;
            loc_Tag |= loc_Char;

            //�����ֽڳ����һ���ֽ��⣬�����ֽڵĵ�8λΪ1
            //�����8λΪ0����˵�������һ�������ֽ�
            if((loc_Char & 0x80) != 0x80)
            {
                break;
            }
        }

        // ��Ϊ��ʽ�� Tag ������� 4
        if(loc_Len > 4)
        {
            return false;
        }
    }

    tag = loc_Tag;
    return input.good();
}

/*!
 * ���ASN1��Len
 */
bool hwasn1GetLen(std::istream& input, size_t& len)
{
    unsigned char loc_Char;
    size_t loc_Len = 0;

    // ��ȡ��һ���ַ�
    loc_Char = input.get();
    loc_Len |= loc_Char;

    // �����һλ��1�����7λ��ʾ������ռ���ֽ���
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
 * ���ASN1��Cont
 */
bool hwasn1GetCont(std::istream& input, size_t len, std::string& cont)
{
    char* loc_Buff = new char[len];
    input.read(loc_Buff, len);

    // ��ֵ�� cont
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

