#ifndef FILESENDER_H
#define FILESENDER_H

#include <QWidget>
#include <QFile>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include <QSettings>

#include <myQLib_FilesAndDirs>

namespace Ui {
class FileSender;
}

struct BaseInfo
{
    uint8_t nGrade;    // 年级
    uint8_t nClass;    // 班级

    enum Subject { Public = 0, Chinese, Math, English, Physics, Chemistry, Biology, Geography, Politics, History };
    Subject eSubject;   // 学科

    // 将 BaseInfo 转换为 QString
    QString toQString() const { return QString("%1*%2*%3").arg(nGrade).arg(nClass).arg(eSubject); }

    // 将 QString 转换为 BaseInfo
    void toThis(const QString& data)
    {
        QStringList list = data.split("*");
        if (list.size() >= 3)
        {
            nGrade = list.at(0).toInt();
            nClass = list.at(1).toInt();
            eSubject = static_cast<Subject>(list.at(2).toInt());
        }
    }

    // 将 BaseInfo 转换为 相对路径
    QString toPath() const { return QString("/%1/%2/%3").arg(nGrade).arg(nClass).arg(eSubject); }

    // 将 BaseInfo 写入设置文件
    void writeToSettings() const
    {
        QString filePath = QCoreApplication::applicationDirPath() + "/setting.ini";
        QSettings settings(filePath, QSettings::IniFormat);
        QString group = "BaseInfo";
        settings.beginGroup(group);
        settings.setValue("nGrade", nGrade);
        settings.setValue("nClass", nClass);
        settings.setValue("eSubject", static_cast<int>(eSubject));
        settings.endGroup();
    }

    // 从设置文件中读取 BaseInfo
    void readFromSettings()
    {
        QString filePath = QCoreApplication::applicationDirPath() + "/setting.ini";
        QSettings settings(filePath, QSettings::IniFormat);
        QString group = "BaseInfo";
        settings.beginGroup(group);
        nGrade = settings.value("nGrade", 0).toUInt();
        nClass = settings.value("nClass", 0).toUInt();
        eSubject = static_cast<Subject>(settings.value("eSubject", 0).toInt());
        settings.endGroup();
    }
};


class FileSender : public QWidget
{
    Q_OBJECT

public:
    explicit FileSender(QStringList filelist,BaseInfo baseInfo,QString ServerIP, QWidget *parent = 0);
    ~FileSender();

    void CompDir();

private slots:
    void SendFile();

private:
    Ui::FileSender *ui;

    FilesAndDirs m_FilesAndDirs;
    int h = 0;

    QStringList m_fileList;
    BaseInfo m_baseInfo;
    QString m_ServerIP;

    QString fileName;
    qint64 fileSize;
    qint64 sendSize;
    QFile file;
    bool flag;
};

#endif // FILESENDER_H
