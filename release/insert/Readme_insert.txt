insert: 话单入库程序，把话单解析程序生成的csv文件入库。

1.目录结构说明
  ./           程序目录
  ./work       存放入库过程中的临时文件
  ./log        程序日志目录
  ./conf       程序配置目录
  ./template   ctl文件模板目录

2.程序参数说明
  insert [-r run path] [-p parallel child number] [-u db_user] [-w db_password] [-s db_server] [-d]
  	       -r changes the run directory to that specified in path, default is ./
  	       -p changes the parallel child number, default is 1
  	       -u specify db user
  	       -w specify db password
  	       -s specify db server
  	       -d debug specified

4.入库操作
  a.调用sqlldr提交指定的csv文件入库
  b.把该csv文件记录到insert_run.YYYYMMDD文件
  d.删除该csv文件（可以选择备份）

5.文件格式
(1)运行日志
说明：
文件名 开始时间 结束时间 记录条数

(2)sqlldr运行日志：由sqlldr自动生成

(3)错误日志
说明:
  发生时间
  错误信息

6.控制文件
 由程序通过模板自动生成.
  
7.运行和停止程序

使用sh start_insert命令运行程序,sh stop_insert命令停止程序.运行参数已经设置完毕.

