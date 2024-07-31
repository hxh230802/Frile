#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCoreApplication>
#include <QProcess>
#include <QtDebug>

#include "UAC.h"

#include "../criticalinfo.h"

void modifyHostsFile(const QString& ipAddress, const QString& domain)
{
    QString hostsFilePath = "C:/Windows/System32/drivers/etc/hosts"; // 提示：使用实际的hosts文件路径

    QFile file(hostsFilePath);
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "Error", "Failed to open hosts file for writing!");
        return;
    }

    QString entry = ipAddress + " " + domain;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << entry << endl;
    file.close();

    // 刷新DNS缓存以使更改生效（需要管理员权限）
    QProcess::execute("ipconfig /flushdns");

    QMessageBox::information(nullptr, "Success", "Hosts file has been modified successfully!");
}

bool updateHostFile(const QString& filePath, const QString& keyIP)
{
    // 读取host文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open host file for reading.";
        return false;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        lines.append(line);
    }
    file.close();

    // 删除包含关键IP的那一行
    QStringList updatedLines;
    foreach (const QString& line, lines)
    {
        if (!line.contains(keyIP))
            updatedLines.append(line);
    }

    // 将修改后的内容写回host文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qDebug() << "Failed to open host file for writing.";
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    foreach (const QString& line, updatedLines)
    {
        out << line << endl;
    }
    file.close();

    qDebug() << "Host file updated successfully.";

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 以管理员权限启动一个新实例
    if (UAC::runAsAdmin())
    {
        return 0; // 启动成功，当前程序退出
    } // 未启动，当前程序继续

    QString ipAddress = WEBSITE_IP;
    QString domain = "www.轩豪小站.com";

    // Host文件路径
    QString hostFilePath = "C:/Windows/System32/drivers/etc/hosts";
    updateHostFile(hostFilePath,ipAddress);

    modifyHostsFile(ipAddress, domain);

    return 0;
}
