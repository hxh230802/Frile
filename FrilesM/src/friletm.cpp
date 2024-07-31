#include "include/friletm.h"
#include "ui_friletm.h"

FriletM::FriletM(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriletM)
{
    ui->setupUi(this);
    start_tcp_server();
    TrayIconInit();

    m_baseInfo.readFromSettings();
    uiInit();

    fileanddirs.removeDirectory(QCoreApplication::applicationDirPath()+"/temp/");
}

/* ---------- 文件信息数据输入 ---------- */
// 拖拽
void FriletM::dragEnterEvent(QDragEnterEvent *event) // 拖动进入事件
{
    if(event->mimeData()->hasUrls())                    // 数据中是否包含URL
        event->acceptProposedAction();                  // 如果是则接收动作
    else event->ignore();                               // 否则忽略该事件
}
void FriletM::dropEvent(QDropEvent *event)           // 放下事件
{
    const QMimeData *mimeData = event->mimeData();      // 获取MIME数据
    if(mimeData->hasUrls())
    {                            // 如果数据中包含URL
        QList<QUrl> urlList = mimeData->urls();         // 获取URL列表

        // 将其中第一个URL表示为本地文件路径
        for(uint8_t i = 0; i < urlList.size(); i++)
        {
            QString fileName = urlList.at(i).toLocalFile();
            if(!fileName.isEmpty())    // 如果文件路径不为空
            {
                m_filedocumentlist.addFile(fileName);
            }
        }
        UIRefresh();

    }
}
// 选择按钮
void FriletM::on_select_Btn_clicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(nullptr, "选择文件", "", "All Files (*.*)");

    foreach(QString filePath, filePaths)
    {
        m_filedocumentlist.addFile(filePath);
    }
    UIRefresh();
}
// 粘贴按钮
void FriletM::on_paste_Btn_clicked()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        foreach (const QUrl &url, urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                m_filedocumentlist.addFile(filePath);
                UIRefresh();
            }
        }
    }
}
// Ctrl+v监测
void FriletM::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V)
    {
        // Ctrl+V 快捷键按下
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasUrls())
        {
            QList<QUrl> urlList = mimeData->urls();
            foreach (const QUrl &url, urlList)
            {
                if (url.isLocalFile())
                {
                    QString filePath = url.toLocalFile();
                    m_filedocumentlist.addFile(filePath);
                    UIRefresh();
                }
            }
        }
    }

    QWidget::keyPressEvent(event); // 将事件传递给基类处理

    return;
}

/* ---------- 文件信息显示 ---------- */
// 刷新UI
void FriletM::UIRefresh()
{

    ui->FileSendList_tw->clearContents();
    ui->FileSendList_tw->setRowCount(0);
    foreach (FileDocument file, m_filedocumentlist.filedocumentlist)
    {
        // 发送缓冲区1添加一行
        uint32_t row = ui->FileSendList_tw->rowCount()+1;
        ui->FileSendList_tw->setRowCount(row);
        // 设置复选框
        QTableWidgetItem *checkBox = new QTableWidgetItem();
        checkBox->setCheckState(Qt::Checked);
        ui->FileSendList_tw->setItem(row-1, 0, checkBox);
        // 显示文件名
        ui->FileSendList_tw->setItem(row-1, 1, new QTableWidgetItem(fileanddirs.GetSpecifiedFileIcon(file.path),file.name));
        // 显示文件大小
        ui->FileSendList_tw->setItem(row-1, 2, new QTableWidgetItem(file.s_size));
        QPushButton *cancelBtn = new QPushButton();
        cancelBtn->setText(tr("取消"));
        cancelBtn->setStyleSheet("QPushButton {background:rgb(237,85,101); color:rgb(255,255,255); border-radius:5px; min-height:20px; min-width:75px; font:15px \"Microsoft YaHei\";}"
                                 "QPushButton:hover {background:rgb(236,71,103);}"
                                 "QPushButton:pressed {background:rgb(171,71,37);}");
        cancelBtn->setCursor(Qt::PointingHandCursor);

        QWidget *buttonContainer = new QWidget();
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
        buttonLayout->setContentsMargins(3, 1, 3, 1); // 设置边距
        buttonLayout->addWidget(cancelBtn);
        buttonContainer->setLayout(buttonLayout);

        connect(cancelBtn, SIGNAL(clicked()), this, SLOT(FileSendList_tw_cancelBtnClicked()));
        ui->FileSendList_tw->setCellWidget(row - 1, 3, buttonContainer);

    }
}

// 表格中的取消按钮被点击
void FriletM::FileSendList_tw_cancelBtnClicked()
{
    QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->FileSendList_tw->indexAt(QPoint(senderObj->frameGeometry().x(),senderObj->frameGeometry().y()));
    uint32_t row    = idx.row();
    uint32_t column = idx.column();
    qDebug()<<"row"<<row<<"column"<<column;
    m_filedocumentlist.filedocumentlist.removeAt(row);       // 删除发送缓冲1（链表）
    ui->FileSendList_tw->removeRow(row); // 删除发送列表（UI）
}

// UI初始化
void FriletM::uiInit()
{
    this->setAcceptDrops(true);//启用拖动事件

    /* 发送缓冲区1 */
    ui->FileSendList_tw->setColumnCount(4);          // 五列
    ui->FileSendList_tw->setRowCount(0);             // 0行
    ui->FileSendList_tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 设置列宽自适应表格
    ui->FileSendList_tw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->FileSendList_tw->horizontalHeader()->resizeSection(0,50);
    ui->FileSendList_tw->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->FileSendList_tw->horizontalHeader()->resizeSection(0,150);
    ui->FileSendList_tw->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->FileSendList_tw->horizontalHeader()->resizeSection(0,75);
    ui->FileSendList_tw->setSelectionMode(QAbstractItemView::NoSelection);               // 设置禁止选中
    ui->FileSendList_tw->setEditTriggers(QAbstractItemView::NoEditTriggers);             // 设置禁止编辑
    ui->FileSendList_tw->setSelectionBehavior(QAbstractItemView::SelectRows);            // 整行选中的方式
    ui->FileSendList_tw->setShowGrid(false);

    QStringList table_h_headers;
    table_h_headers << "选择" << "文件名" << "大小" <<"取消";
    ui->FileSendList_tw->setHorizontalHeaderLabels(table_h_headers); //设置表头

    ui->class_spb->setRange(1,20);

    updateUIFromBaseInfo();

    connect(ui->class_spb, QOverload<int>::of(&QSpinBox::valueChanged), this, &FriletM::updateBaseInfoFromUI);
    connect(ui->grade_cmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FriletM::updateBaseInfoFromUI);
    connect(ui->subject_cmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FriletM::updateBaseInfoFromUI);

    connect(ui->AutoStart_chk, &QCheckBox::stateChanged, this, &FriletM::on_AutoStart_chk_toggled); // 开机自启槽函数
    ui->AutoStart_chk->setChecked(isAutoStartEnabled());

}

// 将 BaseInfo 对象值更新至控件
void FriletM::updateUIFromBaseInfo()
{
    ui->class_spb->setValue(m_baseInfo.nClass);
    ui->grade_cmb->setCurrentIndex(m_baseInfo.nGrade);
    ui->subject_cmb->setCurrentIndex(static_cast<int>(m_baseInfo.eSubject));
}

// 更新控件值至 BaseInfo 对象
void FriletM::updateBaseInfoFromUI()
{
    m_baseInfo.nGrade = ui->grade_cmb->currentIndex();
    m_baseInfo.nClass = ui->class_spb->value();
    m_baseInfo.eSubject = static_cast<BaseInfo::Subject>(ui->subject_cmb->currentIndex());
    m_baseInfo.writeToSettings();
}

// 批量发送
void FriletM::on_FileSend_Btn_clicked()
{
    if(m_filedocumentlist.filedocumentlist.size() == 0) return;
    // 确保文件被选择
    for(uint16_t i = 0; i < m_filedocumentlist.filedocumentlist.size(); i++)
    {
        if(ui->FileSendList_tw->item(i, 0)->checkState() == Qt::Checked)
            break;
        else if(i == m_filedocumentlist.filedocumentlist.size() - 1)
        {
            QMessageBox::warning(this, "警告", "请选择文件！");
            return;
        }
    }
    //尝试连接目标
    QTcpSocket *tcpClient_sendTest;
    tcpClient_sendTest = new QTcpSocket(this);
    // 连接到服务器
    tcpClient_sendTest->connectToHost(m_ServerIP, 2602);
    // 检查连接是否成功
    if (!tcpClient_sendTest->waitForConnected(3000)) {
        qDebug() << "连接失败：" << tcpClient_sendTest->errorString();
        QMessageBox::critical(this, "错误", "服务器连接失败："+tcpClient_sendTest->errorString());
        return;
    }
    tcpClient_sendTest->disconnect();
    tcpClient_sendTest->close();

    // 生成发送列表
    uint16_t i = 0;
    QStringList sendList;
    while(i != ui->FileSendList_tw->rowCount())
    {
        if(ui->FileSendList_tw->item(i, 0)->checkState() == Qt::Checked)
        {
            ui->FileSendList_tw->removeRow(i);
            sendList.append(m_filedocumentlist.filedocumentlist.at(i).path);
            m_filedocumentlist.filedocumentlist.removeAt(i);
        }
        else
        {
            i++;
        }
    }
    qDebug() << "tab" << sendList;
    FileSender *filesender = new FileSender(sendList,m_baseInfo,m_ServerIP);
    filesender->show();
}

void FriletM::on_BatchCancel_clicked()
{
    uint32_t i = 0;
    while(i != ui->FileSendList_tw->rowCount())
    {
        if(ui->FileSendList_tw->item(i, 0)->checkState() == Qt::Checked)
        {
            ui->FileSendList_tw->removeRow(i);
            m_filedocumentlist.filedocumentlist.removeAt(i);
        }
        else
        {
            i++;
        }
    }
}

/*TCP监测服务*/
void FriletM::start_tcp_server() // 启动TCP服务
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

    if (!server->listen(QHostAddress::Any, 6019)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
}
QString FriletM::process_request(const QString &request) // TCP消息处理
{
    if (request == "show") // 显示窗口
    {
        show();
    }
    else                   // 有文件
    {
        show();
        QString modifiedRequest = request;
        modifiedRequest.replace("\\", "/");
        m_filedocumentlist.addFile(modifiedRequest);
        UIRefresh();
    }
    return "服务器的响应成功";
}

/*托盘程序服务*/
void FriletM::iconActivated(QSystemTrayIcon::ActivationReason reason)
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
void FriletM::closeEvent(QCloseEvent *event)
{
    if (trayIcon&&trayIcon->isVisible())
    {
        hide(); //关闭时隐藏任务栏
        event->ignore();
    }
    else
        event->accept();
}
bool FriletM::TrayIconInit()
{
    // 托盘初始化
    if(QSystemTrayIcon::isSystemTrayAvailable())
    {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon("://Frilet.ico"));
        trayIcon->setToolTip("文件传输自由「教师端」 ");

        // 添加开机自启复选框
        QCheckBox *startupCheckBox = new QCheckBox("开机自启");
        startupCheckBox->setChecked(ui->AutoStart_chk->isChecked());
        connect(ui->AutoStart_chk, &QCheckBox::stateChanged, [this, startupCheckBox](int state) { startupCheckBox->setChecked(state == Qt::Checked); });
        connect(startupCheckBox, &QCheckBox::stateChanged, [this, startupCheckBox](int state) { ui->AutoStart_chk->setChecked(state == Qt::Checked); });

        QAction *miniSizeAction = new QAction("最小化(&N)",this);
        QAction * maxSizeAction = new QAction("最大化(&X)",this);
        QAction * restoreWinAction = new QAction("还 原(&R)",this);
        QAction *quitAction = new QAction("退 出(&Q)",this);
        connect(miniSizeAction,SIGNAL(triggered()),this,SLOT(hide()));
        connect(maxSizeAction,SIGNAL(triggered()),this,SLOT(showMaximized()));
        connect(restoreWinAction,SIGNAL(triggered()),this,SLOT(showNormal()));
        connect(quitAction,SIGNAL(triggered()),qApp,SLOT(quit()));

        QMenu * myMenu = new QMenu((QWidget*)QApplication::desktop());
        QWidgetAction *action = new QWidgetAction(this);
        action->setDefaultWidget(startupCheckBox);
        myMenu->addActions(QList<QAction*>() << (qobject_cast<QAction*>(action))); // 没有直接加 QAction 的接口，只能绕一下

        myMenu->addAction(miniSizeAction);
        myMenu->addAction(maxSizeAction);
        myMenu->addAction(restoreWinAction);
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

/*开机自启服务*/
void FriletM::on_AutoStart_chk_toggled(bool checked)
{
    setAutoStart(checked);
}
void FriletM::setAutoStart(bool enabled)
{
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enabled) {
        reg.setValue("Frilet", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        reg.remove("Frilet");
    }
}
bool FriletM::isAutoStartEnabled()
{
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return reg.contains("Frilet");
}


FriletM::~FriletM()
{
    delete ui;
}

