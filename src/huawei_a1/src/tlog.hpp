#ifndef TLOG_HPP
#define TLOG_HPP

#include <string>

class FuncTraceLog
{
private:
    //! 文件名
    std::string funclog_FileName;   

    //! 函数名
    std::string funclog_FuncName;

    //! 行号
    int funclog_LineNo;

public:
    /*!
     * 构造函数
     */
    FuncTraceLog(const char* file_name, const char* func_name, int line_no); 

    /*!
     * 析构函数，没有集成类，不需要声明为虚函数
     */
    ~FuncTraceLog();
};

void LogStrLn(const char* file_name, const char* func_name, int line_no, const char* str);

#if 0

#define FUNC_TRACE_LOG FuncTraceLog __log__(__FILE__, __func__, __LINE__)
#define LOG_STR_LN(STR) LogStrLn(__FILE__, __func__, __LINE__, STR)

#else 

#define FUNC_TRACE_LOG 
#define LOG_STR_LN(STR) 

#endif

#endif //TLOG_HPP

