#include "module_base.hpp"

bool BillModule::billmodParseFile(const char* file_name)
{
    FUNC_TRACE_LOG;

    // �ر�֮ǰ���ļ�
	if(billmod_FStream.is_open())
	{
		billmod_FStream.close();
	}

    // ���õ�ǰ����Ϊ��Ч
    billmod_CurBillType = INVAILD_BILL_TYPE;

    // �ļ����Ƿ�Ϊ��
    if(file_name == NULL)
    {
        return false;
    }

	billmod_FStream.clear();
    billmod_FStream.open(file_name, std::ios::in|std::ios::binary);

    return billmod_FStream.good();
}

