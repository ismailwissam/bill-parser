#ifndef FORMAT_BASE_HPP
#define FORMAT_BASE_HPP

#include <sstream>
#include "value_base.hpp"

/*!
 * 数据格式化基类
 */
class FormatBase
{
protected:
    /*!
     * 数据指针
     */
    const char* fmt_Data;

    /*!
     * 数据长度
     */
    size_t fmt_Len;

public:
    /*!
     * 构造函数
     */
    FormatBase();

    /*!
     * 设置数据对象指针
     */
    void fmtSetValue(const char* data, size_t len);

    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

/*!
 *  无符号整型格式
 */
class UIntFormat : public FormatBase
{
private:
    /*!
     * 如果为true：则是高位在前、低位在后
     * 如果为false：则是低位在前、高位在后
     */
    bool uint_IsBigEnd;

    /*!
     * 返回数值所有的缓冲区
     */
    std::ostringstream uint_Stream;

public:
    /*!
     * 构造函数
     * @param value 数组对象指针
     * @param is_bigend 是否是高位在前，默认为 true
     */
    UIntFormat(bool is_bigend = true);
    
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();

    /*!
     * 以无符号整型格式读取内容
     */
    static bool uintfmtReadValue(bool is_bigend, const char* data,
            size_t len, unsigned long long& val);
};

/*!
 *  有符号整型格式
 */
class IntFormat : public FormatBase
{
private:
    /*!
     * 如果为true：则是高位在前、低位在后
     * 如果为false：则是低位在前、高位在后
     */
    bool int_IsBigEnd;

    /*!
     * 返回数值所有的缓冲区
     */
    std::ostringstream int_Stream;

public:
    /*!
     * 构造函数
     * @param value 数组对象指针
     * @param is_bigend 是否是高位在前，默认为 true
     */
    IntFormat(bool is_bigend = true);
    
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();

    /*!
     * 以无符号整型格式读取内容
     */
    static bool intfmtReadValue(bool is_bigend, const char* data,
            size_t len, long long& val);
};

/*!
 *  十六进制格式
 */
class BCDFormat : public FormatBase
{
private:
    /*!
     * 如果为true：则是压缩的BCD码
     * 如果为false：则是非压缩的BCD码
     */
    bool bcd_IsZip;

    /*!
     * 如果为true：则是高位在后的BCD码
     * 如果为false：则是高位在前的BCD码
     */
    bool bcd_IsBigEnd;

    /*!
     * 如果为true：则字符串只包含数据，当遇到不属于0~9的数字，则停止
     * 如果为false：则字符可以包含0~9A~F的任何字符
     */
    bool bcd_AllNum;

    /*!
     * 获得的BCD数据串
     */
    std::string bcd_Value;

public:
    /*!
     * 构造函数
     */
    BCDFormat(bool is_zip = true, bool is_bigend = true, bool all_num = true);

    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();

    /*!
     * 以BCD格式读取内容
     * @param is_zip    原始数据是压缩格式的BCD
     * @param is_bigend 原始数据是高位在后的BCD
     * @param offset    忽略前面的一个或多个数字，该数据受is_zip标志的影响
     * @param data      原始数据的首地址
     * @param len       原始数据的长度
     * @param val       原始数据值
     */
    static bool bcdfmtReadValue(bool is_zip, bool is_bigend, bool all_num,
            size_t offset, const char* data, size_t len, std::string& val);
};

/*!
 *  字符串
 */
class StrFormat : public FormatBase
{
private:
    /*!
     * 获得的BCD数据串
     */
    std::string str_Value;

public:
    /*!
     * 返回perl读取的字符串格式
     */
    virtual const char* fmtPerlStr();
};

#include "format_base.inl"

#endif // FORMAT_BASE_HPP

