#include "filereceiver.h"
#include "ui_filereceiver.h"
#include <QProcess>
#include <myQLib_FtpManager>

QString __ToFtpEncoding(const QString &InputStr)
{
#ifdef Q_OS_WIN
    return QString::fromLatin1(InputStr.toLocal8Bit());
#else
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    if (codec)
    {
        return QString::fromLatin1(codec->fromUnicode(InputStr));
    }
    else
    {
        return QString("");
    }
#endif
}

QString __FromFtpEncoding(const QString &InputStr)
{
#ifdef Q_OS_WIN
    return  QString::fromLocal8Bit(InputStr.toLatin1());
#else
    QTextCodec *codec = QTextCodec::codecForName("gbk");
    if (codec)
    {
        return codec->toUnicode(InputStr.toLatin1());
    }
    else
    {
        return QString("");
    }
#endif
}

FileReceiver::FileReceiver(Mode mode, QString cloudDir, QString localDir, QString fileName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileReceiver)
{
    ui->setupUi(this);

    ui->label->setText(mode == Open ? QString("正在加载：") : QString("正在下载："));
    ui->fileName_lbl->setText(fileName);

    nMode= mode;
    nlocalDir = localDir;
    localPath = localDir + "/" + fileName;
    QString cloudPath = cloudDir + "/" + fileName;

    fileanddirs.createDirectory(localDir);

    cloudPath = cloudPath.replace(QRegExp("/+"), "/");

    qDebug() << localPath << cloudPath;

    // 创建 QFtp 实例
    ftp = new QFtp(this);

    // 连接 ftp 命令完成信号
//    connect(ftp, &QFtp::commandFinished, this, &FileReceiver::ftpCommandFinished);
    connect(ftp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));

    // 连接 ftp 数据传输进度信号
//    connect(ftp, &QFtp::dataTransferProgress, this, &FileReceiver::ftpDataTransferProgress);
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(ftpDataTransferProgress(qint64,qint64)));

    // 连接 ftp 下载完成信号
    connect(ftp, &QFtp::done, this, &FileReceiver::ftpDone);

    // 连接 ftp 错误信号
    connect(ftp, &QFtp::error, this, &FileReceiver::ftpError);

    // 设置 ftp 主机
    ftp->connectToHost(SERVER_IP);

    // 设置 ftp 用户名和密码（如果需要）
    ftp->login();

    // 设置 ftp 工作目录
    ftp->cd(__ToFtpEncoding(cloudDir));

    file = new QFile(localPath);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        return;
    }

    // 开始下载文件
    ftp->get(__ToFtpEncoding(fileName), file);
}

FileReceiver::~FileReceiver()
{
    delete ui;
    delete ftp;
}

void FileReceiver::ftpCommandFinished(int, bool error)
{
    QFtp *ftp = qobject_cast<QFtp *>(sender());
    if (ftp) {
        if (error) {
            qDebug() << "FTP Command Error: " << ftp->errorString();
            // 处理错误逻辑
        }
    }
}

void FileReceiver::ftpDataTransferProgress(qint64 bytesRead, qint64 totalBytes)
{
    if(bytesRead==totalBytes)
    {
        if(nMode==Open)
        {
            file->close();
            QUrl programUrl = QUrl::fromLocalFile(localPath);
            if(!QDesktopServices::openUrl(programUrl))
            {

                // 构建 rundll32 命令行

                    QString command = "rundll32.exe shell32.dll,OpenAs_RunDLL " + localPath;

                    // 创建 QProcess 对象
                    QProcess *process = new QProcess();

                    // 启动外部进程
                    process->start(command);
            }
            close();
        }
        else
        {
            fileanddirs.openExplorer(nlocalDir);
            close();
        }
    }
    if (totalBytes > 0 && flag % 100 == 0)
        {
        flag = 0;
            // 获取当前时间戳
            QDateTime currentTime = QDateTime::currentDateTime();

            // 计算时间增量（毫秒）
            qint64 timeElapsedMs = lastProgressTime.msecsTo(currentTime);

            // 计算已上传的字节数的增量
            qint64 ReceivedBytesDelta = bytesRead - lastReceivedBytes;

            // 计算上传速度（字节/秒）
            qreal ReceivedSpeed = static_cast<qreal>(ReceivedBytesDelta) / static_cast<qreal>(timeElapsedMs) * 1000;

            qDebug() << ReceivedBytesDelta << timeElapsedMs << ReceivedSpeed;
//            !define PRODUCT_RUNASADMIN_DIR_REGKEY "SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"
//            ;设置以管理员身份运行；使用HKCU参数正常！使用HKLM参数无效！
//            WriteRegStr HKCU "${PRODUCT_RUNASADMIN_DIR_REGKEY}" "$INSTDIR\${PRODUCT_NAME}.exe" "RUNASADMIN"

            // 更新上一次进度更新的时间戳和已上传的字节数
            lastProgressTime = currentTime;
            lastReceivedBytes = bytesRead;

            // 更新进度条
            int percent = static_cast<int>((bytesRead * 100) / totalBytes);
            ui->progressBar->setValue(percent);

            // 打印上传速度（字节/秒）以十进制格式
            //qDebug() << "Upload Speed (Bytes/Second):" << QString::number(uploadSpeed, 'f', 2);

            if(!qIsInf(ReceivedSpeed))
                ui->label_size->setText(fileanddirs.unitConversion(ReceivedSpeed)+"/s");

    }
    flag++;
    // 更新进度条或其他显示进度的控件
    int percent = static_cast<int>((bytesRead * 100) / totalBytes);
    ui->progressBar->setValue(percent);
    ui->label_process->setText(QString("%1/%2").arg(fileanddirs.unitConversion(bytesRead)).arg(fileanddirs.unitConversion(totalBytes)));
}

void FileReceiver::ftpDone(bool error)
{
    QFtp *ftp = qobject_cast<QFtp *>(sender());
    if (ftp) {
        if (error) {
            qDebug() << "FTP Download Error: " << ftp->errorString();
            // 处理下载错误逻辑
        } else {
            file->close();
            qDebug() << "FTP Download Finished";
            // 处理下载完成逻辑
            if(nMode==Open)
            {
                QUrl programUrl = QUrl::fromLocalFile(localPath);
                QDesktopServices::openUrl(programUrl);
            }
            else
            {
                fileanddirs.openExplorer(nlocalDir);
            }

            close();
        }

        // 断开 ftp 连接
        ftp->close();
    }
}

void FileReceiver::ftpError()
{
    QFtp *ftp = qobject_cast<QFtp *>(sender());
    if (ftp) {
        qDebug() << "FTP Error: " << ftp->errorString();
        // 处理 FTP 错误逻辑
    }
}

