#include "serialportobj.h"

#define READ_TIME 10
SerialPortObj::SerialPortObj(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
    byteReadBuffer = "";
    m_pReadTimer = new QTimer(this);
    m_iReadTimeout = READ_TIME;

    connect(serial,&QSerialPort::readyRead,this,&SerialPortObj::serialReadyRead);
    connect(m_pReadTimer,&QTimer::timeout,this,&SerialPortObj::serialReadTimeout);
}

SerialPortObj::~SerialPortObj()
{
    serial->close();
}

void SerialPortObj::setPortName(const QString &portName)      //配置端口号
{
    const QString name = portName.toUpper();
    serial->setPortName(name);
}
void SerialPortObj::setBaudRate(int &baudRate)      //配置波特率
{
    switch (baudRate) {
    case 115200:
        serial->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);
        break;
    case 57600:
        serial->setBaudRate(QSerialPort::Baud57600,QSerialPort::AllDirections);
        break;
    case 38400:
        serial->setBaudRate(QSerialPort::Baud38400,QSerialPort::AllDirections);
        break;
    case 19200:
        serial->setBaudRate(QSerialPort::Baud19200,QSerialPort::AllDirections);
        break;
    case 9600:
        serial->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);
        break;
    case 4800:
        serial->setBaudRate(QSerialPort::Baud4800,QSerialPort::AllDirections);
        break;
    case 2400:
        serial->setBaudRate(QSerialPort::Baud2400,QSerialPort::AllDirections);
        break;
    case 1200:
        serial->setBaudRate(QSerialPort::Baud1200,QSerialPort::AllDirections);
        break;
    default:
        emit serialError((tr("BaudRate Error!\n")));
        break;
    }
}
void SerialPortObj::setDataBits(int &dataBit)       //配置数据位
{
    switch (dataBit) {
    case 5:
        serial->setDataBits(QSerialPort::Data5);
        break;
    case 6:
        serial->setDataBits(QSerialPort::Data6);
        break;
    case 7:
        serial->setDataBits(QSerialPort::Data7);
        break;
    case 8:
        serial->setDataBits(QSerialPort::Data8);
        break;
    default:
        emit serialError(tr("DataBit Error!\n"));
        break;
    }
}
void SerialPortObj::setParity(QString &parityBit)         //配置校验位
{
    if(parityBit.isEmpty())
    {
        return;
    }

    QString str = parityBit.toLower();      //将字符串转换成小写字符串

    if(0<= str.indexOf("none",0))
        serial->setParity(QSerialPort::NoParity);
    else {
        if(0<= str.indexOf("odd",0))
            serial->setParity(QSerialPort::OddParity);
        else {
            if(0<= str.indexOf("even",0))
                serial->setParity(QSerialPort::EvenParity);
            else {
                if(0<= str.indexOf("mark",0))
                    serial->setParity(QSerialPort::MarkParity);
                else {
                    if(0<= str.indexOf("space",0))
                        serial->setParity(QSerialPort::SpaceParity);
                    else {
                        emit serialError(tr("ParityBit Error!\n"));
                    }
                }
            }
        }
    }
}
void SerialPortObj::setStopBits(QString &stopBit)       //配置停止位
{
    if(stopBit.isEmpty())
    {
        return;
    }

    QString str = stopBit.toLower();      //将字符串转换成小写字符串

    if(0<= str.indexOf("1",0))
        serial->setStopBits(QSerialPort::OneStop);
    else
    {
        if(0<= str.indexOf("1.5",0))
            serial->setStopBits(QSerialPort::OneAndHalfStop);
        else
        {
            if(0<= str.indexOf("2",0))
                serial->setStopBits(QSerialPort::TwoStop);
            else
            {
                emit serialError(tr("StopBit Error!\n"));
            }
        }
    }
}
void SerialPortObj::setDTR_RTS(bool setDTR,bool setRTS)
{
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->setDataTerminalReady(setDTR);
    serial->setRequestToSend(setRTS);
}

/*参数：端口号、波特率、数据位、校验位、停止位、DTR和RTS*/
bool SerialPortObj::openSerialPort(const QString &portName, int &baudRate,
                                      int &dataBit, QString &parityBit,
                                      QString &stopBit, bool setDTR, bool setRTS)
{
    setPortName(portName);
    if (serial->open(QIODevice::ReadWrite))
    {
        setBaudRate(baudRate);
        setDataBits(dataBit);
        setParity(parityBit);
        setStopBits(stopBit);
        setDTR_RTS(setDTR,setRTS);
        return true;
    }
    else
    {
        emit serialError(tr("Open failed, the serial port does not exist!\n"));
        return false;
    }
}
void SerialPortObj::closeSerialPort()
{
    serial->close();
}

/*串口读取数据*/
void SerialPortObj::serialPortRead(QString &readString, QString &prefix, QString &suffix)
{
    if (serial->bytesAvailable()<=0)
    {
        return;
    }
    QByteArray byteBuf;     //接收数据缓冲区
    byteBuf = serial->readAll();
    byteReadBuffer.append(byteBuf);
    if(!m_pReadTimer->isActive())
    {
        m_pReadTimer->start(m_iReadTimeout);
    }

    //如果无前缀无后缀，直接读取返回
    if(prefix.isEmpty() && suffix.isEmpty())
    {
//        byteReadBuffer = serial->readAll();
        readString = QString(byteReadBuffer);
        serialReadTimeout();
        return;
    }
    QByteArray pre = prefix.toLatin1();     //获得前缀
    QByteArray suf = suffix.toLatin1();     //获得后缀
    /*判断是否接收完毕*/
    //如果有前缀无后缀
    if(!prefix.isEmpty() && suffix.isEmpty())
    {
//        if(byteReadBuffer.contains(pre))
        if(0 == byteReadBuffer.indexOf(pre))
        {
            readString = QString(byteReadBuffer);
            serialReadTimeout();
            return;
        }
    }
    else
    {//如果无前缀有后缀
        if(prefix.isEmpty() && !suffix.isEmpty())
        {
            if(byteReadBuffer.size() == byteReadBuffer.lastIndexOf(suf) + suf.size())
            {
                readString = QString(byteReadBuffer);
                serialReadTimeout();
                return;
            }
        }
        else
        {//如果有前缀有后缀
            if(!prefix.isEmpty() && !suffix.isEmpty())
            {
                if(0 == byteReadBuffer.indexOf(pre)
                        && byteReadBuffer.size() == byteReadBuffer.lastIndexOf(suf) + suf.size())
                {
                    readString = QString(byteReadBuffer);
                    serialReadTimeout();//关闭超时定时器，并清空缓存
                    return;
                }
            }
        }
    }
    byteBuf.clear();
}

/*串口发送数据*/
void SerialPortObj::serialPortWrite(QString &writeString)
{
    if(writeString.isEmpty())
        return;
    QByteArray byteWrite = writeString.toLatin1();
    serial->write(byteWrite);
}

bool SerialPortObj::serialIsOpen()
{
    return serial->isOpen();
}

void SerialPortObj::getPortName(QList<QSerialPortInfo> &portInfoList)
{
    portInfoList = QSerialPortInfo::availablePorts();
}

void SerialPortObj::serialReadTimeout()
{
    if(m_pReadTimer->isActive())
    {
        m_pReadTimer->stop();
    }
    byteReadBuffer = "";
}

void SerialPortObj::setSerialReadTimeoutTime(const int &msec)
{
    m_iReadTimeout = msec;
    if(READ_TIME > m_iReadTimeout)//超时时间定义为不小于预设时间(ms)
    {
        m_iReadTimeout = READ_TIME;
    }
}
