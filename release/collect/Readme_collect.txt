collect: �����ɼ����򣬴����õ�Ftp�����������ػ����ļ������ء�

1.Ŀ¼�ṹ˵��
  ./           ��������Ŀ¼
  ./work       �ļ���ʱĿ¼ ��������ʱ���δ��ɵ��ļ�
  ./log        ������־Ŀ¼
  ./conf       ��������Ŀ¼ 

2.������ò���
  collect  [-o bill_commit_path] [-r run_path] [-R recollect_run_path] 
		   [-p parallel_child_number] [-m adjust_minute] [-d]
  	       -o bill commit path, default is ./data/collect
  	       -r collect program run path, default is ./
		   -R recollect program run path, must not set
  	       -p the parallel child number, default is 1
		   -m adjust minuate, default is 0
  	       -d enable debug mode
		   
3.�����ļ�˵��
��1��./conf/collect.conf �ɼ�����
���ø�ʽ��
�ɼ����� ���� �豸 ip usr password  �ɼ���Ŀ¼�� �ɼ����ļ���ƥ���ʽ ���ر���Ŀ¼�� �Ƿ��ύ �Ƿ���һ���ļ� 

����ʾ����
<000001> 130.10.10.218 mscbill mscbill /CDB/buffer/dbill * /backup4 yes yes
#<000002> 130.10.10.110 mscbill mscbill  ../buffer/* * /msc5 yes yes

ע�����
#��ͷ���в�����(��һ���ַ���#)
�кű���Ͳɼ�����һ��
���ر���Ŀ¼��,������/��β,not(�����ִ�Сд)ʱ������.
�Ƿ��ύֻ��Ϊ: not(�����ִ�Сд)���ύ �� yes(�����ִ�Сд)�ύ
�Ƿ���һ���ļ�ֻ��Ϊ: not(�����ִ�Сд)����� �� yes(�����ִ�Сд)���

�ɼ���Ŀ¼�����ٰ���һ��/,���һ��Ŀ¼��ɰ���(����)һ��*ͨ���,�����ǲɼ���Ŀ¼���ĸ�ʽ
/CDB/buffer/dbill           �ɼ��ļ�Ϊ/CDB/buffer ��dbill��Ŀ¼�е��ļ�
/CDB/buffer/*               �ɼ��ļ�Ϊ/CDB/buffer ��������Ŀ¼�е��ļ�
/CDB/buffer/cdma1*          �ɼ��ļ�Ϊ/CDB/buffer ��������cdma1��ͷ����Ŀ¼�е��ļ�
/CDB/buffer/*cdma1          �ɼ��ļ�Ϊ/CDB/buffer ��������cdma1��β����Ŀ¼�е��ļ�
/CDB/buffer/cdr*gsm         �ɼ��ļ�Ϊ/CDB/buffer ��������cdr��ͷ����gsm��β����Ŀ¼�е��ļ�
./                          �ɼ��ļ�Ϊ./�е��ļ�

�ɼ����ļ���ƥ���ʽ,�������(����)һ��*ͨ���,�����ǲɼ����ļ���ƥ���ʽ
*     �����ļ�
aa*   aa��ͷ���ļ�
*bb   bb��β���ļ�
cc*dd cc��ͷ����dd��β���ļ�

4.��־�ļ�˵��
��1��������־
	�ļ���collect_run.%��Ԫ����%.%����%
	���ݣ��ɼ����� ��Ԫ���� �ɼ�Ŀ¼ �ɼ��ļ� �ļ���С ����ʱ�� �ɼ�ʱ�� �Ƿ񱸷� �Ƿ��ύ
	ʾ���� <000001>  hzgs2 20100629 gzX3KM0740000003972_tdhzgs20834.dat 1276131  20100629104800 20100629114744 1  1

��2��������־
	�ļ���collect_err.%�ɼ����̺�%
	����:
        -------------------------------------------------------------------------------------
	  	����ʱ��         Wed May 19 11:16:16 2010
	  	������Ϣ         main: clear file fail:./work/010_dir_tmp
