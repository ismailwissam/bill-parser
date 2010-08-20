log_parser: 日志解析程序，用来对采集程序、解析程序和入库程序产生的日志进行分析整理入库，以方便验证整个话单采集、解析、入库过程的数据完整性。

1.目录结构说明
  ./           程序运行目录
  ./work       临时工作目录，用来存放解析过程中产生的临时文件
  ./log        程序日志目录，用来存放解析过程中产生的日志文件
  ./template   入库模版目录，用来存放sqlldr调用的control文件生成模版

2.程序参数说明
  log_parser -c collect_log_path -p pretreat_log_path -i insert_log_path [-r run_path] [-d parse_date]
  	     -r run_path: 程序运行目录，默认为./
  	     -c collect_log_path: 采集日志目录，该参数必须指定
  	     -p pretreat_log_path: 解析日志目录，该参数必须指定
  	     -i insert_log_path: 入库日志目录，该参数必须指定
  	     -d parse_date: 设置要解析哪一天的日志，格式为yyyymmdd，如果不设置，则默认解析前一天的日志
