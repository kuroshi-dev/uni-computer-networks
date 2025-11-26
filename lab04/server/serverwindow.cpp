#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QNetworkInterface>
#include <QMessageBox>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ServerWindow), tcpServer(nullptr)
{
    ui->setupUi(this);

    ui->portSpinBox->setValue(5555);
    ui->portSpinBox->setRange(1024, 65535);

    monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, &ServerWindow::updateClientList);

    connect(ui->startButton, &QPushButton::clicked, this, &ServerWindow::onStartServer);
    connect(ui->stopButton, &QPushButton::clicked, this, &ServerWindow::onStopServer);

    ui->stopButton->setEnabled(false);

    logMessage("Server ready to start");
}

ServerWindow::~ServerWindow()
{
    if (tcpServer && tcpServer->isListening())
    {
        tcpServer->close();
    }
    delete ui;
}

QString ServerWindow::getLocalIP()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress::LocalHost)
        {
            return address.toString();
        }
    }
    return "127.0.0.1";
}

void ServerWindow::onStartServer()
{
    if (!tcpServer)
    {
        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &ServerWindow::onNewConnection);
    }

    quint16 port = ui->portSpinBox->value();

    if (tcpServer->listen(QHostAddress::Any, port))
    {
        logMessage("--- SERVER STARTED ---");
        updateServerInfo();
        monitorTimer->start(3000); // check for clients every 3 seconds

        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->portSpinBox->setEnabled(false);
    }
    else
    {
        QMessageBox::critical(this, "Error:",
                              "Cannot start server: " + tcpServer->errorString());
    }
}

void ServerWindow::onStopServer()
{
    if (tcpServer && tcpServer->isListening())
    {
        monitorTimer->stop();

        foreach (auto client, clients.keys())
        {
            client->disconnectFromHost();
        }
        clients.clear();

        tcpServer->close();
        logMessage("--- SERVER STOPPED ---");

        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->portSpinBox->setEnabled(true);
        ui->serverInfoLabel->clear();
        ui->clientsTextEdit->clear();
    }
}

void ServerWindow::updateServerInfo()
{
    QString info = QString("IP Address: %1\nPort: %2\nStatus: Waiting for connections...")
                       .arg(getLocalIP())
                       .arg(tcpServer->serverPort());
    ui->serverInfoLabel->setText(info);
}

void ServerWindow::onNewConnection()
{
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();

    ClientInfo info;
    info.socket = clientSocket;
    info.ipAddress = clientSocket->peerAddress().toString();
    info.port = clientSocket->peerPort();
    info.lastSeen = QDateTime::currentDateTime();
    info.isOnline = true;

    clients[clientSocket] = info;

    connect(clientSocket, &QTcpSocket::disconnected, this, &ServerWindow::onClientDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead, this, &ServerWindow::onReadyRead);

    logMessage(QString("▶ New client connected: IP: %1, Port: %2, Status: Online")
                   .arg(info.ipAddress)
                   .arg(info.port));

    updateClientList();
}

void ServerWindow::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket)
        return;

    if (clients.contains(clientSocket))
    {
        ClientInfo info = clients[clientSocket];
        logMessage(QString("◀ Client disconnected: IP: %1, Port: %2")
                       .arg(info.ipAddress)
                       .arg(info.port));
        clients.remove(clientSocket);
    }

    clientSocket->deleteLater();
    updateClientList();
}

void ServerWindow::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket)
        return;

    QByteArray data = clientSocket->readAll();
    QString message = QString::fromUtf8(data).trimmed();

    if (clients.contains(clientSocket)) // Update information about client
    {
        clients[clientSocket].lastSeen = QDateTime::currentDateTime();
        clients[clientSocket].isOnline = true;
    }

    if (message == "HEARTBEAT") // Checking for message type
    {
        return; // Message award (keepalive)
    }
    else if (message.startsWith("DATA:"))
    {
        parseDataPacket(message.mid(5), clientSocket); // Information data packet
    }
}

void ServerWindow::parseDataPacket(const QString &data, QTcpSocket *client)
{
    QStringList fields = data.split("||");

    if (fields.size() != 5)
    {
        logMessage("❌ Error: Invalid data packet format");
        return;
    }

    QString surname = fields[0];
    QString phone = fields[1];
    int age = fields[2].toInt();
    int specialization = fields[3].toInt();
    QString description = fields[4];

    QString specName;
    switch (specialization)
    {
    case 1:
        specName = "Networks";
        break;
    case 2:
        specName = "Databases";
        break;
    case 3:
        specName = "Cybersecurity";
        break;
    case 4:
        specName = "Cloud Computing";
        break;
    default:
        specName = "Unknown";
    }

    ClientInfo info = clients[client];

    logMessage("\n" + QString("=").repeated(60));
    logMessage("📦 DATA PACKET RECEIVED");
    logMessage(QString("From client: %1:%2").arg(info.ipAddress).arg(info.port));
    logMessage(QString("─").repeated(60));
    logMessage(QString("Field 1 (Surname):       %1").arg(surname));
    logMessage(QString("Field 2 (Phone):         %1").arg(phone));
    logMessage(QString("Field 3 (Age):           %1").arg(age));
    logMessage(QString("Field 4 (Specialization): %1 (%2)").arg(specialization).arg(specName));
    logMessage(QString("Field 5 (Description):   %1").arg(description));
    logMessage(QString("=").repeated(60) + "\n");
}

void ServerWindow::updateClientList()
{
    if (clients.isEmpty())
    {
        ui->clientsTextEdit->setPlainText("No connected clients");
        return;
    }

    QString list = QString("--- CONNECTED CLIENTS LIST (%1) ---\n\n").arg(clients.size());

    int index = 1;
    QDateTime now = QDateTime::currentDateTime();

    foreach (auto client, clients.keys())
    {
        ClientInfo info = clients[client];

        // Activity check (if no communication for more than 15 seconds - Offline)
        qint64 secondsSinceLastSeen = info.lastSeen.secsTo(now);
        bool isActive = secondsSinceLastSeen < 15;

        list += QString("Client #%1:\n").arg(index++);
        list += QString("  IP Address: %1\n").arg(info.ipAddress);
        list += QString("  Port: %1\n").arg(info.port);
        list += QString("  Status: %1\n").arg(isActive ? "🟢 Online" : "🔴 Offline");
        list += QString("  Last seen: %1 sec. ago\n\n")
                    .arg(secondsSinceLastSeen);
    }

    ui->clientsTextEdit->setPlainText(list);
}

void ServerWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}
