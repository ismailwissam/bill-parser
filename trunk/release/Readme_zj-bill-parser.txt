�㽭�ƶ�������������zj-bill-parser˵���ĵ�

1.������
	host=10.211.106.132
	username=dcadmin
	password=dca$zj2004
	directory=/opt/BOCO.DAL/RT_MON/zj-bill-parser
	
2.����Ŀ¼
  ./src        Դ��Ŀ¼
  ./release    ��ִ�г���Ŀ¼
  ./data       �����������ݴ��Ŀ¼
  ./backup     �ļ�����Ŀ¼
  
3.������
	collectģ���������вɼ������ݲ�����;
	pretreatģ���ȡ��collectģ�����صĻ����ļ�����������csv�ļ�;
	insertģ���ȡ��pretreatģ�����ɵ�csv�ļ���������;
        cleanerģ�鸺����м����ݽ�������
		
4.collect
	4.1 Ŀ¼:./release/collect
	4.2 ����:sh start_collect
	4.3 ֹͣ:sh stop_collect
	4.4 ����:
		4.4.1
		./release/collect/run/conf/collect.conf
		˵��:
		�ɼ���� ���� ��Ԫ       IP     PORT UID     PWD  ���� ����                     ����Ŀ¼                    �̶� ����
		<000001> nsn tzhg04 10.76.33.152 21 CHARGI 123456  ./   * /opt/BOCO.DAL/RT_MON/zj-bill-parser/backup/collect yes yes
		�ɼ����ļ�����ʱ������ ����Ŀ¼/����/��Ԫ ��
		�̶�ֵ�����޸�.
		����ֵ��ʱ����.
		4.4.2
		./release/collect/run/conf/%�ɼ����%_TimePoint
		˵��:�����Ӧ�ɼ����ʱ��˵�,�ɼ�����ֻ�ɼ�ʱ��˵����ļ�.*��������*.
	4.5 ��־
		4.5.1 ������־
		�ļ�:./release/collect/run/log/collect_run.%�ɼ����%.%����%
		����:	�ɼ���� ·��   �ļ���       �ɼ�ʱ��  �ļ���С ���� ����
					<000058>   .  CF0007.DAT 20100525155024 678608    1    1
		4.5.2 ������־
		�ļ�:./release/collect/run/log/collect_err.log.%�ɼ����%
		����:
	  			����ʱ��         Wed May 19 11:16:16 2010
	  			������Ϣ         main: clear file:./work/010_dir_tmp

5.pretreat
	5.1 Ŀ¼:./release/pretreat
	5.2 ����:sh start_pretreat
	5.3 ֹͣ:sh stop_pretreat
	5.4 ����:
		5.4.1
		./release/pretreat/conf/module.conf
		˵��:���ý���ģ��,*��������*.
		5.4.2
		./release/pretreat/conf/pretreat.conf
		˵��:���ö�Ӧ�ɼ���Ľ���ģ��.*��������*.
	5.5 ��־
	5.5.1 ������־
	�ļ�:./release/pretreat/log/pretreat_run.%����%
	����:	                 �ļ���                      ������ʼʱ��  ��������ʱ��  �ļ���С ��������
				000212_nsn_huzgs7_20100522162536_CF2059.DAT 20100527173803 20100527173803 760368    2320	
	5.5.2 ������־
	�ļ�:./release/pretreat/log/pretreat.log.%���̺�%
	����:
	                --------------------------------------------------------
  			����ʱ��         Wed May 19 11:16:16 2010
  			������Ϣ         main: clear file fail:./work/010_dir_tmp

6.insert
	6.1 Ŀ¼:./release/insert
	6.2 ����:sh start_insert
	6.3 ֹͣ:sh stop_insert
	6.4 ����:
	./release/insert/run/template
	˵��:Ϊ����ctl�ļ���ģ��,*��������*.
	6.5 ��־
		6.5.1 ������־
		˵����
		�ļ��� ��ʼʱ�� ����ʱ�� ��¼����
		6.5.2 ������־
		˵��:
	  ����ʱ��
	  ������Ϣ
		6.5.3sqlldr������־����sqlldr�Զ�����
		
7.cleaner
7.1  clean_data_pretreat
        7.1.1 Ŀ¼:./release/cleaner/clean_data_pretreat
        7.1.2 ����:sh start_clean_data_pretreat
              ������ģ��ר����./data/pretreatĿ¼�µ���������ÿ������ֻ����һ������������Ҫ��ϵͳ����
              crontab���ʹ������������Ե�����������
        7.1.3 ��־
                7.1.3.1 ������־����¼�������й������ɳ���������Ĵ���
                7.1.3.2 ������־����¼�������й����������������Ϣ
                        ��ʽ������Ŀ¼��   ����ʱ��    ����״̬
                        
8.data
	7.1 ./data/collect:���collect�ɼ���������,pretreat������ȡ���ݲ�����.
	7.2 ./data/pretreat:���pretreat�������csv�ļ�,insert������ȡ���ݲ����.
	
9.backup:
	8.1 ./backup/collect:���collect�ɼ��������ݱ���,����/����/��Ԫ/����Ŀ¼.
	8.2 ./backup/pretreat:�ƻ����csv�ļ�����,���޴�����,����.
	
