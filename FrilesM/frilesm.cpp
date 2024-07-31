#include "frilesm.h"
#include "ui_frilesm.h"
#include <QRegularExpression>

#include <Windows.h>
#include <winuser.h>

#include <QTimer>

#define WDA_NONE        0x00000000
#define WDA_MONITOR     0x00000001
#define WDA_EXCLUDEFROMCAPTURE 0x00000011

#include <QFileIconProvider>
#include <QIcon>
#include <QTemporaryFile>

QIcon getFileIcon(const QString& fileName) {

    int lastDotIndex = fileName.lastIndexOf(".");
    QString fileExtension = fileName.mid(lastDotIndex + 1);

    QFileIconProvider provider;
    QIcon icon;

    // 创建临时文件
    QString strTemplateName = QDir::tempPath() + QDir::separator() +
                QCoreApplication::applicationName() + "." + fileExtension;

    QFile tmpFile(strTemplateName);


    if (tmpFile.open(QIODevice::ReadWrite)) {
        icon = provider.icon(QFileInfo(tmpFile.fileName()));
        // 在这里可以使用图标（icon），例如显示在界面上或者进行其他处理
        qDebug() << "成功打开文件：" << strTemplateName;
    } else {
        qDebug() << "无法打开文件：" << strTemplateName;
    }

    return icon;
}

QString _ToFtpEncoding(const QString &InputStr)
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

QString _FromFtpEncoding(const QString &InputStr)
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

#include <QFontMetrics>

QString truncateString(const QString &inputString, int maxWidth) {
    QString truncatedString;
    int currentWidth = 0;

    QFont font;  // 创建一个默认字体
    QFontMetrics metrics(font);

    // 获取字符 'a' 的宽度
    int charWidth = metrics.width('a');

    for (int i = 0; i < inputString.length(); ++i) {
        QChar currentChar = inputString.at(i);

        // 获取当前字符的宽度
        int currentCharWidth = metrics.width(currentChar);

        // 判断是否超过限制宽度
        if (currentWidth + currentCharWidth > maxWidth * charWidth) {
            truncatedString.append("..."); // 添加省略号
            break;
        }

        truncatedString.append(currentChar);
        currentWidth += currentCharWidth;
    }

    return truncatedString;
}

// 声明一个函数，用于获取字符串的宽度
int getStringWidth(const QString& text) {
    // 创建一个默认字体
    QFont font;

    // 使用字体创建字体度量对象
    QFontMetrics metrics(font);

    // 获取字符串的宽度
    int stringWidth = metrics.width(text);

    // 返回字符串宽度
    return stringWidth;
}

// 添加Item
void FrilesM::addToList(const QUrlInfo &urlInfo)
{
    // 文件名和图标
    QTreeWidgetItem *item = new QTreeWidgetItem;
    QString name = _FromFtpEncoding(urlInfo.name());
    item->setText(0, name);
    if(urlInfo.isDir())
    {
        isDirectory[name] = true;
        item->setIcon(0,QIcon(":/resources/images/dir.svg"));
    }
    else
        item->setIcon(0,getFileIcon(name));

    ui->fileList_trw->addTopLevelItem(item);

    // 操作按钮
    QString blueBtn = "QPushButton {background:rgb(67, 101, 237); color:rgb(255, 255, 255); border-radius:5px; min-height:20px; min-width:45px; font:15px \"Microsoft YaHei\";}"
                      "QPushButton:hover {background:rgb(61, 92, 213);}"
                      "QPushButton:pressed {background:rgb(36, 57, 133);}";
    QString orangeBtn = "QPushButton {background:rgb(255, 165, 0); color:rgb(255, 255, 255); border-radius:5px; min-height:20px; min-width:45px; font:15px \"Microsoft YaHei\";}"
                        "QPushButton:hover {background:rgb(255, 140, 0);}"
                        "QPushButton:pressed {background:rgb(255, 127, 0);}";

    // 创建打开按钮
    QPushButton *openButton = new QPushButton("打开");openButton->setMaximumWidth(getStringWidth("打开"));
    openButton->setStyleSheet(orangeBtn);
    openButton->setProperty("NameData", name);
    connect(openButton, &QPushButton::clicked, this,&FrilesM::openBtnClicked);
    ui->fileList_trw->setItemWidget(item, 1, openButton); // 将下载按钮添加到第二列
    // 创建下载按钮
    QPushButton *downloadButton = new QPushButton("下载");downloadButton->setMaximumWidth(getStringWidth("下载"));
    downloadButton->setStyleSheet(blueBtn);
    downloadButton->setProperty("NameData", name);
    connect(downloadButton, &QPushButton::clicked, this,&FrilesM::downloadBtnClicked);
    ui->fileList_trw->setItemWidget(item, 2, downloadButton); // 将打开按钮添加到第三列
}

#include <QMessageBox>
void FrilesM::openBtnClicked()
{

    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (senderBtn == nullptr) {
        return;
    }

    qDebug() << "打开按钮被点击";

    QString name = senderBtn->property("NameData").toString();

    if (isDirectory.value(name) )
    {    // 是目录
        changeDir(currentPath+"/"+name);
    }
    else // 是文件
    {
        FileReceiver *f = new FileReceiver(FileReceiver::Open,rootPath+"/"+currentPath,BaseInfo::tmpDir+"/"+(new FilesAndDirs)->GenerateRandomString(25),name,this);
        f->setWindowFlags(Qt::Window /*| Qt::WindowStaysOnTopHint*/);f->show();
    }
}


void FrilesM::downloadBtnClicked()
{
    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (senderBtn == nullptr) {
        return;
    }

    qDebug() << "下载按钮被点击";

    QString name = senderBtn->property("NameData").toString();

    if (isDirectory.value(name) )
    { // 是目录
        changeDir(currentPath+"/"+name);
    }
    else // 是文件
    {
        // 获取桌面目录路径
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

        // 弹出文件对话框，并设置默认打开的目录为桌面
        QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("选择保存目录"),
                                                                      desktopPath,
                                                                      QFileDialog::ShowDirsOnly
                                                                      | QFileDialog::DontResolveSymlinks);
        if (selectedDirectory.isEmpty()) {
            // 用户选择了目录，可以在这里处理所选目录的路径（selectedDirectory）
            // 例如，将路径显示在标签上或执行与所选目录相关的操作
            return;
        }
        FileReceiver *f = new FileReceiver(FileReceiver::Download,rootPath+"/"+currentPath,selectedDirectory,name,this);
        f->setWindowFlags(Qt::Window /*| Qt::WindowStaysOnTopHint*/);f->show();
    }

}

void FrilesM::changeDir(QString path)
{
    // 检查当前连接状态
    qDebug() << ftp->state();
    if (ftp->state() == QFtp::Unconnected) {
        qDebug() << __FUNCTION__ << "Unconnected";
        // 尚未连接，则进行连接和登录
        ftp->connectToHost(SERVER_IP); //  FTP 主机名
        ftp->login();    // 实际的用户名和密码
    }
    qDebug() << __FUNCTION__ << path << SERVER_IP;
    currentPath = path;
    ui->currentPath_lbl->setText(currentPath);
    isDirectory.clear();
    ui->fileList_trw->clear();
    ftp->cd(_ToFtpEncoding(rootPath + currentPath));
    ftp->list();
}

void FrilesM::processItem(QTreeWidgetItem *item, int /*column*/)
{
    qDebug() << __FUNCTION__ << item << item->text(0);
    QString name = item->text(0);
    if(rootPath == "")
    {
        ChangeToRoot(name,false/*true*/);
    }else
    {
        if (isDirectory.value(name))
        {
            changeDir(currentPath+"/"+name);
        }
        else
        {
            FileReceiver *f = new FileReceiver(FileReceiver::Open,rootPath+"/"+currentPath,BaseInfo::tmpDir+"/"+(new FilesAndDirs)->GenerateRandomString(25),name,this);
            f->setWindowFlags(Qt::Window /*| Qt::WindowStaysOnTopHint*/);f->show();

        }
    }

}

#include<QDialogButtonBox>

void FrilesM::ChangeToRoot(QString name, bool hide)
{
    if(name == "年级公共")
    {
        ui->subject_lbl->setText(name);
        rootPath = QString("/Frile/Classes/FilePublic/%1-0").arg(BaseInfo::classInfo.nGrade);
        changeDir("/");
        return;
    }
    if(name == "班级公共")
    {
        ui->subject_lbl->setText(name);
        rootPath = QString("/Frile/Classes/FilePublic/%1-%2").arg(BaseInfo::classInfo.nGrade).arg(BaseInfo::classInfo.nClass);
        changeDir("/");
        return;
    }

    BaseInfo::SubjectData subjectData = BaseInfo::getDataFromJSON(BaseInfo::getEnglishName(name));

    if (subjectData.password != "")
    {
        if(hide)
        {
            return;
        }
        // 输入正确的密码
        QString correctPassword = subjectData.password; // 用你实际的正确密码替换

        bool passwordMatched = false;

        while (!passwordMatched)
        {
            bool ok;
            QInputDialog dialog(this);
            dialog.setWindowTitle("密码验证");
            dialog.setLabelText("请输入密码：");
            dialog.setTextEchoMode(QLineEdit::Password);
            dialog.setInputMode(QInputDialog::TextInput);
            dialog.setInputMethodHints(Qt::ImhNoPredictiveText);
            dialog.setOkButtonText("确定");
            dialog.setCancelButtonText("取消");

            // 设置按钮布局
            auto *buttonBox = dialog.findChild<QDialogButtonBox *>();
            if (buttonBox)
            {
                buttonBox->setLayoutDirection(Qt::RightToLeft);
            }

            QString enteredPassword;
            if (dialog.exec())
            {
                enteredPassword = dialog.textValue();
            }
            else
            {
                qDebug() << "用户点击了取消按钮";
                return; // 退出函数
            }

            if (enteredPassword == correctPassword)
            {
                // 密码匹配，设置标志为真
                passwordMatched = true;

                // 在这里执行下一步操作
                // QMessageBox::information(nullptr, "成功", "密码验证成功！");
            }
            else
            {
                // 密码不匹配，显示错误消息
                QMessageBox::StandardButton reply = QMessageBox::warning(this, "错误", "密码不正确，是否重新输入？", QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::No)
                {
                    // 用户点击了取消按钮
                    qDebug() << "用户点击了取消按钮";
                    return; // 退出函数
                }
            }


        }
    }

    rootPath = subjectData.path;

    if (rootPath == "") // 没有此科老师
    {
        if(hide) return;
        QMessageBox::warning(this, "警告", "此科老师还未绑定！");
        return;
    }

    qDebug() << rootPath;
    ui->subject_lbl->setText(name);
    changeDir("/");
    // 可以在这里添加其他需要在函数执行完毕后继续执行的代码
}


void FrilesM::showHome()
{
    BaseInfo::uploadIPCfgFile();

    ui->subject_lbl->setText("");
    ui->currentPath_lbl->setText("");
    rootPath = "";
    isDirectory.clear();
    ui->fileList_trw->clear();

    // 遍历每个枚举值并输出对应的中文名和英文名
    for (int i = BaseInfo::GradePublic; i <= BaseInfo::History; ++i) {
        //            qDebug() << "枚举值:" << i;
        QString ChineseName = subjectMap[static_cast<BaseInfo::Subject>(i)].first;
        QString EnglishName = subjectMap[static_cast<BaseInfo::Subject>(i)].second;
        //            qDebug() << "中文名:" << bindList.subjectMap[static_cast<BindList::Subject>(i)].first;
        //            qDebug() << "英文名:" << bindList.subjectMap[static_cast<BindList::Subject>(i)].second;
        // 文件名和图标
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, ChineseName);
        item->setIcon(0,QIcon(QString(":/resources/images/%1.svg").arg(EnglishName)));
        ui->fileList_trw->addTopLevelItem(item);
    }
}

void FrilesM::on_backSpace_btn_clicked()
{
    if(rootPath == "") return;
    if(currentPath=="/")
        showHome();
    else
        changeDir(currentPath.left(currentPath.lastIndexOf('/')));// 提取父目录部分
}

#include <QScroller>

#include "classselectiondialog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

void cleanTempDirectory(QString prefix) {
    // 获取Temp目录路径
    QString tempPath = QDir::tempPath();

    // 创建QDir对象
    QDir tempDir(tempPath);

    // 设置过滤器和排序方式
    tempDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    tempDir.setSorting(QDir::Name);

    // 获取目录下所有文件
    QStringList fileList = tempDir.entryList();

    // 遍历文件列表
    foreach (const QString &fileName, fileList) {
        // 检查文件名是否以"Frilet_"开头
        if (fileName.startsWith(prefix)) {
            // 构建文件的完整路径
            QString filePath = tempDir.filePath(fileName);

            // 创建QFileInfo对象
            QFileInfo fileInfo(filePath);

            // 输出要删除的文件路径
            qDebug() << "Deleting file:" << filePath;

            // 删除文件
            if (QFile::remove(filePath)) {
                qDebug() << "File deleted successfully.";
            } else {
                qDebug() << "Error deleting file.";
            }
        }
    }
}

#include "lessonmanager.h"

FrilesM::FrilesM(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrilesM)
{
    ui->setupUi(this);

    TrayIconInit();
    start_tcp_server();

    BaseInfo::initBaseInfo();

    BaseInfo::ReadFromSetting();

    ui->class_lbl->setText(BaseInfo::FormatGradeClass());

    LessonManager* lessonMgr = new LessonManager;
    connect(lessonMgr,&LessonManager::lessonStart,this,[=](QString lessonName){
        qDebug() << lessonName;
        if(lessonName == "Others")  return;
        ChangeToRoot(BaseInfo::getChineseName(lessonName),true);
    });
    connect(lessonMgr,&LessonManager::goHome,this,[=](){
        showHome();
    });

    cleanTempDirectory("FrilesM_");

    ftp = new QFtp(this);

    if(BaseInfo::classInfo.nGrade==0||BaseInfo::classInfo.nClass==0)
    {
        on_setting_btn_clicked();
    }

    (new FilesAndDirs)->clearDirectory(BaseInfo::tmpDir);

    connect(ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));


    connect(ui->fileList_trw, &QTreeWidget::itemDoubleClicked, this, &FrilesM::processItem);

    on_operate_btn_menu_init();

    ui->fileList_trw->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QScroller::grabGesture(ui->fileList_trw->viewport(), QScroller::LeftMouseButtonGesture); //鼠标左键触摸, 如果想仅支持触摸屏下触摸可以使用QScroller::TouchGestur

    QUrl programUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/FrilesAutoUpload.exe");
    QDesktopServices::openUrl(programUrl);

    connect(ui->backSpace_btn_2,&QPushButton::clicked,this,&FrilesM::on_backSpace_btn_clicked);
    connect(ui->Refresh_btn_2,&QPushButton::clicked,this,&FrilesM::on_Refresh_btn_clicked);
    connect(ui->goHome_btn_2,&QPushButton::clicked,this,&FrilesM::on_goHome_btn_clicked);

    showHome();

}


/* ---------- 文件信息显示 ---------- */
// 刷新UI
void FrilesM::on_Refresh_btn_clicked()
{
    if(rootPath == "") BaseInfo::downloadJsonFile();
    else changeDir(currentPath);
}

/*TCP监测服务*/
void FrilesM::start_tcp_server() // 启动TCP服务
{
    server = new QTcpServer(QCoreApplication::instance());

    connect(server, &QTcpServer::newConnection, [&]() {
        QTcpSocket *client = server->nextPendingConnection();
        clients.append(client);

        connect(client, &QTcpSocket::readyRead, this, [=]() {
            QByteArray data = client->readAll();
            QString request = QString::fromUtf8(data);
            qDebug() << "Received request: " << request;

            QString response = process_request(request);
            client->write(response.toUtf8());
        });

        connect(client, &QTcpSocket::disconnected, this, [=]() {
            clients.removeAll(client);
            client->deleteLater();
            qDebug() << "Client disconnected";
        });
    });

    if (!server->listen(QHostAddress::Any, 6020)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
}
QString FrilesM::process_request(const QString &request) // TCP消息处理
{
    if (request == "show") // 显示窗口
    {
        show();
    }
    return "服务器的响应成功";
}

/*托盘程序服务*/
void FrilesM::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:

    case QSystemTrayIcon::DoubleClick: // 左键双击
        showNormal();
        break;
    case QSystemTrayIcon::MiddleClick: // 鼠标中键
        trayIcon->showMessage("文件传输自由「教师端」","欢迎使用",QSystemTrayIcon::Information,1000);
        break;
    default:
        break;
    }
}
void FrilesM::closeEvent(QCloseEvent *event)
{
    if (trayIcon&&trayIcon->isVisible())
    {
        hide(); //关闭时隐藏任务栏
        event->ignore();
    }
    else
        event->accept();
}
bool FrilesM::TrayIconInit()
{
    // 托盘初始化
    if(QSystemTrayIcon::isSystemTrayAvailable())
    {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon("://Friles.ico"));
        trayIcon->setToolTip("文件传输自由「学生端」 ");

        QAction *restartAction = new QAction(QIcon(":/resources/images/restart.svg"),"重启软件(&R)",this);
        QAction *quitAction = new QAction(QIcon(":/resources/images/quit.svg"),"退出软件(&Q)",this);

        connect(restartAction, &QAction::triggered, this, [this]() { qApp->exit(773); });
        connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

        QMenu * myMenu = new QMenu((QWidget*)QApplication::desktop());

        myMenu->addAction(restartAction);
        myMenu->addSeparator();     //加入一个分离符
        myMenu->addAction(quitAction);

        trayIcon->setContextMenu(myMenu);     //设置托盘菜单

        trayIcon->show();

        connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        return true;
    }
    return false;
}

FrilesM::~FrilesM()
{
    delete ui;
}




void FrilesM::on_HXH_Btn_clicked()
{
    // 打开我的主页
    QString url = "https://space.bilibili.com/1396650915?spm_id_from=333.1007.0.0";
    QUrl bilibiliUrl(url);

    if (QDesktopServices::openUrl(bilibiliUrl)) {
        qDebug() << "URL opened successfully.";
    } else {
        qDebug() << "Failed to open URL.";
    }
}

void FrilesM::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(bkR,bkG,bkB,bkA)));//背景色
    painter.setPen(Qt::transparent);//rgba(0,0,0,0)
    painter.setRenderHint(QPainter::Antialiasing);//抗锯齿
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect(),border_radius,border_radius);//添加绘制区域为窗口区域，并设置圆角
    painter.drawPath(painterPath);//绘制
}


void FrilesM::on_maximize_btn_clicked()
{
    if (isMaximized())
    {
        // 在最大化时设置透明度
        bkA = 190;  // 设置为最大透明度值
        showNormal();
        sender()->setProperty("text", "最大化");
    }
    else
    {
        // 在最小化时设置降低透明度
        bkA = 255;  // 设置为最小透明度值
        showMaximized();
        sender()->setProperty("text", "还  原");
    }

    // 重绘窗口
    update();
}

void FrilesM::on_goHome_btn_clicked()
{
    showHome();
}

void FrilesM::on_setting_btn_clicked()
{
//    ClassSelectionDialog d(this);
    SettingWnd d(this);
    int dialogResult = d.exec();

    if (dialogResult == QDialog::Accepted)
    {
        // 用户点击了确定按钮
        qDebug() << "用户点击了确定按钮";
        // 处理相关逻辑
        BaseInfo::ReadFromSetting();
        ui->class_lbl->setText(BaseInfo::FormatGradeClass());
    }
    else if (dialogResult == QDialog::Rejected)
    {
        // 用户点击了取消按钮或关闭按钮
        qDebug() << "用户点击了取消按钮或关闭按钮";
        // 处理相关逻辑
    }
}

void FrilesM::on_ftp_btn_clicked()
{
    if(rootPath=="") return;
//    QProcess *p = new QProcess;
//    p->start("explorer",QStringList("ftp://Emergency:h_114514@"+SERVER_IP"/"+rootPath));
    //    QMessageBox::information(this,"提示","不会有BUG啦！这个按钮已经成为历史。");
}

void FrilesM::on_Restart_btn_clicked()
{
    qApp->exit(773);
}

void FrilesM::on_help_btn_clicked()
{
    QUrl programUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/Help/Friles-Help-3.0.0.0.pdf");
    QDesktopServices::openUrl(programUrl);
}

void FrilesM::on_operate_btn_menu_init()
{

}


void FrilesM::on_quit_btn_clicked()
{
    QMenu *operate_btn_menu;

    QAction *restartAction = new QAction(QIcon(":/resources/images/restart.svg"),"重启软件",this);
    QAction *quitAction = new QAction(QIcon(":/resources/images/quit.svg"),"退出软件",this);

    connect(restartAction, &QAction::triggered, this, [this]() { qApp->exit(773); });
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    operate_btn_menu = new QMenu((QWidget*)QApplication::desktop());

    operate_btn_menu->addAction(restartAction);
    operate_btn_menu->addAction(quitAction);

    QPoint pos = ui->quit_btn->pos();
    pos.setX(pos.x() + ui->quit_btn->width() +1);
    pos.setY(pos.y() - 1);
    //计算菜单位置
    operate_btn_menu->exec(mapToGlobal(pos));
}

void FrilesM::on_information_btn_clicked()
{
    QMenu *operate_btn_menu;

    QAction *helpAction = new QAction(QIcon(":/resources/images/help.svg"),"使用教程",this);
    QAction *questionAction = new QAction(QIcon(":/resources/images/question.svg"),"常见问题",this);

    connect(helpAction, &QAction::triggered, this, [this]() {
        QUrl programUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/Help/Friles-Help-3.0.0.0.pdf");
        QDesktopServices::openUrl(programUrl);
    });
    connect(questionAction, &QAction::triggered, this, [this](){
        FAQWindow faqwnd(this);
        faqwnd.exec();
    });

    operate_btn_menu = new QMenu((QWidget*)QApplication::desktop());

    operate_btn_menu->addAction(questionAction);
    operate_btn_menu->addAction(helpAction);

    QPoint pos = ui->information_btn->pos();
    pos.setX(pos.x() + ui->information_btn->width() +1);
    pos.setY(pos.y() - 1);
    //计算菜单位置
    operate_btn_menu->exec(mapToGlobal(pos));
}
