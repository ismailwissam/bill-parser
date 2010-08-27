insert: 话单入库程序，把话单解析程序生成的csv文件入库。

1.目录结构说明
  ./           程序目录
  ./work       存放入库过程中的临时文件
  ./log        程序日志目录
  ./conf       程序配置目录
  ./template   ctl文件模板目录

2.程序参数说明
  insert [-i csv_input_path] [-r run_path] [-p parallel_child_number] 
         [-u db_user] [-w db_password] [-s db_server] [-d]
         -i csv_input_path: CSV文件的输入目录，默认为./in
  	 -r run_path: 入库程序的运行目录，默认为./
  	 -p parallel_child_number: 设置入库进程的最大数目，默认为1
  	 -u db_user: 数据库登录用户
  	 -w db_password: 数据库登录密码
  	 -s db_server: 数据库标识符
  	 -d ：开启调试模式

3.配置文件说明
（1）./conf/table.conf: 入库表前缀配置文件
内容形式如下：
op_bill_hw_cfw_hzgs11
op_bill_hw_cfw_hzgs12
op_bill_hw_cfw_hzgs16
op_bill_hw_cfw_hzgs17
op_bill_hw_cfw_hzgs18
op_bill_hw_cfw_hzgs19
op_bill_hw_cfw_hzgs20
op_bill_hw_cfw_hzgs21
op_bill_hw_cfw_hzgs2
op_bill_hw_cfw_hzgs3
......

（2）./template/op_bill_xxx.template：调用sqlldr的control模版文件
目前有9种入库模版：
op_bill_hw_tran.ctl.template
op_bill_hw_cfw.ctl.template  
op_bill_hw.ctl.template      
op_bill_hw_moc.ctl.template  
op_bill_hw_mtc.ctl.template  
op_bill_nsn_forw.ctl.template
op_bill_nsn_moc.ctl.template
op_bill_nsn_mtc.ctl.template
op_bill_nsn_roam.ctl.template

模版文件的内容格式如下：
load data
$DATA_FILE
append
into table $TABLE_NAME
fields terminated by ","
Optionally enclosed by "'"
trailing nullcols
(
  FIELD_1,
  FIELD_2,
  ......
)

4.日志文件说明
（1）运行日志
文件：insert_run.%网元名称%.%日期%
内容：加载csv文件名 入库表名 加载时生成的sqlldr日志文件名 开始加载时间 结束加载时间
示例：000189_nsn_jxigs12_mtc_20100627234945_CF1457.csv  op_bill_nsn_mtc_jxigs12_0627 op_bill_nsn_mtc_jxigs12_0627_20100627235959_sqlldr.log 20100627235959 20100628000000

（2）错误日志
文件：insert_err.%入库进程号%
内容：
   -------------------------------------------------------
   发生时间         Wed May 19 11:16:16 2010
   错误信息         main: clear file fail:./work/010_dir_tmp

（3）sqlldr运行日志：由sqlldr自动生成
文件：入库表名_开始加载时间_sqlldr.log



