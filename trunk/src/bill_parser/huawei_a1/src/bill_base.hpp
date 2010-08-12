#ifndef BILL_BASE_HPP
#define BILL_BASE_HPP

#include <istream>
#include <string>
#include <vector>
#include <map>
#include "value_base.hpp"
#include "format_base.hpp"

/*!
 * �����ڵ�����
 */
typedef std::pair<ValueBase*, FormatBase*> BILL_NODE;

/*!
 * �����ڵ��б�����
 */
typedef std::map<std::string, BILL_NODE> BILL_NODE_MAP;

/*!
 * ��������
 */
class BillBase
{
protected:
    //! ��������
    std::string billbase_Name;

    //! ���������ļ������������ֶ�
    BILL_NODE_MAP billbase_NodeMap;

public:
    /*!
     * ���캯��
     */
    BillBase(const char* name);

    /*!
     * ���ػ�������
     */
    const char* billbaseGetName();

    /*!
     * �����ֶ�����
     */
    virtual const char* billbaseItemName(int idx) = 0;

    /*!
     * ������нڵ�
     */
    void billbaseReset();

    /*!
     * ���ڵ����ӵ��б���
     */
    void billbaseAddNode(const char* name, ValueBase* val, FormatBase* fmt);

    /*!
     * �����ֶ����кţ����ظ�perl�ַ���
     */
    virtual const char* billbasePerlStr(const char* name);

    /*!
     * ��ȡ��������
     * ������Ľ�����뽫��Ч���ֶα����� billbase_NodeMap ��
     */
    virtual bool billbaseReadData(std::istream& in) = 0;
};

/*!
 * �����б�
 */
typedef std::vector<BillBase*> BILL_LIST;

#include "bill_base.inl"

#endif // BILL_BASE_HPP

