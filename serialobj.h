#ifndef SERIALOBJ_H
#define SERIALOBJ_H

#include <QObject>
#include "serialportobj.h"
//#include "language.h"
#include <QMetaType>
#include <QEvent>
#include <QMap>

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
    ~SerialObj();

signals:
    void serialIsOpen(const bool &isOpen);
    void log(const QString &msg, SHOW_MSG index);

public slots:
    void init();
    void setSerialPrefixSuffix(const QString &prefix, const QString &suffix);
    void setCommFileName(const QString &fileName);
    void setTimingFileName(const QString &fileName);
    void openSerial(SerialPar serialPar);
    void closeSerial();
    void sendSerialData(QString strSendMsg);
    void serialReadyRead();
    void serialError(const QString &errorMsg);
    void setRegExpPattern(const QString &split);

private:
    SerialPortObj *m_pSerialPort;
    QString m_strPrefix;
    QString m_strSuffix;
    QString m_strCommFileName;
    QString m_strTimingFileName;
    QString m_sPattern;
    QMap <QString, QString> m_map;
    void clearMap();
    void checkTimerMsg(QString sendMsg);
    void checkMsgForRet(QString &msg, int &sleepTime);

protected:
    virtual void timerEvent(QTimerEvent *event);
};

#endif // SERIALOBJ_H
