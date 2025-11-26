#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void onConnect();
    void onDisconnect();
    void onSendData();
    void onConnected();
    void onDisconnectedFromServer();
    void onError(QAbstractSocket::SocketError socketError);
    void sendHeartbeat();

private:
    Ui::ClientWindow *ui;
    QTcpSocket *tcpSocket;
    QTimer *heartbeatTimer;

    void logMessage(const QString &message);
    void updateConnectionStatus(bool connected);
    bool validateInput();
};

#endif // CLIENTWINDOW_H
