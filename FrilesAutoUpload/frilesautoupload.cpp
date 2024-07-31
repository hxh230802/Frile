#include "frilesautoupload.h"


#include <QString>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QMessageBox>

#include <myQLib_FtpManager>
#include <myQLib_ProcessManager>

FrilesAutoUpload::FrilesAutoUpload(QObject *parent) : QObject(parent)
{
    ftpMgr = new FtpManager(SERVER_IP);

    checkForUpdates();

    ProcessManager::killProcess("UptConsole.exe");
}

void FrilesAutoUpload::checkForUpdates()
{
    const QString latestVersion = ftpMgr->readFileContent("Frile/Servers/Version/Friles/Version.txt");

    if (compareWithLatestVersion(latestVersion) == 1) // 有新版本
    {
        update();
    }
    else
    {
        qApp->quit();
        ProcessManager::killProcess("FrilesAutoUpload.exe");
    }
}

/**
     * @brief 比较当前版本和最新版本
     *
     * @param latestVersion 最新版本号
     * @return int 返回比较结果：-1 表示当前版本较旧，1 表示当前版本较新，0 表示版本相同
     */
int FrilesAutoUpload::compareWithLatestVersion(const QString& latestVersion)
{
    QString currentVersion = VERSION;

    // 输出当前版本和最新版本
    qDebug() << "currentVersion:" << currentVersion;
    qDebug() << "latestVersion:" << latestVersion;

    // 比较版本号
    QStringList parts1 = latestVersion.split('.');
    QStringList parts2 = currentVersion.split('.');

    if (parts1.size() != 4 || parts2.size() != 4)
    {
        // 版本号格式不正确，记录错误并返回-1
        qDebug() << "版本号格式不正确";
        return -1;
    }

    for (int i = 0; i < 4; ++i)
    {
        int part1 = parts1.at(i).toInt();
        int part2 = parts2.at(i).toInt();

        if (part1 < part2)
        {
            return -1;  // 最新版本较低，返回-1
        }
        else if (part1 > part2)
        {
            return 1;   // 最新版本较高，返回1
        }
    }

    return 0; // 两个版本号相等，返回0
}


void FrilesAutoUpload::update()
{
    // 获取本地临时目录中的Friles子目录路径
    tempDir = QDir::tempPath() + "/Friles";
    QDir dir(tempDir);
    if (!dir.exists()) {
        dir.mkpath(tempDir); // 创建Friles子目录
    }

    QNetworkReply *reply = ftpMgr->get("Frile/Servers/Version/Friles/AppFile/UptConsole.exe",tempDir+"/UptConsole.exe");

    // 连接信号，以便更新下载进度
    connect(reply, &QNetworkReply::downloadProgress, this, [=](qint64 bytesReceived, qint64 bytesTotal){

        int progress = qRound((bytesReceived * 100.0) / bytesTotal);

    });

    connect(ftpMgr,&FtpManager::downloadFinished_s,this,[=](){
        if(!flag)ftpMgr->get("Frile/Servers/Version/Friles/AppFile/UptScript.upst",tempDir+"/UptScript.upst");
        if(flag) start();
        flag = true;
    });
}

void FrilesAutoUpload::start()
{

    QString program = tempDir + "/UptConsole.exe";
    QStringList arguments;
    arguments << QCoreApplication::applicationDirPath() << "Friles"; // 获取程序所在目录
    QProcess* process = new QProcess;
    process->start(program, arguments);

    qApp->quit();
    ProcessManager::killProcess("FrilesAutoUpload.exe");
}
