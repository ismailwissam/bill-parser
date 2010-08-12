#include "module_base.hpp"

bool BillModule::billmodParseFile(const char* file_name)
{
    FUNC_TRACE_LOG;

    // 关闭之前的文件
	if(billmod_FStream.is_open())
	{
		billmod_FStream.close();
	}

    // 设置当前话单为无效
    billmod_CurBillType = INVAILD_BILL_TYPE;

    // 文件名是否为空
    if(file_name == NULL)
    {
        return false;
    }

	billmod_FStream.clear();
    billmod_FStream.open(file_name, std::ios::in|std::ios::binary);

    return billmod_FStream.good();
}

