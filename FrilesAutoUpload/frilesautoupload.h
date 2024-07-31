#ifndef FRILESAUTOUPLOAD_H
#define FRILESAUTOUPLOAD_H

#include <QObject>

#include<myQLib_FtpManager>

#include "../criticalinfo.h"

class FrilesAutoUpload : public QObject
{
    Q_OBJECT
public:
    explicit FrilesAutoUpload(QObject *parent = nullptr);
    void checkForUpdates();
        int compareWithLatestVersion(const QString& latestVersion);
            void update();
                void start();

private:
                FtpManager *ftpMgr;
                QString tempDir;
                bool flag = false;
};

#endif // FRILESAUTOUPLOAD_H
