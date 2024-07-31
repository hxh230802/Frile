#include "lessonmanager.h"

#include <QStandardPaths>
#include <QDir>

LessonManager::LessonManager(QObject *parent) : QObject(parent) { startLessonTimer(); }

void LessonManager::startLessonTimer()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LessonManager::checkLessonTime);
    timer->start(10000); // 定时器每分钟触发一次
}

void LessonManager::checkLessonTime()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTime = currentDateTime.toString("hh:mm:ss");

    // 获取Windows上的AppData目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // 解析路径并获取父目录
        QFileInfo fileInfo(appDataPath);
        QString Path = fileInfo.dir().path()+"/Schedule/Schedule_Info/red_lesson.json";

    qDebug() << currentTime<<Path;

    QFile file(Path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file: " << file.errorString();
        return;
    }

    QString jsonString = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Error parsing JSON: " << error.errorString();
        return;
    }

    QJsonObject lessonInfo = jsonDoc.object();

    QString startTimeStr = lessonInfo["start_time"].toString();
    startTime = QTime::fromString(startTimeStr, "hh:mm:ss");

    if (currentLesson != lessonInfo)
    {
        start = false;
        finish = false;
        currentLesson = lessonInfo;
    }

    if(currentDateTime.time() <= startTime && currentDateTime.time() >= startTime.addSecs(-180) && !start)
    {
        start = true;
        QString lessonName = lessonInfo["lesson_information"].toString();
        qDebug() << "It's almost time for the lesson " << lessonName << ". Lesson starts at " << startTimeStr;
        currentLesson = lessonInfo;
        emit lessonStart(lessonName);
    }

    if(currentDateTime.time() <= startTime.addSecs(900) && currentDateTime.time() >= startTime.addSecs(600) && !finish)
    {
        finish = true;
        emit goHome();
    }

}
