浙江移动话单解析程序zj-bill-parser说明文档

1.程序部署
	host=10.211.106.132
	username=dcadmin
	password=dca$zj2004
	directory=/opt/BOCO.DAL/RT_MON/zj-bill-parser
	
2.程序目录
  ./src        源码目录
  ./release    可执行程序目录
  ./data       程序运行数据存放目录
  ./backup     文件备份目录
  
3.程序简介
	collect模块下载所有采集点数据并备份;
	pretreat模块读取由collect模块下载的话单文件并解析生成csv文件;
	insert模块读取由pretreat模块生成的csv文件并完成入库;
        cleaner模块负责对中间数据进行清理
		
4.collect
	4.1 目录:./release/collect
	4.2 启动:sh start_collect
	4.3 停止:sh stop_collect
	4.4 配置:
		4.4.1
		./release/collect/run/conf/collect.conf
		说明:
		采集点号 厂商 网元       IP     PORT UID     PWD  保留 保留                     备份目录                    固定 保留
		<000001> nsn tzhg04 10.76.33.152 21 CHARGI 123456  ./   * /opt/BOCO.DAL/RT_MON/zj-bill-parser/backup/collect yes yes
		采集的文件备份时保存在 备份目录/厂商/网元 下
		固定值不可修改.
		保留值暂时无用.
		4.4.2
		./release/collect/run/conf/%采集点号%_TimePoint
		说明:保存对应采集点的时间端点,采集程序只采集时间端点后的文件.*不用配置*.
	4.5 日志
		4.5.1 运行日志
		文件:./release/collect/run/log/collect_run.%采集点号%.%日期%
		内容:	采集点号 路径   文件名       采集时间  文件大小 保留 保留
					<000058>   .  CF0007.DAT 20100525155024 678608    1    1
		4.5.2 错误日志
		文件:./release/collect/run/log/collect_err.log.%采集点号%
		内容:
	  			发生时间         Wed May 19 11:16:16 2010
	  			错误信息         main: clear file:./work/010_dir_tmp

5.pretreat
	5.1 目录:./release/pretreat
	5.2 启动:sh start_pretreat
	5.3 停止:sh stop_pretreat
	5.4 配置:
		5.4.1
		./release/pretreat/conf/module.conf
		说明:配置解析模块,*不用配置*.
		5.4.2
		./release/pretreat/conf/pretreat.conf
		说明:配置对应采集点的解析模块.*不用配置*.
	5.5 日志
	5.5.1 运行日志
	文件:./release/pretreat/log/pretreat_run.%日期%
	内容:	                 文件名                      解析开始时间  解析结束时间  文件大小 话单条数
				000212_nsn_huzgs7_20100522162536_CF2059.DAT 20100527173803 20100527173803 760368    2320	
	5.5.2 错误日志
	文件:./release/pretreat/log/pretreat.log.%进程号%
	内容:
	                --------------------------------------------------------
  			发生时间         Wed May 19 11:16:16 2010
  			错误信息         main: clear file fail:./work/010_dir_tmp

6.insert
	6.1 目录:./release/insert
	6.2 启动:sh start_insert
	6.3 停止:sh stop_insert
	6.4 配置:
	./release/insert/run/template
	说明:为各个ctl文件的模板,*不用配置*.
	6.5 日志
		6.5.1 运行日志
		说明：
		文件名 开始时间 结束时间 记录条数
		6.5.2 错误日志
		说明:
	  发生时间
	  错误信息
		6.5.3sqlldr运行日志：由sqlldr自动生成
		
7.cleaner
7.1  clean_data_pretreat
        7.1.1 目录:./release/cleaner/clean_data_pretreat
        7.1.2 启动:sh start_clean_data_pretreat
              该清理模块专用于./data/pretreat目录下的数据清理，每次启动只负责一次清理，所以需要跟系统工具
              crontab结合使用来完成周期性的数据清理工作
        7.1.3 日志
                7.1.3.1 错误日志：记录程序运行过程中由程序本身产生的错误
                7.1.3.2 操作日志：记录程序运行过程中清理的数据信息
                        格式：清理目录名   清理时间    清理状态
                        
8.data
	7.1 ./data/collect:存放collect采集到的数据,pretreat从这里取数据并处理.
	7.2 ./data/pretreat:存放pretreat解析后的csv文件,insert从这里取数据并入库.
	
9.backup:
	8.1 ./backup/collect:存放collect采集到的数据备份,按照/厂商/网元/划分目录.
	8.2 ./backup/pretreat:计划存放csv文件备份,暂无此需求,保留.
	
