1.目录结构
  ./           程序目录
  ./work       文件临时目录 程序运行时存放未完成的文件
  ./log        程序日志目录
  ./conf       程序配置目录

2.文件命名规则
  a.提交文件名为：原文件名.pre
  b.日志文件分为，运行日志(pretreat_run.YYYYMMDD) 和 错误日志(pretreat_err.log.xxx)xxx为子进程编号
  c.程序名为：pretreat

  
3.程序参数
  pretreat  [-o file commit path] [-i file from path] [-r run path] [-p parallel child number] [-d]
  	       -o changes the file commit directory to that specified in path, default is ./out
  	       -i changes the file from directory to that specified in path, default is ./in
  	       -r changes the run directory to that specified in path, default is ./
  	       -p changes the parallel child number, default is 1
  	       -d debug specified

4.提交文件
  a.link工作目录文件至提交目录
  b.记录pretreat_run.YYYYMMDD文件
  c.清理work目录下,xxx_ 开头文件,xxx为子进程编号
  d.unlink原文件

5.临时文件名
----------------------------------------------------------------------------------------------------------------------
预处理过程中
     临时文件名为：子进程编号(3位数字)_文件名


6.文件格式
运行日志
说明：
文件名 开始时间 结束时间 输入文件大小 输出文件大小 记录条数

7.配置文件

  (a) ./conf/module.conf
      格式:
           预处理模块编号 动态库路径名 函数名称
      要求:     
           #开头的行不处理(第一个字符是#)
           行号必须和模块编号一致
      示例:
           1 ./conf/siemens.so siemens
           2 ./conf/huawei.so  huawei
  
  (b) ./conf/pretreat.conf
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
           
  