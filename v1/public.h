#pragma once

#define LOG(str) \
	cout << __FILE__ << ":" << __LINE__ << " " << \
	__TIMESTAMP__ << " : " << str << endl;
/*
	__FILE__��Ԥ����꣬����ǰԴ�ļ���
	__LINE__��Ԥ����꣬����ǰ�к�
	__TIMESTAMP__��Ԥ����꣬����Դ�ļ�����޸ĵ����ں�ʱ��
*/
