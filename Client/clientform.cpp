#include <winsock.h>
#include "../MsgType.h"
#include "clientform.h"
#include "ui_clientform.h"

extern char* sz_msg[PACKET_TYPE_MAX] = {    "PACKET_TYPE_INVALID",
                                     "PACKET_TYPE_NEW_USER_LOGIN",			// �������յ��µĿͻ��˵�¼������¼��Ϣ���͸������ͻ���
                                     "PACKET_TYPE_WELCOME",				// �ͻ��˵�¼ʱ���������͸û�ӭ��Ϣ���ͻ��ˣ��Ը�֪�ͻ��˵�¼�ɹ�
                                     "PACKET_TYPE_REQUEST_CONN_CLIENT",	// ĳ�ͻ�������������룬Ҫ������һ���ͻ��˽���ֱ�ӵ�TCP���ӣ�����Ҫ����TCP��
                                     "PACKET_TYPE_REQUEST_MAKE_HOLE",		// ����������ĳ�ͻ�������һ�ͻ��˽���TCP�򶴣�������һ�ͻ���ָ�����ⲿIP�Ͷ˿ںŽ���connect����
                                     "PACKET_TYPE_REQUEST_DISCONNECT",		// ����������Ͽ�����
                                     "PACKET_TYPE_TCP_DIRECT_CONNECT",		// ������Ҫ�������ˣ��ͻ���A��ֱ�����ӱ����ˣ��ͻ���B�����ⲿIP�Ͷ˿ں�
                                     "PACKET_TYPE_HOLE_LISTEN_READY",		// �����ˣ��ͻ���B���򶴺���������׼������
                                     "PACKET_TYPE_Logon",
                                     "PACKET_TYPE_UserList"};

ClientForm::ClientForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientForm)
{
    ui->setupUi(this);
    connect(ui->Connect, SIGNAL(clicked()), this, SLOT(ON_Connect()));
    server_socket_ = new QTcpSocket(this);
    connect(server_socket_, SIGNAL(readyRead()), this, SLOT(ON_ReadyRead()));
    connect(server_socket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(ON_SokcetError(QAbstractSocket::SocketError)));
    user_socket_ = new QTcpSocket(this);
    connect(user_socket_, SIGNAL(readyRead()), this, SLOT(ON_ReadyRead()));
    connect(user_socket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(ON_SokcetError(QAbstractSocket::SocketError)));
    mode_ = new QStandardItemModel(this);
    hole_socket_ = new QTcpServer(this);
    QStringList headers;
    headers << "User" << "IP" << "PORT";
    mode_->setHorizontalHeaderLabels(headers);
    ui->UserTab->setModel(mode_);
    ui->UserTab->setSelectionBehavior(QAbstractItemView::SelectRows);
}

ClientForm::~ClientForm()
{
    delete ui;
}

void ClientForm::ON_Connect()
{
    server_socket_->abort();
    server_socket_->connectToHost(ui->Server->text(), ui->Port->value());
    if (server_socket_->waitForConnected()) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << PACKET_TYPE_Logon;
        stream << ui->User->text();
        server_socket_->write(data);
    }
}

void ClientForm::ON_Send()
{
}

void ClientForm::ON_ReadyRead()
{
    qDebug() << "ClientForm::ON_ReadyRead()";
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (NULL != socket){
        QByteArray data = socket->readAll();
        QDataStream stream(&data, QIODevice::ReadOnly);
        int type;
        stream >> type;
        qDebug() << "type" << sz_msg[type];
        switch ( type )
        {
            case PACKET_TYPE_WELCOME:
            {
                // �յ��������Ļ�ӭ��Ϣ��˵����¼�Ѿ��ɹ�
                ui->Msg->append("log in!");
                break;
            }
            case PACKET_TYPE_REQUEST_CONN_CLIENT: {
                //�յ������ͻ���Ҫ��򶴵�����
                MakeHole(stream);
            }
            case PACKET_TYPE_NEW_USER_LOGIN:
            {
                // �����ͻ��ˣ��ͻ���B����¼����������
                break;
            }
            case PACKET_TYPE_HOLE_LISTEN_READY: {
                //�Է���׼����
                HandleListenReady(stream);
                break;
            }
            case PACKET_TYPE_REQUEST_MAKE_HOLE:
            {
                // ������Ҫ�ң��ͻ���B��������һ���ͻ��ˣ��ͻ���A����
                break;
            }
            case PACKET_TYPE_UserList:
            {
                GetUserList(stream);
                break;
            }
        }
    }
}

void ClientForm::HandleListenReady(QDataStream &stream)
{
    ST_Msg msg;
    DecodeHoleMsg(msg, stream);
    user_socket_->abort();
    user_socket_->connectToHost(msg.remote_ip, msg.remote_port);
    if (user_socket_->waitForConnected()) {
        //�򶴳ɹ�
//        user_socket_->
    }
}

void ClientForm::MakeHole(QDataStream &stream)
{
    ST_Msg msg;
    DecodeHoleMsg(msg, stream);
    //��Ŀ��˿ڷ���һ����Ϣ���ñ��˵�NAT�豸��¼
    user_socket_->connectToHost(msg.remote_ip, msg.route_out_port);
    if (user_socket_->waitForConnected()) {
        //���ӳɹ�����ʾ�����
        user_socket_->write("Hello i'm");
        user_socket_->write(ui->User->text().toUtf8().data());
    } else {
        //����ʧ�ܣ��Է���NAT�豸Ϊ�Գ�NAT
        //���ӵ���������������˵�NAT�豸���ǶԳ�NAT�豸������һ���Ŀͻ������ӹ�����ʱ���ǿ������ӳɹ���
        msg.type = PACKET_TYPE_HOLE_LISTEN_READY;
        QString to_user = msg.to_user;
        msg.to_user = msg.user;
        msg.user = to_user;
        QByteArray data;
        EncodeCennectHoleMsg(msg, data);
        user_socket_->connectToHost(ui->Server->text(), ui->Port->value());
        if (user_socket_->waitForConnected()) {
            int flag = 1;
            setsockopt(user_socket_->socketDescriptor(),
                SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
            hole_socket_->listen(QHostAddress::Any, user_socket_->localPort());
            user_socket_->write(data);
        }
    }
}

void ClientForm::ON_SokcetError(QAbstractSocket::SocketError error)
{
}

void ClientForm::SendMsg(QModelIndex &index)
{
    hole_socket_->abort();
    hole_socket_->connectToHost(ui->Server->text(), SRV_TCP_HOLE_PORT);
    if (hole_socket_->waitForConnected()) {
        int flag = 1;
        setsockopt(hole_socket_->socketDescriptor(), SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
        ST_Msg msg;
        msg.type = PACKET_TYPE_REQUEST_CONN_CLIENT;
        msg.user = ui->User->text();
        msg.route_out_port = ui->OutPort->value();
        msg.route_in_port = ui->InPort->value();
        msg.to_user = mode_->data(mode_->index(index.row(), 0)).toString();
        QByteArray send_data;
        EncodeCennectHoleMsg(msg, send_data);
        hole_socket_->write(send_data);
    } else {
        qDebug() << "SendMsg" << hole_socket_->errorString();
    }
}

void ClientForm::DecodeHoleMsg(ST_Msg &msg, QDataStream &stream)
{
    stream >> msg.user;
    stream >> msg.to_user;
    stream >> msg.remote_ip;
    stream >> msg.local_ip;
    stream >> msg.route_out_port;
    stream >> msg.route_in_port;
    stream >> msg.remote_port;
}

void ClientForm::EncodeCennectHoleMsg(ST_Msg &msg, QByteArray &data)
{
    QDataStream send_stream(&data, QIODevice::WriteOnly);
    send_stream << msg.type;
    send_stream << msg.user;
    send_stream << msg.to_user;
    send_stream << msg.route_out_port;
    send_stream << msg.route_in_port;
    send_stream << msg.remote_port;
}

void ClientForm::GetUserList(QDataStream& stream)
{
    mode_->clear();
    QStringList ips;
    QList<quint16> ports;
    QStringList users;
    stream >> users;
    stream >> ips;
    stream >> ports;
    qDebug() << "users" << users << "ips" << ips << "ports" << ports;
    QStringList headers;
    headers << "User" << "IP" << "PORT";
    mode_->setHorizontalHeaderLabels(headers);
    mode_->setRowCount(users.size());
    for (int i = 0; i < users.size(); ++i) {
        mode_->setData(mode_->index(i, 0), users[i]);
        mode_->setData(mode_->index(i, 1), ips[i]);
        mode_->setData(mode_->index(i, 2), ports[i]);
    }
//    mode_->setRowCount(users.size());

    ui->UserTab->resizeColumnsToContents ();
    ui->UserTab->resizeRowsToContents ();

    QHeaderView *pTableHeaderView = ui->UserTab->horizontalHeader();
    if (pTableHeaderView) {
        pTableHeaderView->setDefaultAlignment(Qt::AlignCenter); //����
        pTableHeaderView->setTextElideMode(Qt::ElideRight); //...Ч��
        pTableHeaderView->setStretchLastSection(true); //β�����հ�
    }
    ui->UserTab->setUpdatesEnabled(true);
}
