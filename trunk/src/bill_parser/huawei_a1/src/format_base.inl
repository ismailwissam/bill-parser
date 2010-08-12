
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  FormatBase
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline FormatBase::FormatBase()
{
    fmt_Data = NULL;
    fmt_Len = 0;
}

inline void FormatBase::fmtSetValue(const char* data, size_t len)
{
    fmt_Data = data;
    fmt_Len = len;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  UIntFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline UIntFormat::UIntFormat(bool is_bigend)
{
    uint_IsBigEnd = is_bigend;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  IntFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline IntFormat::IntFormat(bool is_bigend)
{
    int_IsBigEnd = is_bigend;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  BCDFormat
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
inline BCDFormat::BCDFormat(bool is_zip, bool is_bigend, bool all_num)
{
    bcd_IsZip = is_zip;
    bcd_IsBigEnd = is_bigend;
	bcd_AllNum = all_num;
}

