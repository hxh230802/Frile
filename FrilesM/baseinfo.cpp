#include "baseinfo.h"

QString BaseInfo::settingFilePath = "";
QString BaseInfo::bindListPath = "";
QString BaseInfo::tmpDir = "";

BaseInfo::ClassInfo BaseInfo::classInfo = { 0, 0 };

QMap<BaseInfo::Subject, QPair<QString, QString>> BaseInfo::subjectMap = {
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

QMap<QString, QString> gradeMap = {
    {"高一", "1"},
    {"高二", "2"},
    {"高三", "3"}
};

bool CheckDirectory(const QString &path)
{
    QDir dir(path);

    if (!dir.exists())
    {
        return dir.mkpath(path);
    }

    return true;  // 目录已存在，返回true
}

void BaseInfo::initBaseInfo()
{
    settingFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/MainConfig.ini";
    bindListPath = QCoreApplication::applicationDirPath() + "/bindList.json";
    tmpDir = QCoreApplication::applicationDirPath() + "/FileTmp";

//    qDebug() << CheckDirectory(APPDATA_DIR) << APPDATA_DIR;
    CheckDirectory(tmpDir);
}

BaseInfo::BaseInfo()
{
}

void BaseInfo::ReadFromSetting()
{
    QSettings settings("Combox", "Digital Class\\Frile\\Friles");

    // 读取年级和班级信息
    classInfo.nGrade = settings.value("Grade", 0).toInt();
    classInfo.nClass = settings.value("Class", 0).toInt();

}

void BaseInfo::WriteToSetting()
{
    QSettings settings("Combox", "Digital Class\\Frile\\Friles");

    // 写入年级和班级信息
    settings.setValue("Grade", classInfo.nGrade);
    settings.setValue("Class", classInfo.nClass);
}

QString BaseInfo::FormatGradeClass()
{
    QString formattedGrade = gradeMap.key(QString::number(classInfo.nGrade));
    QString formattedClass = QString("（%1）班").arg(QString::number(classInfo.nClass));
    return formattedGrade + formattedClass;
}

BaseInfo::ClassInfo BaseInfo::ParseGradeClass(const QString &formattedStr)
{
    BaseInfo::ClassInfo classInfo;

    // 解析年级
    QString formattedGrade = formattedStr.split("（").first();  // 提取年级部分
    classInfo.nGrade = gradeMap.value(formattedGrade).toInt(); // 将年级字符串转换为对应的数值

    // 解析班级
    QString formattedClass = formattedStr.split("）").first().split("（").last();  // 提取班级部分
    classInfo.nClass = formattedClass.toInt();  // 将班级字符串转换为对应的整数值

    return classInfo;
}

BaseInfo::SubjectData BaseInfo::getDataFromJSON(const QString &subject)
{


    FtpManager* ftpMgr = new FtpManager(SERVER_IP);

    QByteArray jsonData = ftpMgr->readFileContent(QString("/Frile/Classes/ClassBlindList/%1-%2.json").arg(QString::number(classInfo.nGrade)).arg(QString::number(classInfo.nClass)));

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject rootObject = jsonDocument.object();

    QJsonArray subjectsArray = rootObject["Subjects"].toArray();

    for (const auto &subjectObject : subjectsArray) {
        QJsonObject obj = subjectObject.toObject();
        if (obj["Subject"].toString() == subject) {
            SubjectData data;
            data.subject = obj["Subject"].toString();
            data.name = obj["Name"].toString();
            data.path = obj["Path"].toString();
            data.password = obj["Password"].toString();
            return data;
        }
    }

    // 如果未找到科目，返回一个空的 SubjectData 结构体
    return SubjectData();
}

QString BaseInfo::getChineseName(const QString &englishName)
{
    for (auto it = subjectMap.begin(); it != subjectMap.end(); ++it) {
        if (it.value().second == englishName) {
            return it.value().first;
        }
    }

    // 如果没有找到匹配的英文名，返回空字符串或者其他适当的值
    return "";
}

QString BaseInfo::getEnglishName(const QString &chineseName)
{
    for (auto it = subjectMap.begin(); it != subjectMap.end(); ++it) {
        if (it.value().first == chineseName) {
            return it.value().second;
        }
    }

    // 如果没有找到匹配的中文名，返回空字符串或者其他适当的值
    return "";
}


void BaseInfo::downloadJsonFile()
{
    FtpManager* ftp = new FtpManager(SERVER_IP);
    qDebug() << QString("/Frile/Data/%1/%2/bindList.json").arg(QString::number(classInfo.nGrade)).arg(QString::number(classInfo.nClass)) << bindListPath;
    ftp->get(QString("/Frile/Data/%1/%2/bindList.json").arg(QString::number(classInfo.nGrade)).arg(QString::number(classInfo.nClass)),bindListPath);
    return;
}
#include <QHostInfo>
QString getHostIpAddress()
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

void BaseInfo::uploadIPCfgFile()
{
    FtpManager* ftp = new FtpManager(SERVER_IP);
    ftp->writeFileContent(QString("/Frile/Classes/IPList/%1-%2.config").arg(QString::number(classInfo.nGrade)).arg(QString::number(classInfo.nClass))
                             ,getHostIpAddress().toLatin1());
}
