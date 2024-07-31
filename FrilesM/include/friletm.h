#ifndef FRILETM_H
#define FRILETM_H

#include "include/filesender.h"

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

#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QFile>

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <myQLib_FilesAndDirs>

namespace Ui {
class FriletM;
}

struct FileDocument
{
    QString name;            // 文件名或目录名
    QString path;            // 文件路径
    enum Type { file = 0, dir };    // 文件类型，file表示文件，dir表示目录
    Type type;               // 类型
    QString suffix;          // 文件类型的描述，如文本文件、图像文件等
    qint64 size;             // 文件大小（以字节为单位）
    QString s_size;          // 文件大小的字符串表示，如"2.5 MB"
    QString createTime;      // 创建时间
    QString changeTime;      // 修改时间
};

// 文件详细信息列表
class FileDocumentList  : public QObject
{
     Q_OBJECT
public:
    explicit FileDocumentList(QWidget* parentWidget = nullptr) : QObject(parentWidget) {mParentWidget=parentWidget;}
    void addFile(QString filePath)
    {
        qDebug() << filePath;
        if(find(filePath)) return; // 有相同文件，返回
        FileDocument document;
        // 获取详细信息
        QFileInfo fileInfo(filePath);
        // 获取文件名
        document.name = fileInfo.fileName();
        // 文件路径
        document.path = filePath;
        // 获取类型
        if(fileInfo.isFile()) document.type = FileDocument::Type::file;
        if(fileInfo.isDir()) document.type = FileDocument::Type::dir;
        //        if(document.type == NULL) return;
        // 分离出文件类型
        document.suffix = fileInfo.suffix();
        if(fileInfo.suffix() == "lnk") // 用户选择了快捷方式
        {
            QMessageBox::StandardButton reply = QMessageBox::warning(mParentWidget, "警告", "你选择了快捷方式，并不是文件本体。\n这可能会导致发送失败。请三思而后行！\n是否继续发送此快捷方式？", QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;  // 用户选择不发送，返回或执行其他操作
            }
        }
        // 获取文件大小
        document.size = fileanddirs.getFileOrDir_size(document.path);
        if(document.size == 0){ QMessageBox::warning(mParentWidget, "警告", "不可发送空文件！"); return;}
        // 智能判断大小
        document.s_size = fileanddirs.unitConversion(document.size);
        // 获取创建时间
        document.createTime = fileInfo.created().toString("yyyy-MM-dd hh:mm:ss");
        // 获取修改时间
        document.changeTime = fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");

        filedocumentlist.append(document);
    }
    bool find(QString filePath)
    {
        bool isSame = false;
        foreach(FileDocument filedoc, filedocumentlist)
        {
            if(filePath == filedoc.path) isSame = true;
        }
        return isSame;
    }

    QList<FileDocument> filedocumentlist;
    FilesAndDirs fileanddirs;
    QWidget* mParentWidget;
};

class FriletM : public QWidget
{
    Q_OBJECT

public:
    explicit FriletM(QWidget *parent = 0);

    ~FriletM();

private slots:
    void FileSendList_tw_cancelBtnClicked();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void updateBaseInfoFromUI();

    void on_select_Btn_clicked();

    void on_paste_Btn_clicked();

    void on_FileSend_Btn_clicked();

private:
    Ui::FriletM *ui;
    FileDocumentList m_filedocumentlist;
    FilesAndDirs fileanddirs;
    QString m_ServerIP = "127.0.0.1";
    BaseInfo m_baseInfo;

    /*文件信息数据输入*/
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
    /*文件信息显示*/
    void UIRefresh();
    void uiInit();
    void updateUIFromBaseInfo();
    /*TCP监测服务*/
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    void start_tcp_server();
    QString process_request(const QString &request);
    /*托盘程序服务*/
    QSystemTrayIcon *trayIcon = nullptr;
    void closeEvent(QCloseEvent *event);
    bool TrayIconInit();

private slots: // 开机自启
    void on_AutoStart_chk_toggled(bool checked);

    void on_BatchCancel_clicked();

private:
    void setAutoStart(bool enabled);
    bool isAutoStartEnabled();
};

#endif // FRILETM_H
