#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioOutput>
#include <QPointer>

class AudioPlayer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AudioPlayer)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate)
    Q_PROPERTY(int channelCount READ channelCount WRITE setChannelCount)
    Q_PROPERTY(int sampleSize READ sampleSize WRITE setSampleSize)
    Q_PROPERTY(int byteOrder READ byteOrder WRITE setByteOrder)
    Q_PROPERTY(int sampleType READ sampleType WRITE setSampleType)
    Q_PROPERTY(QString codec READ codec WRITE setCodec)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume)
    Q_PROPERTY(QByteArray data READ data WRITE setData)
    Q_PROPERTY(bool error READ error)
    Q_PROPERTY(int state READ state)

private:
    QPointer<QAudioOutput> m_audioOutput;
    QAudioFormat m_format;
    qreal m_volume;
    QByteArray _data;

public:
    explicit AudioPlayer(QObject* parent = 0);
    int sampleRate() const;
    void setSampleRate(int rate);
    int channelCount() const;
    void setChannelCount(int count);
    int sampleSize() const;
    void setSampleSize(int size);
    int byteOrder() const;
    void setByteOrder(int order);
    int sampleType() const;
    void setSampleType(int type);
    const QString codec() const;
    void setCodec(const QString& codec);
    qreal volume() const;
    void setVolume(qreal volume);
    bool error() const;
    int state() const;
    void setData(const QByteArray& data);
    const QByteArray& data() const;

public slots:
    void play(); // play from beginning
    void pause(); // pause
    void resume(); // resume
    void stop(); // stop

signals:
    void stateChanged() const;

private slots:
    void handleStateChanged(QAudio::State newState);
};

#endif // AUDIOPLAYER_H
