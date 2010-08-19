#include "interface.hpp"
#include "module_base.hpp"

#include <map>

/*!
 * ����ģ��ָ��
 */
extern std::auto_ptr<BillModule> glb_BillModulePtr;

/*!
 * ���̳߳�ʼ��
 */
int InitMT()
{
    BillModule* loc_NewMod = glb_BillModulePtr->billmodClone();
    return reinterpret_cast<int>(loc_NewMod);
}

/*!
 * ���߳��ͷ�
 */
void UninitMT(int handle)
{
    BillModule* loc_Mod = reinterpret_cast<BillModule*>(handle);
    delete loc_Mod;
}

/*!
 * ���ػ�������
 */
const char* GetTypeName(int type_idx)
{
    return glb_BillModulePtr->billmodGetTypeName(type_idx);
}

/*!
 * ���ػ������͵�ÿ���ֶ�����
 */
const char* GetItemName(int type_idx,  int item_idx)
{
    return glb_BillModulePtr->billmodGetItemName(type_idx, item_idx);
}

/*!
 * ����������ڣ����̰߳汾������ͬʱ�������������ļ���
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
 * ��ȡ��һ����������
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
 * ��ǰ�������Ļ�������
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
 * ��ǰ�������Ļ�������
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
 * ��û���ĳ������
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

