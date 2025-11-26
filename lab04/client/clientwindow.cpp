#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QMessageBox>
#include <QDateTime>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ClientWindow), tcpSocket(nullptr)
{
    ui->setupUi(this);

    ui->serverIPLineEdit->setText("127.0.0.1");
    ui->portSpinBox->setValue(5555);
    ui->portSpinBox->setRange(1024, 65535);

    ui->ageSpinBox->setRange(18, 100);
    ui->ageSpinBox->setValue(25);

    ui->descriptionTextEdit->setMaximumHeight(100);
    ui->descriptionTextEdit->setPlaceholderText("Maximum 255 characters");

    ui->specializationComboBox->addItem("1 - Networks", 1);
    ui->specializationComboBox->addItem("2 - Databases", 2);
    ui->specializationComboBox->addItem("3 - Cybersecurity", 3);
    ui->specializationComboBox->addItem("4 - Cloud Computing", 4);

    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &ClientWindow::sendHeartbeat);

    connect(ui->connectButton, &QPushButton::clicked, this, &ClientWindow::onConnect);
    connect(ui->disconnectButton, &QPushButton::clicked, this, &ClientWindow::onDisconnect);
    connect(ui->sendButton, &QPushButton::clicked, this, &ClientWindow::onSendData);

    updateConnectionStatus(false);

    logMessage("Client ready for connection");
}

ClientWindow::~ClientWindow()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        tcpSocket->disconnectFromHost();
    }
    delete ui;
}

void ClientWindow::onConnect()
{
    if (!tcpSocket)
    {
        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket, &QTcpSocket::connected, this, &ClientWindow::onConnected);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnectedFromServer);
        connect(tcpSocket, &QTcpSocket::errorOccurred, this, &ClientWindow::onError);
    }

    QString serverIP = ui->serverIPLineEdit->text();
    quint16 port = ui->portSpinBox->value();

    logMessage(QString("Connecting to server %1:%2...").arg(serverIP).arg(port));
    tcpSocket->connectToHost(serverIP, port);
}

void ClientWindow::onDisconnect()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        heartbeatTimer->stop();
        tcpSocket->disconnectFromHost();
        logMessage("Disconnecting from server...");
    }
}

void ClientWindow::onConnected()
{
    logMessage("✅ Connection established!");
    updateConnectionStatus(true);

    // Start heartbeat
    heartbeatTimer->start(5000); // Every 5 seconds
    sendHeartbeat();
}

void ClientWindow::onDisconnectedFromServer()
{
    logMessage("❌ Connection lost");
    heartbeatTimer->stop();
    updateConnectionStatus(false);
}

void ClientWindow::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    logMessage("❌ Error: " + tcpSocket->errorString());
    updateConnectionStatus(false);
}

void ClientWindow::sendHeartbeat()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        tcpSocket->write("HEARTBEAT");
        tcpSocket->flush();
    }
}

bool ClientWindow::validateInput()
{
    if (ui->surnameLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter surname");
        return false;
    }

    if (ui->phoneLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter phone number");
        return false;
    }

    QString description = ui->descriptionTextEdit->toPlainText();
    if (description.length() > 255)
    {
        QMessageBox::warning(this, "Error",
                             QString("Description is too long (%1 characters). Maximum 255 characters.")
                                 .arg(description.length()));
        return false;
    }

    return true;
}

void ClientWindow::onSendData()
{
    if (!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::warning(this, "Error", "No connection to server");
        return;
    }

    if (!validateInput())
    {
        return;
    }

    // Forming information data packet
    QString surname = ui->surnameLineEdit->text().trimmed();
    QString phone = ui->phoneLineEdit->text().trimmed();
    int age = ui->ageSpinBox->value();
    int specialization = ui->specializationComboBox->currentData().toInt();
    QString description = ui->descriptionTextEdit->toPlainText().trimmed();

    // Format: DATA:field1||field2||field3||field4||field5
    QString dataPacket = QString("DATA:%1||%2||%3||%4||%5")
                             .arg(surname)
                             .arg(phone)
                             .arg(age)
                             .arg(specialization)
                             .arg(description);

    tcpSocket->write(dataPacket.toUtf8());
    tcpSocket->flush();

    logMessage("\n" + QString("=").repeated(50));
    logMessage("📤 INFORMATION DATA PACKET SENT:");
    logMessage(QString("─").repeated(50));
    logMessage(QString("Surname:        %1").arg(surname));
    logMessage(QString("Phone:          %1").arg(phone));
    logMessage(QString("Age:            %1").arg(age));
    logMessage(QString("Specialization: %1 (%2)")
                   .arg(specialization)
                   .arg(ui->specializationComboBox->currentText().mid(4)));
    logMessage(QString("Description:    %1").arg(description));
    logMessage(QString("=").repeated(50) + "\n");

    QMessageBox::information(this, "Success", "Data successfully sent to server!");
}

void ClientWindow::updateConnectionStatus(bool connected)
{
    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);
    ui->sendButton->setEnabled(connected);

    ui->serverIPLineEdit->setEnabled(!connected);
    ui->portSpinBox->setEnabled(!connected);

    QString status = connected ? "🟢 Connected (Online)" : "🔴 Disconnected (Offline)";
    ui->statusLabel->setText(status);

    if (connected)
    {
        ui->surnameLineEdit->setEnabled(true);
        ui->phoneLineEdit->setEnabled(true);
        ui->ageSpinBox->setEnabled(true);
        ui->specializationComboBox->setEnabled(true);
        ui->descriptionTextEdit->setEnabled(true);
    }
}

void ClientWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}