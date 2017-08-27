#include "serialobj.h"
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QThread>
#include "language.h"

SerialObj::SerialObj(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<SerialPar>("SerialPar");//注册SerialPar类型
    qRegisterMetaType<SHOW_MSG>("SHOW_MSG");//注册SHOW_MSG类型
}
SerialObj::~SerialObj()
{
}
void SerialObj::init()
{
    m_pSerialPort = new SerialPortObj(this);
    m_strPrefix = "";
    m_strSuffix = "";
    m_strCommFileName = "";
    m_strTimingFileName = "";
    m_map.clear();
    connect(m_pSerialPort,&SerialPortObj::serialReadyRead,this,&SerialObj::serialReadyRead);
    connect(m_pSerialPort,&SerialPortObj::serialError,this,&SerialObj::serialError);
}
void SerialObj::setCommFileName(const QString &fileName)
{
    m_strCommFileName = fileName;
}
void SerialObj::setTimingFileName(const QString &fileName)
{
    m_strTimingFileName = fileName;
}
void SerialObj::setRegExpPattern(const QString &split)
{
    if(!split.isEmpty())
    {
        m_sPattern = QString("(.*)%1(.*)%2(.*)").arg(split).arg(split);
    }
}
void SerialObj::setSerialPrefixSuffix(const QString &prefix, const QString &suffix)
{
    m_strPrefix = prefix;
    m_strSuffix = suffix;
}
void SerialObj::openSerial(SerialPar serialPar)
{
    bool bIsOpen = m_pSerialPort->openSerialPort(serialPar.portName,
                                                 serialPar.baudRate,
                                                 serialPar.dataBit,
                                                 serialPar.parityBit,
                                                 serialPar.stopBit,
                                                 serialPar.setDTR,
                                                 serialPar.setRTS);
    emit serialIsOpen(bIsOpen);
    clearMap();
}
void SerialObj::sendSerialData(QString strSendMsg)
{
    if(m_pSerialPort->serialIsOpen())
    {
        if(!strSendMsg.isEmpty())
        {
            strSendMsg = QString("%1%2%3")
                    .arg(m_strPrefix).arg(strSendMsg).arg(m_strSuffix);
            m_pSerialPort->serialPortWrite(strSendMsg);
            emit log(strSendMsg,SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
                checkTimerMsg(strSendMsg);
        }
    }
}
void SerialObj::serialReadyRead()
{
    QString readString;
    m_pSerialPort->serialPortRead(readString,m_strPrefix,m_strSuffix);
    if(readString.isEmpty())
        return;
    emit log(readString,SHOW_RECEIVE);
    if(!m_strCommFileName.isEmpty())
    {
        int iSleepTime = 0;
        checkMsgForRet(readString,iSleepTime);
        if(!readString.isEmpty())
        {
            QThread::msleep(iSleepTime);
            QString strSendMsg = QString("%1%2%3")
                    .arg(m_strPrefix).arg(readString).arg(m_strSuffix);
            m_pSerialPort->serialPortWrite(strSendMsg);
            emit log(strSendMsg,SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
                checkTimerMsg(strSendMsg);
        }
    }
}
void SerialObj::checkTimerMsg(QString sendMsg)
{
    sendMsg.replace(m_strSuffix,"");
    QFile msgFile(m_strTimingFileName);
    if(msgFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream txtInput(&msgFile);
        QString strLine;
        QRegExp msgRE(m_sPattern);
        while(!txtInput.atEnd())
        {
            strLine = txtInput.readLine();
            if(0 <= strLine.indexOf(msgRE))
            {
                if(msgRE.cap(1).contains(sendMsg))
                {
                    QString msg_temp = msgRE.cap(2);
                    bool bOk;
                    int iTime = msgRE.cap(3).toInt(&bOk,10);
                    if(!bOk || 0>=iTime)
                        iTime = 1000;
                    QString strID = QString("%1").arg(this->startTimer(iTime));
                    m_map[strID] = msg_temp;
                    break;
                }
            }
        }
        msgFile.close();
    }
}
void SerialObj::timerEvent(QTimerEvent *event)
{
    QString strTimerID = QString("%1").arg(event->timerId());
    if(m_map.contains(strTimerID))
    {
        QString strSendMsg = m_map[strTimerID];
        if(!strSendMsg.isEmpty() && m_pSerialPort->serialIsOpen())
        {
            strSendMsg = QString("%1%2%3")
                    .arg(m_strPrefix).arg(strSendMsg).arg(m_strSuffix);
            m_pSerialPort->serialPortWrite(strSendMsg);
            emit log(strSendMsg,SHOW_SENDER);
            checkTimerMsg(strSendMsg);
        }
    }
    this->killTimer(event->timerId());
}
void SerialObj::checkMsgForRet(QString &msg, int &sleepTime)
{
    msg.replace(m_strSuffix,"");
    QString strTemp = msg;
    QFile msgFile(m_strCommFileName);
    if(msgFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream txtInput(&msgFile);
        QString strLine;
        QRegExp msgRE(m_sPattern);
        while(!txtInput.atEnd())
        {
            strLine = txtInput.readLine();
            if(0 <= strLine.indexOf(msgRE))
            {
                if(msgRE.cap(1).contains(msg))
                {
                    msg = msgRE.cap(2);
                    bool bOk;
                    sleepTime = msgRE.cap(3).toInt(&bOk,10);
                    if(!bOk || 0 > sleepTime)
                        sleepTime = 100;
                    break;
                }
            }
        }
        msgFile.close();
    }
    if(strTemp == msg)
    {
        msg = "";
        sleepTime = 0;
    }
}
void SerialObj::serialError(const QString &errorMsg)
{
    emit log(QString(tr("%1").arg(errorMsg)),SHOW_NULL);
}
void SerialObj::closeSerial()
{
    m_pSerialPort->closeSerialPort();
    emit serialIsOpen(false);
    clearMap();
}
void SerialObj::clearMap()
{
    if(!m_map.isEmpty())
    {
        m_map.clear();
    }
    m_strCommFileName = "";
    m_strTimingFileName = "";
}
