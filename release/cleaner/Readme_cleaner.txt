1. clean_data_pretreat: pretreat数据目录清理程序
pretreat数据目录用来存放解析生成的csv文件，以对应的入库表名称为单位按目录存放，csv文件入库后，其存放的目录不会被入库程序删除，所有必须有一个机制来定时删除这些临时的目录，clean_data_pretreat程序就完成这一清理工作的。通过start_clean_data_pretreat来启动clean_data_pretreat程序，完成一次清理。

（1）目录结构说明
	./log	程序日志目录
  
（2）程序调用参数
	clean_data_pretreat [-r run_path] [-p pretreat_data_path] [-s save_day] [-d]
			-r program running path, default is ./
			-p pretreat data path, default is ./data/pretreat
			-s save days of the pretreat data, default is 3 days
			-d enable debug mode
  
