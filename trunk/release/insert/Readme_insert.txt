insert: ���������򣬰ѻ��������������ɵ�csv�ļ���⡣

1.Ŀ¼�ṹ˵��
  ./           ����Ŀ¼
  ./work       ����������е���ʱ�ļ�
  ./log        ������־Ŀ¼
  ./conf       ��������Ŀ¼
  ./template   ctl�ļ�ģ��Ŀ¼

2.�������˵��
  insert [-i csv_input_path] [-r run_path] [-p parallel_child_number] 
         [-u db_user] [-w db_password] [-s db_server] [-d]
         -i csv_input_path: CSV�ļ�������Ŀ¼��Ĭ��Ϊ./in
  	 -r run_path: �����������Ŀ¼��Ĭ��Ϊ./
  	 -p parallel_child_number: ���������̵������Ŀ��Ĭ��Ϊ1
  	 -u db_user: ���ݿ��¼�û�
  	 -w db_password: ���ݿ��¼����
  	 -s db_server: ���ݿ��ʶ��
  	 -d ����������ģʽ

3.�����ļ�˵��
��1��./conf/table.conf: ����ǰ׺�����ļ�
��2��./template/op_bill_xxx.template������sqlldr��controlģ���ļ�

4.��־�ļ�˵��
��1��������־
�ļ���insert_run.%��Ԫ����%.%����%
���ݣ�csv�ļ��� ������ ��ʼ���ʱ�� ������ʱ��
ʾ����000189_nsn_jxigs12_mtc_20100627234945_CF1457.csv  op_bill_nsn_mtc_jxigs12_0627 20100627235959 20100628000000

��2��������־
�ļ���insert_err.%�����̺�%
���ݣ�
   --------------------------------------------------------------------------------------
   ����ʱ��         Wed May 19 11:16:16 2010
   ������Ϣ         main: clear file fail:./work/010_dir_tmp

��3��sqlldr������־����sqlldr�Զ�����



