#ifndef MODULE_BASE_HPP
#define MODULE_BASE_HPP

#include "bill_base.hpp"
#include "tlog.hpp"
#include <fstream>
#include <iostream>

/*!
 * 话单模块基类
 */
class BillModule
{
protected:
    enum { INVAILD_BILL_TYPE = -1 };

    /*!
     * 该模块支持的话单列表
     */
    BILL_LIST billmod_BillList;

    /*!
     * 需要解析话单文件的句柄
     */
    std::ifstream billmod_FStream;

    /*!
     * 当前解析出的话单编号
     */
    int billmod_CurBillType;

protected:
    /*!
     * 向话单列表中增加话单对象指针
     */
    void billmodAddBill(BillBase* bill);

	/*!
	 * 获得当前的数据输入流
	 */
	std::istream& billmodStream();

public:
    /*!
     * 构造函数
     */
    BillModule();

    /*!
     * 克隆自身
     */
    virtual BillModule* billmodClone() const = 0;

    /*!
     * 返回话单名
     */
    const char* billmodGetTypeName(int idx);

    /*!
     * 返回话单类型的每项字段类型
     */
    const char* billmodGetItemName(int type_idx, int item_idx);

    /*!
     * 开始解析文件
     */
    virtual bool billmodParseFile(const char* file_name);

    /*!
     * 读取下一条话单
     */
    virtual bool billmodFetchNextBill() = 0;

	/*!
	 * 设置当前解析的话单
	 */
	void billmodCurBillType(int type);

    /*!
     * 返回当前解析的话单ID
     */
    int billmodCurBillType();

    /*!
     * 返回当前解析的话单名称
     */
    const char* billmodCurBillName();

    /*!
     * 获得当前话单内容
     */
    const char* billmodGetItemData(const char* item_name);
};

#include "module_base.inl"

#endif // MODULE_BASE_HPP
