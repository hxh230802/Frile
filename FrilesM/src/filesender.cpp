#include "include/filesender.h"
#include "ui_filesender.h"

#include <QProcess>

FileSender::FileSender(QStringList filelist, BaseInfo baseInfo, QString ServerIP, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSender)
{
    ui->setupUi(this);

    m_fileList = filelist;
    m_baseInfo = baseInfo;
    m_ServerIP = ServerIP;

    m_FilesAndDirs.createDirectory(QCoreApplication::applicationDirPath()+"/temp/");

    ui->FileNum_prb->setMaximum(0);
    ui->FileNum_prb->setValue(0);
    ui->FileSend_prb->setMaximum(0);
    ui->FileSend_prb->setValue(0);

    CompDir();

}

void FileSender::CompDir()
{
    QStringList dirlist;
    foreach (QString filepath, m_fileList)
    {
        QFileInfo fileInfo(filepath);
        if(fileInfo.isDir()) // 是目录
        {
            dirlist.append(filepath);
        }
    }
    if(dirlist.size()==0)QTimer::singleShot(100, this, &FileSender::SendFile);
    ui->FileNum_prb->setMaximum(dirlist.size());
    ui->FileNum_prb->setValue(0);
    ui->label->setText(QString("正在多线程压缩文件夹，共%1个，下方会显示压缩进度，请耐心等待。").arg(dirlist.size()));
    foreach (QString dirpath, dirlist)
    {
        QFileInfo fileInfo(dirpath);
        QString zipPath = QCoreApplication::applicationDirPath()+"/temp/"+fileInfo.fileName()+".7z";
        QFile::remove(zipPath);
        QProcess* process = new QProcess(this);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus){
            h++;
            ui->FileNum_prb->setValue(h);
            if(h == dirlist.size()) QTimer::singleShot(100, this, &FileSender::SendFile);

        });
        QString program = QCoreApplication::applicationDirPath()+"/7z.exe";
        QStringList arguments = {"a", zipPath, dirpath,"-mmt"};
        process->start(program, arguments);
    }
}

void FileSender::SendFile()
{

    ui->FileNum_prb->setMaximum(m_fileList.size());
    ui->FileSend_prb->setMaximum(100);
    ui->FileSend_prb->setValue(0);

    qint16 fileSubscript = ui->FileNum_prb->value();
    qDebug() << "fileSubscript" << fileSubscript;
    qDebug() << "sendfilelist" << m_fileList;

    // 更新文件数量 进度条
    ui->FileNum_prb->setValue(ui->FileNum_prb->value()+1);

    //尝试连接目标
    QTcpSocket *tcpClient_sendFiles;
    tcpClient_sendFiles = new QTcpSocket(this);
    // 连接到服务器
    tcpClient_sendFiles->connectToHost(m_ServerIP, 2601);
    // 检查连接是否成功
    if (!tcpClient_sendFiles->waitForConnected(3000)) {
        qDebug() << "连接失败：" << tcpClient_sendFiles->errorString();
        QMessageBox::critical(this, "错误", "服务器连接失败："+tcpClient_sendFiles->errorString());
        return;
    }

    // 发送文件
    QString filePath = m_fileList.at(fileSubscript);
    QFileInfo fileInfod(filePath);
    ui->label->setText("正在发送文件："+fileInfod.fileName());
    if(fileInfod.isDir())
    {
        filePath = QCoreApplication::applicationDirPath()+"/temp/"+fileInfod.fileName()+".7z";
    }
    qDebug() << "filePath" << filePath;
    // 创建一个文件对象
    file.setFileName(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"文件打开成功"<<file.fileName();
    }
    else
    {
        qDebug()<<"文件打开失败";
    }
    sendSize = 0;
    fileSize = 0;
    flag = false;
    // 使用 QFileInfo 获取文件大小
    QFileInfo fileInfo(file);
    fileSize = fileInfo.size();
    fileName = fileInfo.fileName();
    qDebug() << "fileName" << fileName;
    QObject::connect(tcpClient_sendFiles, &QTcpSocket::readyRead, [=]() {
        QByteArray data = tcpClient_sendFiles->readAll();
        qDebug() << fileName << data;
        if(QString(data) != fileName && flag == false) // 文件头信息不正确
        {
            return;
        }
        if(flag == true) // 对方进度回传
        {
            // 对方接收完成
            if(QString(data).contains("OK") || QString(data).toInt() == 100)
            {
                ui->FileSend_prb->setValue(0);

                if(ui->FileNum_prb->value()==ui->FileNum_prb->maximum()) // 对方全部接收完成
                {
                    qDebug() << "完成";
                    ui->label->setText("发送完成！");
                    ui->FileSend_prb->setValue(100);
                    return;
                }
                else
                {
                    qDebug() << "dig";
                    QTimer::singleShot(100, this, &FileSender::SendFile);
                }
            }
            else // 更新进度条
            {
                int progress = QString(data).toInt();
                ui->FileSend_prb->setValue(progress);
                return;
            }
        }

        flag = true;

        qint64 len = 0;
        do{
            char buf[64*1024] = {0};
            len = 0;
            len = file.read(buf,sizeof(buf));
            len = tcpClient_sendFiles->write(buf,len);
            int progress = (int)((sendSize*1.0)/(fileSize*1.0) * 100);
            qDebug() << fileName << "progress" << progress;
            sendSize += len;
        }while(len > 0);
        if(sendSize == fileSize)
        {

            file.close();
        }

    });

    //发送头部信息
    QString head = tr("%1##%2##%3").arg(m_baseInfo.toQString()).arg(fileName).arg(fileSize);
    quint64 len = tcpClient_sendFiles->write(head.toUtf8().data());
    if(len<=0)
    {
        qDebug()<<"头部信息发送失败 ";
        file.close();
    }
    qDebug() << "头部信息发送完成 ";
}

FileSender::~FileSender()
{
    delete ui;
}
