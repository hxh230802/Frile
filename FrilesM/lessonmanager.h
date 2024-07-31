#ifndef LESSONMANAGER_H
#define LESSONMANAGER_H

#include <QObject>

#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QDateTime>

class QTimer;

class LessonManager : public QObject
{
    Q_OBJECT

public:
    explicit LessonManager(QObject *parent = nullptr);

    void startLessonTimer();

private slots:
    void checkLessonTime();

signals:
    void lessonStart(QString lessonName);
    void goHome();

private:
    QJsonObject currentLesson;
    QTime startTime;
    bool start = false;
    bool finish = false;
};

#endif // LESSONMANAGER_H
