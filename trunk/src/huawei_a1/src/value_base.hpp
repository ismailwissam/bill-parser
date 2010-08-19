#ifndef VALUE_BASE_HPP
#define VALUE_BASE_HPP

#include <istream>

/*!
 * ���ݻ���
 */
class ValueBase
{
private:
    /*!
     * �������Ƿ���Ч
     */
    bool val_Ok;

protected:
    /*!
     * ���������ж�ȡ����
     */
    virtual bool valReadData(std::istream& in) = 0;

public:
    /*!
     * ���캯��
     */
    ValueBase();

    /*!
     * ���麯�������ض�ȡ��������ָ��
     */
    virtual const char* valRawData() const = 0;

    /*!
     * ���麯�������ض�ȡ�������ݳ���
     */
    virtual size_t valRawDataLen() const = 0;

    /*!
     * ����ֵ�Ƿ���Ч
     */
    void valOk(bool ok);

    /*!
     * ���ظ�ֵ�Ƿ���Ч
     */
    bool valOk() const;

private: 
	friend std::istream& operator >> (std::istream& in, ValueBase& v);
};

/*!
 * �̶�����������
 */
template<int L>
class StaticLenValue : public ValueBase
{
private:
    /*!
     * ���ݻ���
     */
    char staticlenval_Buff[L];

protected:
    /*!
     * ��ȡ���ݵĺ���
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
     * ���ض�ȡ��ԭʼ����
     */
    virtual const char* valRawData() const
    {
        return staticlenval_Buff;
    }

    /*!
     * ���ض�ȡ��ԭʼ���ݵĳ���
     */
    virtual size_t valRawDataLen() const
    {
        return L;
    }
};

/*!
 * ��̬����������
 */
class DynLenValue : public ValueBase
{
private:
    /*!
     * ���ݻ���
     */
    std::string dynlenval_Buff;

protected:
    /*!
     * ��ȡ���ݵĺ���, ���ڶ�̬���ȣ���ʵ��
     */
    virtual bool valReadData(std::istream& in)
    {
        return true;
    }

public: 
    /*!
     * ���ض�ȡ��ԭʼ����
     */
    virtual const char* valRawData() const
    {
        return dynlenval_Buff.data();
    }

    /*!
     * ���ض�ȡ��ԭʼ���ݵĳ���
     */
    virtual size_t valRawDataLen() const
    {
        return dynlenval_Buff.size();
    }

    /*!
     * ���ö�̬��������
     */
    void dynlenvalSetData(const char* data, size_t len)
    {
        dynlenval_Buff.assign(data, len);
    }

    /*!
     * ���ö�̬��������
     */
    void dynlenvalSetData(const std::string& str)
    {
        dynlenval_Buff.assign(str);
    }
};
#include "value_base.inl"

#endif //VALUE_BASE_HPP

