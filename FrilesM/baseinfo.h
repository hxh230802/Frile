#ifndef BASEINFO_H
#define BASEINFO_H

#include <QObject>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QJsonObject>
#include <QFile>
#include <QMap>
#include <QtDebug>
#include <QDir>
#include <myQLib_FtpManager>

#include "../criticalinfo.h"

#define APP_DIR             QCoreApplication::applicationDirPath()   // 软件所在目录
#define APPDATA_DIR         QStandardPaths::writableLocation(QStandardPaths::AppDataLocation

class BaseInfo
{
public:
    BaseInfo();
    static void initBaseInfo();

    /* 设置文件相关内容 */

    struct ClassInfo
    {
        int nGrade;
        int nClass;
    };
    static QString settingFilePath;
    static QString bindListPath;
    static QString tmpDir;
    static ClassInfo classInfo;

    struct SubjectData {
        QString subject;
        QString name;
        QString path;
        QString password;

        void initialize(const QString &subj, const QString &n, const QString &p, const QString &pass) {
            subject = subj;
            name = n;
            path = p;
            password = pass;
        }
    };

    /* 绑定清单相关内容 */

    enum Subject { GradePublic = 0, ClassPublic, Chinese, Math, English, Physics, Chemistry, Biology, Geography, Politics, History };

    static QMap<Subject, QPair<QString, QString>> subjectMap;


    /* 设置文件相关内容 */
    static void ReadFromSetting();
    static void WriteToSetting();
    static QString FormatGradeClass();
    static ClassInfo ParseGradeClass(const QString &formattedStr);
    /* 绑定清单相关内容 */
    static SubjectData getDataFromJSON(const QString &subject);
    static QString getChineseName(const QString &englishName);
    static QString getEnglishName(const QString &chineseName);
    static void downloadJsonFile();
    static void uploadIPCfgFile();

};


#endif // BASEINFO_H
