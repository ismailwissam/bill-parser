
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  ValueBase
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline ValueBase::ValueBase()
{
    val_Ok = true;
}

inline void ValueBase::valOk(bool ok)
{
    val_Ok = ok;
}

inline bool ValueBase::valOk() const
{
    return val_Ok;
}

//
// global function
//
inline std::istream& operator >> (std::istream& in, ValueBase& v)
{
    v.valOk(v.valReadData(in));
    return in;
}

