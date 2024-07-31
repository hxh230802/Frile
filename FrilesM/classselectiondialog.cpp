#include "ClassSelectionDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <QHostInfo>
#include <QDebug>
#include <QSpinBox>

ClassSelectionDialog::ClassSelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    ftpmgr = new FtpManager(SERVER_IP);

    // 设置窗口标题
    setWindowTitle("班级选择");

    // 创建年级标签和下拉框
    QLabel *gradeLabel = new QLabel("年级：");
    gradeComboBox = new QComboBox;
    gradeComboBox->addItem("高一");
    gradeComboBox->addItem("高二");
    gradeComboBox->addItem("高三");

    int initialGrade = BaseInfo::classInfo.nGrade;
    int initialClassNumber = BaseInfo::classInfo.nClass;

    // 设置初始年级和班级信息
    gradeComboBox->setCurrentIndex(qBound(0, initialGrade - 1, 2)); // 限制在范围 [1, 3]

    // 创建班级标签和输入框
    QLabel *classLabel = new QLabel("班级：");
    QSpinBox *classSpinBox = new QSpinBox;
    classSpinBox->setRange(1, 30);
    classSpinBox->setValue(qBound(1, initialClassNumber, 30));

    // 设置布局
    classGridLayout = new QGridLayout;
    classGridLayout->addWidget(gradeLabel, 0, 0);
    classGridLayout->addWidget(gradeComboBox, 0, 1);
    classGridLayout->addWidget(classLabel, 1, 0);
    classGridLayout->addWidget(classSpinBox, 1, 1);

    // 创建确认和取消按钮
    confirmButton = new QPushButton("确认");
    cancelButton = new QPushButton("取消");

    // 设置布局
        QGridLayout *mainLayout = new QGridLayout;
        mainLayout->addLayout(classGridLayout, 0, 0, 1, 2);
        mainLayout->addWidget(cancelButton, 1, 0);
        mainLayout->addWidget(confirmButton, 1, 1);

    setLayout(mainLayout);

    // 连接accept()槽函数
    connect(confirmButton, SIGNAL(clicked()), this, SLOT(onAccept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    // 将焦点设置到确认按钮上
        confirmButton->setFocus();
}

int ClassSelectionDialog::getSelectedGradeAsNumber() const
{
    // 根据中文描述的年级获取对应的数字
    QString selectedGradeStr = gradeComboBox->currentText();
    if (selectedGradeStr == "高一") {
        return 1;
    } else if (selectedGradeStr == "高二") {
        return 2;
    } else if (selectedGradeStr == "高三") {
        return 3;
    }
    return -1; // 如果出现未知的年级，返回-1或其他默认值
}

int ClassSelectionDialog::getClassNumberAsNumber() const
{
    // 由于使用QSpinBox，直接获取值即可
    QSpinBox *classSpinBox = qobject_cast<QSpinBox*>(classGridLayout->itemAtPosition(1, 1)->widget());
    return classSpinBox->value();
}

void ClassSelectionDialog::onAccept()
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
        executeFunctionOnNo();
    }

    accept(); // 默认的accept行为
}

QString ClassSelectionDialog::getHostIpAddress() const
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

void ClassSelectionDialog::executeFunctionOnYes()
{
    // 实现用户选择是时的逻辑
    BaseInfo::classInfo.nGrade = getSelectedGradeAsNumber();
    BaseInfo::classInfo.nClass = getClassNumberAsNumber();

    BaseInfo::WriteToSetting();

    qDebug() << getHostIpAddress();

    BaseInfo::uploadIPCfgFile();
}

void ClassSelectionDialog::executeFunctionOnNo()
{
    // 实现用户选择否时的逻辑
}
