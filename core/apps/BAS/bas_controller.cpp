#include "bas_controller.h"

//Qt includes
#include <QTimer>
#include <QDebug>


using namespace AirPlug;

namespace BasApplication
{

class Q_DECL_HIDDEN BasController::Private
{
public:

    Private()
        : timer(nullptr),
          nbSequence(0)
    {
        // TODO: get default message ffrom QSettings
        messageToSend = QLatin1String("~");
    }

    ~Private()
    {
        delete timer;
    }

public:

    QTimer*               timer;

    QString               messageToSend;
    int                   nbSequence;
};



BasController::BasController(QObject* parent)
    : ApplicationController(QLatin1String("BAS"), parent),
      d(new Private)
{
}

BasController::~BasController()
{
    delete d;
}

void BasController::init(const QCoreApplication& app)
{
    ApplicationController::init(app);


    if (m_optionParser.autoSend && m_optionParser.delay > 0)
    {
        slotActivateTimer(m_optionParser.delay);
    }
}

void BasController::pause(bool b)
{
    m_optionParser.start = !b;

    if (b)
    {
        slotDeactivateTimer();
    }
}

void BasController::setMessage(const QString& msg)
{
    d->messageToSend = msg;

    ++(*m_clock);
}

void BasController::slotActivateTimer(int period)
{
    if (! d->timer)
    {
        d->timer = new QTimer(this);

        connect(d->timer, &QTimer::timeout,
                    this, &BasController::slotSendMessage);
    }

    m_optionParser.delay    = period;
    m_optionParser.autoSend = true;

    d->timer->start(period);

    ++(*m_clock);
}

void BasController::slotDeactivateTimer()
{
    m_optionParser.autoSend = false;
    m_optionParser.delay    = 0;

    if (d->timer)
    {
        d->timer->stop();
    }

    ++(*m_clock);
}

void BasController::slotPeriodChanged(int period)
{
    m_optionParser.delay = period;

    if (d->timer)
    {
        d->timer->setInterval(period);
    }

    ++(*m_clock);
}

void BasController::slotSendMessage()
{
    ++(*m_clock);

    Message message;

    // attache clock to the message
    message.addContent(QLatin1String("clock"), QJsonDocument(m_clock->convertToJson()).toJson(QJsonDocument::Compact));
    message.addContent(QLatin1String("payload"), d->messageToSend);
    message.addContent(QLatin1String("nseq"), QString::number(d->nbSequence));

    // TODO: get what, where, who from user interface
    sendMessage(message, QString(), QString(), QString());

    ++d->nbSequence;

    emit signalSequenceChange(d->nbSequence);
}

void BasController::slotReceiveMessage(Header header, Message message)
{
    QHash<QString, QString> contents = message.getContents();

    // read sender's clock
    if (contents.contains(QLatin1String("clock")))
    {
        QJsonObject jsonClock =  QJsonDocument::fromJson(contents[QLatin1String("clock")].toUtf8()).object();

        VectorClock senderClock(jsonClock);

        m_clock->updateClock(senderClock);
    }

    emit signalMessageReceived(header, message);
}

QJsonObject BasController::captureLocalState() const
{
    ++(*m_clock);

    QJsonObject applicationState;

    applicationState[QLatin1String("messageToSend")] = d->messageToSend;
    applicationState[QLatin1String("nbSequence")] = d->nbSequence;

    QJsonObject localState = m_clock->convertToJson();
    localState[QLatin1String("options")] = m_optionParser.convertToJson();
    localState[QLatin1String("state")]   = applicationState;

    return localState;
}

}
