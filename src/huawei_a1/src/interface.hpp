#ifndef MODULE_INTERFACE_HPP
#define MODULE_INTERFACE_HPP

#include <memory>

extern "C" 
{

/*!
 * ���̰߳汾�ĳ�ʼ�����ͷ�
 */
int InitMT();
void UninitMT(int handle);

/*!
 * ���ػ�������
 */
const char* GetTypeName(int type_idx);

/*!
 * ���ػ������͵�ÿ���ֶ�����
 */
const char* GetItemName(int type_idx,  int item_idx);

/********************************************************************
 *                                                                  *
 *  һ�º����� *MTΪ���̰߳汾���ȵ��̰߳汾��һ���������˾������  *
 *                                                                  *
 ********************************************************************/

/*!
 * ����������ڣ�������ͬʱ�������������ļ���
 */
bool ParseFile(const char* file_name); 
bool ParseFileMT(int handle, const char* file_name); 


/*!
 * ��ȡ��һ����������
 */
bool FetchNextBill();
bool FetchNextBillMT(int handle);

/*!
 * ��ǰ�������Ļ�������
 */
int CurBillType();
int CurBillTypeMT(int handle);

/*!
 * ��ǰ�������Ļ�������
 */
const char* CurBillName();
const char* CurBillNameMT(int handle);

/*!
 * ��û���ĳ������
 */
const char* GetItemData(const char* item_name);
const char* GetItemDataMT(int handle, const char* item_name);

/*
 * ����������
 */
int huawei_a1(char * in_file_name, char * out_file_name, int * rec_num);

}

#endif //MODULE_INTERFACE_HPP

