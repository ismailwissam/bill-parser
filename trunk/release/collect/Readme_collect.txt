collect: 话单采集程序，从配置的Ftp服务器上下载话单文件到本地。

1.目录结构说明
  ./           程序运行目录
  ./work       文件临时目录 程序运行时存放未完成的文件
  ./log        程序日志目录
  ./conf       程序配置目录 

2.程序调用参数
  collect  [-o bill_commit_path] [-r run_path] [-R recollect_run_path] 
		   [-p parallel_child_number] [-m adjust_minute] [-d]
  	       -o bill commit path, default is ./data/collect
  	       -r collect program run path, default is ./
		   -R recollect program run path, must not set
  	       -p the parallel child number, default is 1
		   -m adjust minuate, default is 0
  	       -d enable debug mode
		   
3.配置文件说明
（1）./conf/collect.conf 采集配置
配置格式：
采集点编号 厂商 设备 ip usr password  采集点目录名 采集点文件名匹配格式 本地备份目录名 是否提交 是否间隔一个文件 

配置示例：
<000001> 130.10.10.218 mscbill mscbill /CDB/buffer/dbill * /backup4 yes yes
#<000002> 130.10.10.110 mscbill mscbill  ../buffer/* * /msc5 yes yes

注意事项：
#开头的行不处理(第一个字符是#)
行号必须和采集点编号一致
本地备份目录名,不能以/结尾,not(不区分大小写)时不备份.
是否提交只能为: not(不区分大小写)不提交 或 yes(不区分大小写)提交
是否间隔一个文件只能为: not(不区分大小写)不间隔 或 yes(不区分大小写)间隔

采集点目录名最少包含一个/,最后一个目录项可包含(或是)一个*通配符,以下是采集点目录名的格式
/CDB/buffer/dbill           采集文件为/CDB/buffer 下dbill子目录中的文件
/CDB/buffer/*               采集文件为/CDB/buffer 下所有子目录中的文件
/CDB/buffer/cdma1*          采集文件为/CDB/buffer 下所有以cdma1开头的子目录中的文件
/CDB/buffer/*cdma1          采集文件为/CDB/buffer 下所有以cdma1结尾的子目录中的文件
/CDB/buffer/cdr*gsm         采集文件为/CDB/buffer 下所有以cdr开头并以gsm结尾的子目录中的文件
./                          采集文件为./中的文件

采集点文件名匹配格式,必须包含(或是)一个*通配符,以下是采集点文件名匹配格式
*     所有文件
aa*   aa开头的文件
*bb   bb结尾的文件
cc*dd cc开头并且dd结尾的文件

4.日志文件说明
（1）运行日志
	文件：collect_run.%网元名称%.%日期%
	内容：采集点编号 网元名称 采集目录 采集文件 文件大小 生成时间 采集时间 是否备份 是否提交
	示例： <000001>  hzgs2 20100629 gzX3KM0740000003972_tdhzgs20834.dat 1276131  20100629104800 20100629114744 1  1

（2）错误日志
	文件：collect_err.%采集进程号%
	内容:
        -------------------------------------------------------------------------------------
	  	发生时间         Wed May 19 11:16:16 2010
	  	错误信息         main: clear file fail:./work/010_dir_tmp
