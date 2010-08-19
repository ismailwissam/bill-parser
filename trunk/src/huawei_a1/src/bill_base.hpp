#ifndef BILL_BASE_HPP
#define BILL_BASE_HPP

#include <istream>
#include <string>
#include <vector>
#include <map>
#include "value_base.hpp"
#include "format_base.hpp"

/*!
 * 话单节点类型
 */
typedef std::pair<ValueBase*, FormatBase*> BILL_NODE;

/*!
 * 话单节点列表类型
 */
typedef std::map<std::string, BILL_NODE> BILL_NODE_MAP;

/*!
 * 话单基类
 */
class BillBase
{
protected:
    //! 话单名称
    std::string billbase_Name;

    //! 这条话单文件包含的所有字段
    BILL_NODE_MAP billbase_NodeMap;

public:
    /*!
     * 构造函数
     */
    BillBase(const char* name);

    /*!
     * 返回话单名称
     */
    const char* billbaseGetName();

    /*!
     * 返回字段名称
     */
    virtual const char* billbaseItemName(int idx) = 0;

    /*!
     * 清除所有节点
     */
    void billbaseReset();

    /*!
     * 将节点增加到列表中
     */
    void billbaseAddNode(const char* name, ValueBase* val, FormatBase* fmt);

    /*!
     * 根据字段序列号，返回给perl字符串
     */
    virtual const char* billbasePerlStr(const char* name);

    /*!
     * 读取话单数据
     * 解析后的结果必须将有效的字段保存在 billbase_NodeMap 中
     */
    virtual bool billbaseReadData(std::istream& in) = 0;
};

/*!
 * 话单列表
 */
typedef std::vector<BillBase*> BILL_LIST;

#include "bill_base.inl"

#endif // BILL_BASE_HPP

