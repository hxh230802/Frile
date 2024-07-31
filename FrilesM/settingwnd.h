#ifndef SETTINGWND_H
#define SETTINGWND_H

#include <QDialog>

#include <QMessageBox>
#include <QHostInfo>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>

#include <myQLib_FtpManager>

#include "baseinfo.h"

namespace Ui {
class SettingWnd;
}

class SettingWnd : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWnd(QWidget *parent = 0);
    int getSelectedGradeAsNumber() const;
    int getClassNumberAsNumber() const;
    ~SettingWnd();

private slots:
    void on_Sure_btn_clicked();
    void on_cancel_btn_clicked();
    void executeFunctionOnYes();
    QString getHostIpAddress() const;

    void on_autoRun_btn_clicked();

    void on_regedit_btn_clicked();

    void on_website_btn_clicked();

private:
    Ui::SettingWnd *ui;
    FtpManager* ftpmgr;
};

#endif // SETTINGWND_H
