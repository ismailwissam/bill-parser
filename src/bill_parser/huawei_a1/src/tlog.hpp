#ifndef TLOG_HPP
#define TLOG_HPP

#include <string>

class FuncTraceLog
{
private:
    //! �ļ���
    std::string funclog_FileName;   

    //! ������
    std::string funclog_FuncName;

    //! �к�
    int funclog_LineNo;

public:
    /*!
     * ���캯��
     */
    FuncTraceLog(const char* file_name, const char* func_name, int line_no); 

    /*!
     * ����������û�м����࣬����Ҫ����Ϊ�麯��
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

