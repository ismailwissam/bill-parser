insert: ���������򣬰ѻ��������������ɵ�csv�ļ���⡣

1.Ŀ¼�ṹ˵��
  ./           ����Ŀ¼
  ./work       ����������е���ʱ�ļ�
  ./log        ������־Ŀ¼
  ./conf       ��������Ŀ¼
  ./template   ctl�ļ�ģ��Ŀ¼

2.�������˵��
  insert [-r run path] [-p parallel child number] [-u db_user] [-w db_password] [-s db_server] [-d]
  	       -r changes the run directory to that specified in path, default is ./
  	       -p changes the parallel child number, default is 1
  	       -u specify db user
  	       -w specify db password
  	       -s specify db server
  	       -d debug specified

4.������
  a.����sqlldr�ύָ����csv�ļ����
  b.�Ѹ�csv�ļ���¼��insert_run.YYYYMMDD�ļ�
  d.ɾ����csv�ļ�������ѡ�񱸷ݣ�

5.�ļ���ʽ
(1)������־
˵����
�ļ��� ��ʼʱ�� ����ʱ�� ��¼����

(2)sqlldr������־����sqlldr�Զ�����

(3)������־
˵��:
  ����ʱ��
  ������Ϣ

6.�����ļ�
 �ɳ���ͨ��ģ���Զ�����.
  
7.���к�ֹͣ����

ʹ��sh start_insert�������г���,sh stop_insert����ֹͣ����.���в����Ѿ��������.

