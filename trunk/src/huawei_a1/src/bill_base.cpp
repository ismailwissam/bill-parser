#include "bill_base.hpp"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  BillBase
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const char* BillBase::billbasePerlStr(const char* name)
{
    if(name == NULL || name[0] == '\0')
    {
        return NULL;
    }

    BILL_NODE_MAP::iterator loc_It = billbase_NodeMap.find(name);

    if(loc_It != billbase_NodeMap.end())
    {
        (*loc_It).second.second->fmtSetValue(
                (*loc_It).second.first->valRawData(),
                (*loc_It).second.first->valRawDataLen());

        return (*loc_It).second.second->fmtPerlStr();
    }

    return NULL;
}

