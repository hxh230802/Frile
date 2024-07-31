#include "settingwnd.h"
#include "ui_settingwnd.h"

#include "../criticalinfo.h"

SettingWnd::SettingWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingWnd)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("设置");

    ftpmgr = new FtpManager(SERVER_IP);

    ui->version_lbl->setText(QString("当前版本：%1").arg(VERSION));

    ui->grade_cmb->addItem("高一");
    ui->grade_cmb->addItem("高二");
    ui->grade_cmb->addItem("高三");

    int initialGrade = BaseInfo::classInfo.nGrade;
    int initialClassNumber = BaseInfo::classInfo.nClass;

    // 设置初始年级和班级信息
    ui->grade_cmb->setCurrentIndex(qBound(0, initialGrade - 1, 2)); // 限制在范围 [1, 3]

    // 创建班级标签和输入框
    ui->class_spb->setRange(1, 30);
    ui->class_spb->setValue(qBound(1, initialClassNumber, 30));
}

SettingWnd::~SettingWnd()
{
    delete ui;
}

void SettingWnd::on_Sure_btn_clicked()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认操作", "是否要进行此操作?",
                                  QMessageBox::Yes | QMessageBox::No);

    // 根据用户选择执行不同的函数
    if (reply == QMessageBox::Yes) {
        // 获取主机的IP地址
        QString ipAddress = getHostIpAddress();

        // 打印每一行IP地址
        QStringList ipList = ipAddress.split("\n", QString::SkipEmptyParts);
        qDebug() << "用户选择是，主机IP地址：";
        for (const QString& ip : ipList) {
            qDebug() << ip;
        }

        // 执行用户选择是时的逻辑
        executeFunctionOnYes();
    } else {
        // 执行用户选择否时的逻辑

    }

    accept(); // 默认的accept行为
}

void SettingWnd::on_cancel_btn_clicked()
{
    reject();
}

int SettingWnd::getSelectedGradeAsNumber() const
{
    // 根据中文描述的年级获取对应的数字
    QString selectedGradeStr = ui->grade_cmb->currentText();
    if (selectedGradeStr == "高一") {
        return 1;
    } else if (selectedGradeStr == "高二") {
        return 2;
    } else if (selectedGradeStr == "高三") {
        return 3;
    }
    return -1; // 如果出现未知的年级，返回-1或其他默认值
}

int SettingWnd::getClassNumberAsNumber() const
{
    return ui->class_spb->value();
}

QString SettingWnd::getHostIpAddress() const
{
    QString ipAddress;

    // 获取主机名
    QString hostName = QHostInfo::localHostName();

    // 根据主机名获取IP地址
    QHostInfo hostInfo = QHostInfo::fromName(hostName);
    foreach (const QHostAddress& address, hostInfo.addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ipAddress += address.toString() + "\n";
        }
    }

    return ipAddress;
}

void SettingWnd::executeFunctionOnYes()
{
    // 实现用户选择是时的逻辑
    BaseInfo::classInfo.nGrade = getSelectedGradeAsNumber();
    BaseInfo::classInfo.nClass = getClassNumberAsNumber();

    BaseInfo::WriteToSetting();

    qDebug() << getHostIpAddress();

    BaseInfo::uploadIPCfgFile();
}

void SettingWnd::on_autoRun_btn_clicked()
{
    QProcess::startDetached("explorer.exe", { "shell:Common Startup" });
}

void SettingWnd::on_regedit_btn_clicked()
{
    // 打开我的主页
    QString url = "regedit";
    QUrl bilibiliUrl(url);
    QDesktopServices::openUrl(bilibiliUrl);
}

void SettingWnd::on_website_btn_clicked()
{
    // 打开我的主页
    QString url = "http://www.轩豪小站.com/";
    QUrl bilibiliUrl(url);

    if (QDesktopServices::openUrl(bilibiliUrl)) {
        qDebug() << "URL opened successfully.";
    } else {
        qDebug() << "Failed to open URL.";

        QString url1 = QString("http://%1/").arg(WEBSITE_IP);
        QUrl bilibiliUrl1(url1);

        if (QDesktopServices::openUrl(bilibiliUrl1)) {
            qDebug() << "URL opened successfully.";
        } else {
            qDebug() << "Failed to open URL.";
        }
    }
}
