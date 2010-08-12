#ifndef VALUE_BASE_HPP
#define VALUE_BASE_HPP

#include <istream>

/*!
 * 数据基类
 */
class ValueBase
{
private:
    /*!
     * 该数据是否有效
     */
    bool val_Ok;

protected:
    /*!
     * 从输入流中读取数据
     */
    virtual bool valReadData(std::istream& in) = 0;

public:
    /*!
     * 构造函数
     */
    ValueBase();

    /*!
     * 纯虚函数，返回读取到的数据指针
     */
    virtual const char* valRawData() const = 0;

    /*!
     * 纯虚函数，返回读取到的数据长度
     */
    virtual size_t valRawDataLen() const = 0;

    /*!
     * 设置值是否有效
     */
    void valOk(bool ok);

    /*!
     * 返回该值是否有效
     */
    bool valOk() const;

private: 
	friend std::istream& operator >> (std::istream& in, ValueBase& v);
};

/*!
 * 固定长度数据类
 */
template<int L>
class StaticLenValue : public ValueBase
{
private:
    /*!
     * 数据缓冲
     */
    char staticlenval_Buff[L];

protected:
    /*!
     * 读取数据的函数
     */
    virtual bool valReadData(std::istream& in)
    {
        if(in.bad() || in.eof())
        {
            return false;
        }

        in.read(staticlenval_Buff, L);
        return in.good();
    }

public: 
    /*!
     * 返回读取的原始数据
     */
    virtual const char* valRawData() const
    {
        return staticlenval_Buff;
    }

    /*!
     * 返回读取的原始数据的长度
     */
    virtual size_t valRawDataLen() const
    {
        return L;
    }
};

/*!
 * 动态长度数据类
 */
class DynLenValue : public ValueBase
{
private:
    /*!
     * 数据缓冲
     */
    std::string dynlenval_Buff;

protected:
    /*!
     * 读取数据的函数, 对于动态长度，不实现
     */
    virtual bool valReadData(std::istream& in)
    {
        return true;
    }

public: 
    /*!
     * 返回读取的原始数据
     */
    virtual const char* valRawData() const
    {
        return dynlenval_Buff.data();
    }

    /*!
     * 返回读取的原始数据的长度
     */
    virtual size_t valRawDataLen() const
    {
        return dynlenval_Buff.size();
    }

    /*!
     * 设置动态长度数据
     */
    void dynlenvalSetData(const char* data, size_t len)
    {
        dynlenval_Buff.assign(data, len);
    }

    /*!
     * 设置动态长度数据
     */
    void dynlenvalSetData(const std::string& str)
    {
        dynlenval_Buff.assign(str);
    }
};
#include "value_base.inl"

#endif //VALUE_BASE_HPP

