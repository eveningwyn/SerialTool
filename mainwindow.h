#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialobj.h"
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void setSerialPrefixSuffix(const QString& prefix, const QString& suffix);
    void openSerial(SerialPar serialPar);
    void closeSerial();
    void sendSerialData(QString strSendMsg);
    void setCommFileName(const QString& fileName);
    void setTimingFileName(const QString& fileName);
    void setIniFileName(const QString& fileName);

public slots:
    void log(const QString &msg, SHOW_MSG index);
    void serialIsOpen(const bool& isOpen);

private slots:
    void on_pushButton_refresh_clicked();

    void on_pushButton_open_close_clicked();

    void on_checkBox_saveLog_clicked();

    void on_pushButton_set_clicked();

    void on_pushButton_send_clicked();

    void on_pushButton_clearShow_clicked();

    void on_pushButton_loadCommFile_clicked();

    void on_pushButton_loadTimerFile_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    SerialObj *m_pSerial;
    QThread *m_pThread;
    QString m_strLogFileName;
    QString m_strPrefix;
    QString m_strSuffix;
    bool m_bSerialIsOpen;

    void init();
    void getPortName();
    void saveLog(QString& msg);
};

#endif // MAINWINDOW_H
