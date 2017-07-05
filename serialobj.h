#ifndef SERIALOBJ_H
#define SERIALOBJ_H

#include <QObject>
#include "serialportobj.h"
//#include "language.h"
#include <QMetaType>
#include <QEvent>

enum SHOW_MSG
{
    SHOW_SENDER,
    SHOW_RECEIVE,
    SHOW_NULL,
};
typedef struct
{
    QString portName;
    int baudRate;
    int dataBit;
    QString parityBit;
    QString stopBit;
    bool setDTR;
    bool setRTS;
}SerialPar;

class SerialObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialObj(QObject *parent = 0);

signals:
    void serialIsOpen(const bool &isOpen);
    void log(const QString &msg, SHOW_MSG index);

public slots:
    void init();
    void setSerialPrefixSuffix(const QString &prefix, const QString &suffix);
    void openSerial(SerialPar serialPar);
    void closeSerial();
    void sendSerialData(QString strSendMsg);
    void setCommFileName(const QString &fileName);
    void setTimingFileName(const QString &fileName);
    void setIniFileName(const QString &fileName);
    void serialReadyRead();
    void serialError(const QString &errorMsg);

private:
    SerialPortObj *m_pSerialPort;
    QString m_strPrefix;
    QString m_strSuffix;
    QString m_strCommFileName;
    QString m_strTimingFileName;
    QString m_strIniFileName;
    void checkTimerMsg(QString sendMsg);
    void checkMsgForRet(QString &msg, int &sleepTime);

protected:
    virtual void timerEvent(QTimerEvent *event);
};

#endif // SERIALOBJ_H
