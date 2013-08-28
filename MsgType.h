#pragma once
#include <QString>
// ��������ַ�Ͷ˿ںŶ���
#define SRV_TCP_MAIN_PORT		4000				// �����������ӵĶ˿ں�
#define SRV_TCP_HOLE_PORT		8000				// �����������ӵĶ˿ں�
#define NET_BUFFER_SIZE			1024				// �����С
// ���ݰ�����
typedef enum _packet_type
{
    PACKET_TYPE_INVALID = 0,
    PACKET_TYPE_NEW_USER_LOGIN,			// �������յ��µĿͻ��˵�¼������¼��Ϣ���͸������ͻ���
    PACKET_TYPE_WELCOME,				// �ͻ��˵�¼ʱ���������͸û�ӭ��Ϣ���ͻ��ˣ��Ը�֪�ͻ��˵�¼�ɹ�
    PACKET_TYPE_REQUEST_CONN_CLIENT,	// ĳ�ͻ�������������룬Ҫ������һ���ͻ��˽���ֱ�ӵ�TCP���ӣ�����Ҫ����TCP��
    PACKET_TYPE_REQUEST_MAKE_HOLE,		// ����������ĳ�ͻ�������һ�ͻ��˽���TCP�򶴣�������һ�ͻ���ָ�����ⲿIP�Ͷ˿ںŽ���connect����
    PACKET_TYPE_REQUEST_DISCONNECT,		// ����������Ͽ�����
    PACKET_TYPE_TCP_DIRECT_CONNECT,		// ������Ҫ�������ˣ��ͻ���A��ֱ�����ӱ����ˣ��ͻ���B�����ⲿIP�Ͷ˿ں�
    PACKET_TYPE_HOLE_LISTEN_READY,		// �����ˣ��ͻ���B���򶴺���������׼������
    PACKET_TYPE_Logon,
    PACKET_TYPE_UserList,
    PACKET_TYPE_CONNECT_ROUTE_PORT_ERROR,
    PACKET_TYPE_CONNECT_PC_PORT_ERROR,
    PACKET_TYPE_MAX
} PACKET_TYPE;

struct ST_Msg
{
    //��Ϣ����
    PACKET_TYPE type;
    //��ǰ�û�
    QString user;
    //Ŀ���û�
    QString to_user;
    //Զ��IP
    QString remote_ip;
    //����IP
    QString local_ip;
    //·�������ڶ˿�
    quint16 route_out_port;
    //·������ڶ˿�
    quint16 route_in_port;
    //PC�˿�
    quint16 remote_port;
};
