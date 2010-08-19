/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  BillBase
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline BillBase::BillBase(const char* name)
{
    billbase_Name = name;
}

inline const char* BillBase::billbaseGetName()
{
    return billbase_Name.c_str();
}

inline void BillBase::billbaseReset()
{
    billbase_NodeMap.clear();
}

inline void BillBase::billbaseAddNode(const char* name, ValueBase* val, FormatBase* fmt)
{
    billbase_NodeMap[name] = BILL_NODE(val, fmt);
}

