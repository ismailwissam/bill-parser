log_parser: ��־�������������Բɼ����򡢽���������������������־���з���������⣬�Է�����֤���������ɼ��������������̵����������ԡ�

1.Ŀ¼�ṹ˵��
  ./           ��������Ŀ¼
  ./work       ��ʱ����Ŀ¼��������Ž��������в�������ʱ�ļ�
  ./log        ������־Ŀ¼��������Ž��������в�������־�ļ�
  ./template   ���ģ��Ŀ¼���������sqlldr���õ�control�ļ�����ģ��

2.�������˵��
  log_parser -c collect_log_path -p pretreat_log_path 
             -i insert_log_path [-r run_path] [-d parse_date]
  	     -r run_path: ��������Ŀ¼��Ĭ��Ϊ./
  	     -c collect_log_path: �ɼ���־Ŀ¼���ò�������ָ��
  	     -p pretreat_log_path: ������־Ŀ¼���ò�������ָ��
  	     -i insert_log_path: �����־Ŀ¼���ò�������ָ��
  	     -d parse_date: ����Ҫ������һ�����־����ʽΪyyyymmdd����������ã���Ĭ�Ͻ���ǰһ�����־
  	     
3. ������Ϣ
��1��./template/sys_bill_xxx.template������sqlldr��control����ļ�ģ��
Ŀǰ���ݿ��������־���ĸ��洢��������Ӧ�����ĸ�controlģ���ļ���
sys_bill_collect_log.template
sys_bill_insert_log.template
sys_bill_log_to_csv.template
sys_bill_pretreat_log.template

ģ���ļ������ݸ�ʽ�������£�
load data
infile "./work/sys_bill_xxx.csv"
append
into table sys_bill_xxx
fields terminated by ","
Optionally enclosed by "'"
trailing nullcols
(
  FIELD_1,
  FIELD_2,
  ......
)

