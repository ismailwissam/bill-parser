1.Ŀ¼�ṹ
  ./           ����Ŀ¼
  ./work       �ļ���ʱĿ¼ ��������ʱ���δ��ɵ��ļ�
  ./log        ������־Ŀ¼
  ./conf       ��������Ŀ¼

2.�ļ���������
  a.�ύ�ļ���Ϊ��ԭ�ļ���.pre
  b.��־�ļ���Ϊ��������־(pretreat_run.YYYYMMDD) �� ������־(pretreat_err.log.xxx)xxxΪ�ӽ��̱��
  c.������Ϊ��pretreat

  
3.�������
  pretreat  [-o file commit path] [-i file from path] [-r run path] [-p parallel child number] [-d]
  	       -o changes the file commit directory to that specified in path, default is ./out
  	       -i changes the file from directory to that specified in path, default is ./in
  	       -r changes the run directory to that specified in path, default is ./
  	       -p changes the parallel child number, default is 1
  	       -d debug specified

4.�ύ�ļ�
  a.link����Ŀ¼�ļ����ύĿ¼
  b.��¼pretreat_run.YYYYMMDD�ļ�
  c.����workĿ¼��,xxx_ ��ͷ�ļ�,xxxΪ�ӽ��̱��
  d.unlinkԭ�ļ�

5.��ʱ�ļ���
----------------------------------------------------------------------------------------------------------------------
Ԥ���������
     ��ʱ�ļ���Ϊ���ӽ��̱��(3λ����)_�ļ���


6.�ļ���ʽ
������־
˵����
�ļ��� ��ʼʱ�� ����ʱ�� �����ļ���С ����ļ���С ��¼����

7.�����ļ�

  (a) ./conf/module.conf
      ��ʽ:
           Ԥ����ģ���� ��̬��·���� ��������
      Ҫ��:     
           #��ͷ���в�����(��һ���ַ���#)
           �кű����ģ����һ��
      ʾ��:
           1 ./conf/siemens.so siemens
           2 ./conf/huawei.so  huawei
  
  (b) ./conf/pretreat.conf
      ��ʽ:
           �ɼ����� ʹ�õ�Ԥ����ģ����
      Ҫ��:     
           #��ͷ���в�����(��һ���ַ���#)
           �кű���Ͳɼ�����һ��(�ļ���ǰ6λ����)
      ʾ��:
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
           
  