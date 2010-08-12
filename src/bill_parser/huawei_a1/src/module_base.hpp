#ifndef MODULE_BASE_HPP
#define MODULE_BASE_HPP

#include "bill_base.hpp"
#include "tlog.hpp"
#include <fstream>
#include <iostream>

/*!
 * ����ģ�����
 */
class BillModule
{
protected:
    enum { INVAILD_BILL_TYPE = -1 };

    /*!
     * ��ģ��֧�ֵĻ����б�
     */
    BILL_LIST billmod_BillList;

    /*!
     * ��Ҫ���������ļ��ľ��
     */
    std::ifstream billmod_FStream;

    /*!
     * ��ǰ�������Ļ������
     */
    int billmod_CurBillType;

protected:
    /*!
     * �򻰵��б������ӻ�������ָ��
     */
    void billmodAddBill(BillBase* bill);

	/*!
	 * ��õ�ǰ������������
	 */
	std::istream& billmodStream();

public:
    /*!
     * ���캯��
     */
    BillModule();

    /*!
     * ��¡����
     */
    virtual BillModule* billmodClone() const = 0;

    /*!
     * ���ػ�����
     */
    const char* billmodGetTypeName(int idx);

    /*!
     * ���ػ������͵�ÿ���ֶ�����
     */
    const char* billmodGetItemName(int type_idx, int item_idx);

    /*!
     * ��ʼ�����ļ�
     */
    virtual bool billmodParseFile(const char* file_name);

    /*!
     * ��ȡ��һ������
     */
    virtual bool billmodFetchNextBill() = 0;

	/*!
	 * ���õ�ǰ�����Ļ���
	 */
	void billmodCurBillType(int type);

    /*!
     * ���ص�ǰ�����Ļ���ID
     */
    int billmodCurBillType();

    /*!
     * ���ص�ǰ�����Ļ�������
     */
    const char* billmodCurBillName();

    /*!
     * ��õ�ǰ��������
     */
    const char* billmodGetItemData(const char* item_name);
};

#include "module_base.inl"

#endif // MODULE_BASE_HPP
