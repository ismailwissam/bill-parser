#!/bin/csh
#安装时调用的系统命令,主要完成文件COPY操作
#用法: copy.csh <bill_parser_home_dir>

set BILL_PARSER_HOME=$1
set LOG="tmp/install.log"

#记录安装日志
set date=`date '+%Y%m%d%H%M'`
set INSTALL_PATH=`pwd`
touch $LOG
mkdir -p "tmp";

echo "[ $date ] Install bill parser package" >>$LOG 
echo "[ $date ] Install bill parser package" 

#copy backup dir
if ( -d $BILL_PARSER_HOME/backup ) rm -rf $BILL_PARSER_HOME/backup
/bin/cp -r $INSTALL_PATH/backup $BILL_PARSER_HOME

#copy data dir
if ( -d $BILL_PARSER_HOME/data ) rm -rf $BILL_PARSER_HOME/data
/bin/cp -r $INSTALL_PATH/data $BILL_PARSER_HOME

#copy release dir
if ( -d $BILL_PARSER_HOME/release ) rm -rf $BILL_PARSER_HOME/release
/bin/cp -r $INSTALL_PATH/release $BILL_PARSER_HOME

# chmod excutable flag of scripts
chmod +x $BILL_PARSER_HOME/release/start_all
chmod +x $BILL_PARSER_HOME/release/stop_all
chmod +x $BILL_PARSER_HOME/release/cleaner/start_clean_data_pretreat
chmod +x $BILL_PARSER_HOME/release/collect/start_collect
chmod +x $BILL_PARSER_HOME/release/collect/stop_collect
chmod +x $BILL_PARSER_HOME/release/controller/start_controller
chmod +x $BILL_PARSER_HOME/release/controller/stop_controller
chmod +x $BILL_PARSER_HOME/release/insert/start_insert
chmod +x $BILL_PARSER_HOME/release/insert/stop_insert
chmod +x $BILL_PARSER_HOME/release/log_parser/start_log_parser
chmod +x $BILL_PARSER_HOME/release/log_parser/start_log_parser_realtime
chmod +x $BILL_PARSER_HOME/release/log_parser/stop_log_parser
chmod +x $BILL_PARSER_HOME/release/pretreat/start_pretreat
chmod +x $BILL_PARSER_HOME/release/pretreat/stop_pretreat
chmod +x $BILL_PARSER_HOME/release/recollector/start_recollector
chmod +x $BILL_PARSER_HOME/release/recollector/stop_recollector

