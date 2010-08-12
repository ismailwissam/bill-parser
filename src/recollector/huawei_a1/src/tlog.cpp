#include "tlog.hpp"
#include <iostream>

/*!
 * 构造函数
 */
FuncTraceLog::FuncTraceLog(const char* file_name, const char* func_name, int line_no)
    : funclog_FileName(file_name), funclog_FuncName(func_name), funclog_LineNo(line_no)
{
    std::cout << "Enter => " << funclog_FuncName <<
        "[" << funclog_FileName << ':' << funclog_LineNo << "]"
        << std::endl;
}

/*!
 * 析构函数，没有集成类，不需要声明为虚函数
 */
FuncTraceLog::~FuncTraceLog()
{
    std::cout << "Leave <= " << funclog_FuncName <<
        "[" << funclog_FileName << "]" << std::endl;
}

void LogStrLn(const char* file_name, const char* func_name, int line_no, const char* str)
{
    std::cout << func_name << " : [" <<
        file_name << ':' << line_no << ']' << str << std::endl;
}
