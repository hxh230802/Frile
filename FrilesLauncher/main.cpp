#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>
#include <QTcpSocket>
#include <QDesktopServices>
#include <QUrl>

#include <myQLib_ProcessManager>

bool isProcessRunning(const QString& processName) {
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");

    if (process.waitForStarted() && process.waitForFinished()) {
        QByteArray output = process.readAll();

        const QString utf8Output = QString::fromLocal8Bit(output);
        QStringList processList = utf8Output.split("\n");

        for (const QString& process : processList) {
            if (process.contains(processName))
                return true;
        }
    }

    return false;
}

void sendTcpMessage() {
    const QString host = "127.0.0.1";
    const quint16 port = 6020;

    QTcpSocket socket;
    socket.connectToHost(host, port);

    if (socket.waitForConnected()) {
        const QString message = "show";
        socket.write(message.toUtf8());
        socket.waitForBytesWritten();
        socket.waitForReadyRead();
        const QByteArray response = socket.readAll();
        qDebug() << "Received server response:" << response;
    } else {
        QMessageBox::critical(nullptr, "错误", "无法连接到本地服务器（代码：1）");
    }

    socket.disconnectFromHost();
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const QString processName = "FrilesM.exe";
    if (!isProcessRunning(processName)) {
        const QString filePath = QCoreApplication::applicationDirPath() + '/' + processName;
        QUrl programUrl = QUrl::fromLocalFile(filePath);
        QDesktopServices::openUrl(programUrl);
    }
    else
        sendTcpMessage();

    return 0;
}
