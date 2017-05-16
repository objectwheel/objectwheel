#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioOutput>
#include <QPointer>

class AudioPlayer : public QObject
{
		Q_OBJECT
		Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate)
		Q_PROPERTY(int channelCount READ channelCount WRITE setChannelCount)
		Q_PROPERTY(int sampleSize READ sampleSize WRITE setSampleSize)
		Q_PROPERTY(int byteOrder READ byteOrder WRITE setByteOrder)
		Q_PROPERTY(int sampleType READ sampleType WRITE setSampleType)
		Q_PROPERTY(QString codec READ codec WRITE setCodec)
		Q_PROPERTY(float volume READ volume WRITE setVolume)
		Q_PROPERTY(QByteArray data READ data WRITE setData)
		Q_PROPERTY(bool error READ error)
		Q_PROPERTY(int state READ state)

	private:
		QPointer<QAudioOutput> m_audioOutput;
		QAudioFormat m_format;
		float m_volume;
		QByteArray m_data;

	public:
		explicit AudioPlayer(QObject* parent = 0);
		int sampleRate() const;
		void setSampleRate(const int rate);
		int channelCount() const;
		void setChannelCount(const int count);
		int sampleSize() const;
		void setSampleSize(const int size);
		int byteOrder() const;
		void setByteOrder(const int order);
		int sampleType() const;
		void setSampleType(const int type);
		const QString codec() const;
		void setCodec(const QString& codec);
		float volume() const;
		void setVolume(const float volume);
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

	public:
		#ifdef QT_QML_LIB
		static void registerQmlType();
		#endif
};

#endif // AUDIOPLAYER_H
