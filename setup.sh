#!/bin/csh

#build the source
set CUR_DIR=`pwd`
cd $CUR_DIR/src
chmod +x build_all
build_all
cd $CUR_DIR

#mkdir tmp dir
mkdir -p tmp

#subsitute variable
perl parameter.pl -q -s ./release/cleaner/start_clean_data_pretreat -d tmp/start_clean_data_pretreat
set BILL_PARSER_HOME=`perl read_cfg_attr.pl -c setup.cfg -t Config -a BILL_PARSER_HOME`

perl parameter.pl -s ./release/collect/start_collect -d tmp/start_collect
perl parameter.pl -s ./release/pretreat/start_pretreat -d tmp/start_pretreat
perl parameter.pl -s ./release/insert/start_insert -d tmp/start_insert
perl parameter.pl -s ./release/controller/start_controller -d tmp/start_controller
perl parameter.pl -s ./release/log_parser/start_log_parser -d tmp/start_log_parser
perl parameter.pl -s ./release/log_parser/start_log_parser_realtime -d tmp/start_log_parser_realtime
perl parameter.pl -s ./release/recollector/start_recollector -d tmp/start_recollector

#overwrite the old scripts
mv -f tmp/start_clean_data_pretreat ./release/cleaner/start_clean_data_pretreat
mv -f tmp/start_collect ./release/collect/start_collect
mv -f tmp/start_pretreat ./release/pretreat/start_pretreat
mv -f tmp/start_insert ./release/insert/start_insert
mv -f tmp/start_controller ./release/controller/start_controller
mv -f tmp/start_log_parser ./release/log_parser/start_log_parser
mv -f tmp/start_log_parser_realtime ./release/log_parser/start_log_parser_realtime
mv -f tmp/start_recollector ./release/recollector/start_recollector

#copy file
chmod +x copy.csh
copy.csh $BILL_PARSER_HOME

#add crontab info to current user crontab list
crontab (crontab -l >> crontab.txt)

