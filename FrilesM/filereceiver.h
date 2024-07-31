#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <QWidget>
#include <QObject>
#include <QString>
#include <QFile>
#include <QtDebug>
#include <QTime>
#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QtNetwork/QTcpSocket>
#include <myQLib_FilesAndDirs>
#include <QDesktopServices>

#include "baseinfo.h"

#include <QFtp>  // Include QFtp header

namespace Ui {
class FileReceiver;
}

class FileReceiver : public QWidget
{
    Q_OBJECT

public:
    enum Mode { Open = 0, Download };
    explicit FileReceiver(Mode mode, QString cloudDir, QString localDir, QString fileName, QWidget *parent = 0);
    ~FileReceiver();

private slots:
    void ftpCommandFinished(int, bool error);
    void ftpDataTransferProgress(qint64 bytesRead, qint64 totalBytes);
    void ftpDone(bool error);
    void ftpError();

private:
    Ui::FileReceiver *ui;
    Mode nMode;
    QFtp *ftp;
    QString nlocalDir;
    QString localPath;
    QFile *file;
    FilesAndDirs fileanddirs;
    // 声明一个成员变量，用于存储上一次进度更新的时间戳
    QDateTime lastProgressTime;
    qint64 flag = 0;
    // 声明一个成员变量，用于存储上一次进度更新时已上传的字节数
    qint64 lastReceivedBytes = 0;
};

#endif // FILERECEIVER_H
