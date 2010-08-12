#ifndef MODULE_INTERFACE_HPP
#define MODULE_INTERFACE_HPP

#include <memory>

extern "C" 
{

/*!
 * 多线程版本的初始化、释放
 */
int InitMT();
void UninitMT(int handle);

/*!
 * 返回话单类型
 */
const char* GetTypeName(int type_idx);

/*!
 * 返回话单类型的每项字段类型
 */
const char* GetItemName(int type_idx,  int item_idx);

/********************************************************************
 *                                                                  *
 *  一下函数中 *MT为多线程版本，比单线程版本第一个参数多了句柄参数  *
 *                                                                  *
 ********************************************************************/

/*!
 * 解析话单入口（不允许同时解析两个话单文件）
 */
bool ParseFile(const char* file_name); 
bool ParseFileMT(int handle, const char* file_name); 


/*!
 * 获取下一条话单内容
 */
bool FetchNextBill();
bool FetchNextBillMT(int handle);

/*!
 * 当前解析出的话单类型
 */
int CurBillType();
int CurBillTypeMT(int handle);

/*!
 * 当前解析出的话单名称
 */
const char* CurBillName();
const char* CurBillNameMT(int handle);

/*!
 * 获得话单某项内容
 */
const char* GetItemData(const char* item_name);
const char* GetItemDataMT(int handle, const char* item_name);

/*
 * 主解析函数
 */
int huawei_a1(char * in_file_name, char * out_file_name, int * rec_num);

}

#endif //MODULE_INTERFACE_HPP

