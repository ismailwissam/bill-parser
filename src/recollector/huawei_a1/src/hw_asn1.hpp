#ifndef HUAWEI_ASN1_HPP
#define HUAWEI_ASN1_HPP

#include <istream>

/*!
 * ���ASN1��Tag
 */
bool hwasn1GetTag(std::istream& input, int& tag);

/*!
 * ���ASN1��Len
 */
bool hwasn1GetLen(std::istream& input, size_t& len);

/*!
 * ���ASN1��Cont
 */
bool hwasn1GetCont(std::istream& input, size_t len, std::string& cont);

#endif //HUAWEI_ASN1_HPP

