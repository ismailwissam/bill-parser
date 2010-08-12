/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  BillModule
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline BillModule::BillModule()
{
    FUNC_TRACE_LOG;

    // 设置当前话单为无效
    billmod_CurBillType = INVAILD_BILL_TYPE;
}

inline std::istream& BillModule::billmodStream()
{
	return billmod_FStream;
}


inline void BillModule::billmodAddBill(BillBase* bill)
{
    billmod_BillList.push_back(bill);
}

inline const char* BillModule::billmodGetTypeName(int idx)
{
    if(idx < 0 || idx >= billmod_BillList.size())
    {
        return NULL;
    }

    return billmod_BillList[idx]->billbaseGetName();
}

inline const char* BillModule::billmodGetItemName(int type_idx, int item_idx)
{
    if(type_idx < 0 || type_idx >= billmod_BillList.size())
    {
        return NULL;
    }

    return billmod_BillList[type_idx]->billbaseItemName(item_idx);
}

inline void BillModule::billmodCurBillType(int type)
{
	billmod_CurBillType = type;
}

inline int BillModule::billmodCurBillType()
{
    return billmod_CurBillType;
}

inline const char* BillModule::billmodCurBillName()
{
    if(billmodCurBillType() < 0 || billmodCurBillType() >= billmod_BillList.size())
    {
        return NULL;
    }

    return billmod_BillList[billmodCurBillType()]->billbaseGetName();
}

inline const char* BillModule::billmodGetItemData(const char* item_name)
{
    if(billmodCurBillType() < 0 || billmodCurBillType() >= billmod_BillList.size())
    {
        return NULL;
    }

    return billmod_BillList[billmodCurBillType()]->billbasePerlStr(item_name);
}

