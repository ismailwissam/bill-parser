#ifndef HUAWEI_ASN1_HPP
#define HUAWEI_ASN1_HPP

#include <istream>

/*!
 * 获得ASN1的Tag
 */
bool hwasn1GetTag(std::istream& input, int& tag);

/*!
 * 获得ASN1的Len
 */
bool hwasn1GetLen(std::istream& input, size_t& len);

/*!
 * 获得ASN1的Cont
 */
bool hwasn1GetCont(std::istream& input, size_t len, std::string& cont);

#endif //HUAWEI_ASN1_HPP

