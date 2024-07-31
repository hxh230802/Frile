#ifndef CLASSSELECTIONDIALOG_H
#define CLASSSELECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>

#include <myQLib_FtpManager>

#include "baseinfo.h"

class ClassSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    ClassSelectionDialog(QWidget *parent = nullptr);

    int getSelectedGradeAsNumber() const;
    int getClassNumberAsNumber() const;

private slots:
    void onAccept();
    void executeFunctionOnYes();
    void executeFunctionOnNo();
    QString getHostIpAddress() const;

private:
    QComboBox *gradeComboBox;
    QGridLayout *classGridLayout;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    FtpManager* ftpmgr;
};
#endif // CLASSSELECTIONDIALOG_H
