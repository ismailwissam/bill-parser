pretreat: 话单解析程序，对话单采集程序采集到的话单文件进行解析，生成csv文件，提供给后续的话单入库程序入库。

1.目录结构说明
  ./           程序目录
  ./work       文件临时目录，程序运行时存放未完成的文件
  ./log        程序日志目录，用来存放解析产生的日志文件
  ./conf       程序配置目录，用来存放解析程序的配置文件
  
2.程序参数说明
  pretreat [-i bill_file_input_path] [-o csv_file_output_path] [-b csv_file_backup_path]
           [-r run_path] [-p parallel_child_number] [-d]
  	   -i bill_file_input_path: 话单文件的输入路径，默认为./in
  	   -o csv_file_output_path: csv文件的输入路径，默认为./out
  	   -b csv_file_backup_path: csv文件的备份目录，默认为./backup
  	   -r run_path: 程序的运行目录，默认为./
  	   -p parallel_child_number: 设置解析进程的最大数目，默认为1
  	   -d 启动调试模式

3.配置文件说明

（1） ./conf/module.conf
      格式:
           预处理模块编号 动态库路径名 函数名称
      要求:     
           #开头的行不处理(第一个字符是#)
           行号必须和模块编号一致
      示例:
           1 ./conf/siemens.so siemens
           2 ./conf/huawei.so  huawei
  
（2） ./conf/pretreat.conf
      格式:
           采集点编号 使用的预处理模块编号
      要求:     
           #开头的行不处理(第一个字符是#)
           行号必须和采集点编号一致(文件名前6位数字)
      示例:
           <000001> 1
           <000002> 1
           <000003> 1
           <000004> 1
           <000005> 1
           <000006> 1
           <000007> 1
           <000008> 1
           <000009> 1
           <000010> 1
           <000011> 2
           <000012> 2
           
4. 日志文件说明
（1）运行日志
文件：pretreat_run.%网元名称%.%日期%
内容：文件名 文件大小 解析开始时间 解析结束时间 解析话单条数
示例：000212_nsn_huzgs7_20100522162536_CF2059.DAT 760368 20100527173803 20100527173803  2320	

（2）错误日志
文件：pretreat.err.%解析进程号%
内容:    --------------------------------------------------------
  	发生时间         Wed May 19 11:16:16 2010
  	错误信息         main: clear file fail:./work/010_dir_tmp


