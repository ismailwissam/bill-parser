recollect: 补采程序，对正常采集程序没有采集到的话单进行补采，同时进行解析，把解析生成的csv文件放入正常采集解析的csv文件目录中，用统一的入库程序完成入库。

1.目录结构说明
  ./           程序运行目录
  ./work       文件临时目录
  ./log        程序日志目录
  ./conf       程序配置目录

2.程序参数说明
  recollect [-c collect_path] [-p pretreat_path] [-r run_path] [-b csv_backup_path]
            [-x collect_process_max_num] [-y pretreat_process_max_num] [-f] [-d]
  	    -c collect_path: 设置存放话单文件的目录，默认为./data/collect
  	    -p pretreat_path: 设置存放csv文件的目录，默认为./data/pretreat
  	    -r run_path: 设置程序的运行目录，默认为./
  	    -b csv_backup_path: 设置csv文件的备份目录，如果不设置，则不备份csv文件
  	    -x collect_process_max_num: 设置采集进程的最大数目
  	    -y pretreat_process_max_num: 设置解析进程的最大数目
  	    -f : 强制下载模式，如果设置了该参数，即使话单文件之前已经下载，也会重新下载；否则略过
  	    -d : 调试运行模式

3.配置文件说明
（1）./conf/collect.conf : 采集配置文件
采集点编号 厂商 网元名称 IP地址 端口 用户 密码 采集点目录名 采集点文件 名匹配格式 本地备份目录名 是否提交 是否间隔一个文件 采集开始时间 采集结束时间

后面两个是新加配置项，前面的跟正常采集一样

采集开始时间和采集结束时间的格式为：YYYYMMDDhhsssmm（例如：20100720105000）。

（2）./conf/module.conf : 解析模块配置文件
格式:
    预处理模块编号 动态库路径名 函数名称
要求:     
    #开头的行不处理(第一个字符是#)
    行号必须和模块编号一致
示例:
    1 ./conf/nokia.so nokia
    2 ./conf/huawei.so  huawei
    
（3）./conf/pretreat.conf : 解析程序配置文件
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
           
（4）./conf/table.conf : 入库表前缀配置文件

           
  
