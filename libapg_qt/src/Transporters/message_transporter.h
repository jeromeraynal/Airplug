#ifndef MESSAGE_TRANSPORTER_H
#define MESSAGE_TRANSPORTER_H

//Qt includes
#include <QString>
#include <QObject>

namespace AirPlug
{

class MessageTransporter : public QObject
{
    Q_OBJECT
public:

    enum UdpType
    {
        UniCast = 0,
        MultiCast,
    };

public:
    MessageTransporter(QObject* parent = nullptr);
    ~MessageTransporter();

    virtual void send(const QString& message) = 0;

public:

    Q_SIGNAL void signalMessageReceived(const QString& message);
    Q_SIGNAL void signalDisconnected();
};

}

#endif // MESSAGE_TRANSPORTER_H
