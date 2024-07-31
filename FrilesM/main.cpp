#include "frilesm.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QScreen>
#include <myQLib_ProcessManager>

#include <Windows.h>
#include <shellapi.h>

void createBatchFile() {
    QString filePath = "start_program.bat"; // 设置 BAT 文件路径
    QFile batchFile(filePath);

    if (batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&batchFile);

        QString appPath = qApp->applicationFilePath();
        QString appDir = QCoreApplication::applicationDirPath();
        QFileInfo fileInfo(appDir);

        QString drive = fileInfo.absolutePath().left(2); // 提取盘符部分

        appPath.replace("/", "\\"); // 将斜杠替换为反斜杠

        // 写入启动程序的命令
        out << "@echo off" << endl;
        out << drive << endl;
        out << "timeout /t 0.5 >nul" << endl; // 延时 5 秒
        out << "\"" << appPath << "\"" <<endl;

        batchFile.close();
    }
}

int main(int argc, char *argv[])
{
    //#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    //    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //#endif
    //#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    //    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    //#endif

    QApplication a(argc, argv);

    createBatchFile();

    ProcessManager pmgr;
    if(pmgr.getProcessCount("FrilesM.exe")>1) return 0;

    FrilesM w;

    // 获取当前屏幕
    QScreen *screen = QGuiApplication::primaryScreen();

    // 获取当前屏幕的宽度和高度
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算窗口的宽度和高度
    int windowWidth = static_cast<int>(screenWidth * 0.335);  // 33.5%
    int windowHeight = static_cast<int>(screenHeight * 0.60);  // 60%

    // 计算窗口的位置，以左上角为参考点
    int x = static_cast<int>(screenWidth * 0.575);  // 左边距为100%-33.5%-9.0%
    int y = static_cast<int>(screenHeight * 0.325);  // 上边距为100%-60%-7.5%

    // 设置窗口大小和位置
    w.setGeometry(x, y, windowWidth, windowHeight);
    w.setFixedSize(QSize(windowWidth, windowHeight));

    // 设置窗口标志位：保持在最底层，排除在任务栏之外，隐藏任务栏图标，隐藏标题栏
    w.setWindowFlags(Qt::Desktop | Qt::WindowStaysOnBottomHint | Qt::Tool | Qt::SplashScreen | Qt::FramelessWindowHint);
    // 设置窗口透明
    w.setAttribute(Qt::WA_TranslucentBackground);

    w.show();

    int ret = a.exec();
    if (ret == 773) {
        // 获取当前应用程序路径
                QString program = QCoreApplication::applicationFilePath();

                // 启动一个新的实例
                QProcess::startDetached(program, QStringList());

        return 0;
    }

    return ret;
}
