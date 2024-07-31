#ifndef FRILESM_H
#define FRILESM_H

#include <QWidget>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileInfo>
#include <QDateTime>
#include <QMimeData>
#include <QSettings>
#include <QProcess>
#include <QWidgetAction>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QDragEnterEvent>
#include <QDesktopServices>
#include <QPainter>

#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QFile>
#include <QCheckBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include "filereceiver.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <QFtp>
#include <QTreeWidgetItem>

#include "baseinfo.h"

#include <myQLib_FtpManager>

#include "settingwnd.h"

namespace Ui {
class FrilesM;
}


class FrilesM : public QWidget
{
    Q_OBJECT

protected:
    virtual void paintEvent(QPaintEvent *event) override;           //绘制事件（绘制透明背景）
    //背景颜色RGBA值
    int bkR = 230;
    int bkG = 230;
    int bkB = 230;
    int bkA = 190;//基础透明度
    int border_radius = 5; //窗口圆角半径

public:
    explicit FrilesM(QWidget *parent = 0);
    ~FrilesM();

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void on_Refresh_btn_clicked();

    void on_HXH_Btn_clicked();

    void addToList(const QUrlInfo &urlInfo);

    void openBtnClicked();
    void downloadBtnClicked();

    void on_backSpace_btn_clicked();
    void processItem(QTreeWidgetItem *item, int column);

    void on_maximize_btn_clicked();

    void on_goHome_btn_clicked();

    void on_setting_btn_clicked();

    void on_ftp_btn_clicked();

    void on_Restart_btn_clicked();

    void checkWindowState()
        {
            // 检测窗口是否被最小化
            if (isMinimized()) {
                qDebug() << "Window is minimized.";
            } else {
                qDebug() << "Window is not minimized.";
            }
        }

    void on_help_btn_clicked();

    void on_quit_btn_clicked();

    void on_information_btn_clicked();

private:
    Ui::FrilesM *ui;

    QMap<BaseInfo::Subject, QPair<QString, QString>> subjectMap = {
        {BaseInfo::Subject::GradePublic, {"年级公共", "Grade Public"}},
        {BaseInfo::Subject::ClassPublic, {"班级公共", "Class Public"}},
        {BaseInfo::Subject::Chinese, {"语文", "Chinese"}},
        {BaseInfo::Subject::Math, {"数学", "Math"}},
        {BaseInfo::Subject::English, {"英语", "English"}},
        {BaseInfo::Subject::Physics, {"物理", "Physics"}},
        {BaseInfo::Subject::Chemistry, {"化学", "Chemistry"}},
        {BaseInfo::Subject::Biology, {"生物", "Biology"}},
        {BaseInfo::Subject::Geography, {"地理", "Geography"}},
        {BaseInfo::Subject::Politics, {"政治", "Politics"}},
        {BaseInfo::Subject::History, {"历史", "History"}}
    };

    QFtp *ftp;
    FtpManager* ftpmgr;
    QString currentPath;
    QString rootPath;
    QHash<QString, bool> isDirectory;
    void changeDir(QString path);
    void showHome();
    void ChangeToRoot(QString name, bool hide);

    /*文件信息显示*/
    void UIRefresh();
    /*TCP监测服务*/
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    void start_tcp_server();
    QString process_request(const QString &request);

    /*托盘程序服务*/
    QSystemTrayIcon *trayIcon = nullptr;
    void closeEvent(QCloseEvent *event);
    bool TrayIconInit();

    void on_operate_btn_menu_init();
};

class FAQWindow : public QDialog
{
    Q_OBJECT

public:
    FAQWindow(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("常见问题");

        QLabel *titleLabel = new QLabel("如果有任何问题 试试重启软件", this);
        QPushButton *restartButton = new QPushButton("重启软件", this);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(titleLabel);
        layout->addWidget(restartButton);

        setLayout(layout);

        connect(restartButton, &QPushButton::clicked, this, &FAQWindow::restartSoftware);
    }

private slots:
    void restartSoftware()
    {
        qApp->exit(773);
    }
};

#endif // FRILESM_H
