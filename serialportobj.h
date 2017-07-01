#ifndef SERIALPORTOBJ_H
#define SERIALPORTOBJ_H

/*需要在.pro文件添加QT += serialport*/
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialPortObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortObj(QObject *parent = 0);
    ~SerialPortObj();

    bool openSerialPort(const QString &portName,int &baudRate,
                        int &dataBit,QString &parityBit,QString &stopBit,
                        bool setDTR=true,bool setRTS=true);//参数：端口号、波特率、数据位、校验位、停止位、DTR和RTS
    void closeSerialPort();
    void setDTR_RTS(bool setDTR=true,bool setRTS=true);
    void serialPortRead(QString &readString,QString &prefix,QString &suffix);
    void serialPortWrite(QString &writeString);
    bool serialIsOpen();
    void getPortName(QList<QSerialPortInfo> &portInfoList);

signals:
    void serialReadReady();
    void serialError(QString &errorMsg);

public slots:

private slots:

private:
    QSerialPort *serial;
    QByteArray byteRead;    //存取串口读取的全部数据

    void setPortName(const QString &portName);
    void setBaudRate(int &baudRate);
    void setDataBits(int &dataBit);
    void setParity(QString &parityBit);
    void setStopBits(QString &stopBit);
};

#endif // SERIALPORTOBJ_H
