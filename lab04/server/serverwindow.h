#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

struct ClientInfo {
    QTcpSocket* socket;
    QString ipAddress;
    quint16 port;
    QDateTime lastSeen;
    bool isOnline;
};

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    void updateClientList();
    void onStartServer();
    void onStopServer();

private:
    Ui::ServerWindow *ui;
    QTcpServer *tcpServer;
    QMap<QTcpSocket*, ClientInfo> clients;
    QTimer *monitorTimer;

    void logMessage(const QString &message);
    void updateServerInfo();
    QString getLocalIP();
    void parseDataPacket(const QString &data, QTcpSocket *client);
};

#endif // SERVERWINDOW_H
