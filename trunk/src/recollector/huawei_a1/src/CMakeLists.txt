##################################### 定义部分 #####################################

#
# 设置头文件路径和库文件路径
#
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
LINK_DIRECTORIES("${PROJECT_SOURCE_DIR}/../../../release/recollect/module")

#
# 设置 c++源文件
#
SET(CPP_SOURCES
    "bill_base.cpp"
    "format_base.cpp"
    "interface.cpp"
    "module_base.cpp"
    "tlog.cpp"
    "value_base.cpp"
    "huawei_a1.cpp"
    "hw_asn1.cpp"
    "value.cpp"
    "format.cpp"
    "asn1_bill.cpp"
    "module.cpp")

#
# 设置 目标文件名
#
SET(LIB_NAME "huawei_a1")

##################################### 执行部分 #####################################

#
# 增加 编译目标
#
ADD_LIBRARY(${LIB_NAME} SHARED ${CPP_SOURCES})


