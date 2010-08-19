#include "interface.hpp"
#include "module_base.hpp"

#include <map>

/*!
 * 话单模块指针
 */
extern std::auto_ptr<BillModule> glb_BillModulePtr;

/*!
 * 多线程初始化
 */
int InitMT()
{
    BillModule* loc_NewMod = glb_BillModulePtr->billmodClone();
    return reinterpret_cast<int>(loc_NewMod);
}

/*!
 * 多线程释放
 */
void UninitMT(int handle)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    delete loc_Mod;
}

/*!
 * 返回话单类型
 */
const char* GetTypeName(int type_idx)
{
    return glb_BillModulePtr->billmodGetTypeName(type_idx);
}

/*!
 * 返回话单类型的每项字段类型
 */
const char* GetItemName(int type_idx,  int item_idx)
{
    return glb_BillModulePtr->billmodGetItemName(type_idx, item_idx);
}

/*!
 * 解析话单入口（单线程版本不允许同时解析两个话单文件）
 */
bool ParseFile(const char* file_name)
{
    return glb_BillModulePtr->billmodParseFile(file_name);
}
bool ParseFileMT(int handle, const char* file_name)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    return loc_Mod->billmodParseFile(file_name);
}

/*!
 * 获取下一条话单内容
 */
bool FetchNextBill()
{
    return glb_BillModulePtr->billmodFetchNextBill();
}
bool FetchNextBillMT(int handle)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    return loc_Mod->billmodFetchNextBill();
}

/*!
 * 当前解析出的话单类型
 */
int CurBillType()
{
    return glb_BillModulePtr->billmodCurBillType();
}
int CurBillTypeMT(int handle)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    return loc_Mod->billmodCurBillType();
}

/*!
 * 当前解析出的话单类型
 */
const char* CurBillName()
{
    return glb_BillModulePtr->billmodCurBillName();
}
const char* CurBillNameMT(int handle)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    return loc_Mod->billmodCurBillName();
}

/*!
 * 获得话单某项内容
 */
const char* GetItemData(const char* item_name)
{
    return glb_BillModulePtr->billmodGetItemData(item_name);
}
const char* GetItemDataMT(int handle, const char* item_name)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    return loc_Mod->billmodGetItemData(item_name);
}

