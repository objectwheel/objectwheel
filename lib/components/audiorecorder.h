#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QAudioInput>

class QIODevice;

class AudioRecorder : public QObject
{
		Q_OBJECT
		Q_PROPERTY(bool error READ error)
		Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
		Q_PROPERTY(int bufferSize READ bufferSize WRITE setBufferSize)
		Q_PROPERTY(qreal level READ level NOTIFY levelChanged)

	private:
		QAudioInput* m_audioInput;
		QIODevice* m_audioDevice;
		qreal m_level;

	public:
		explicit AudioRecorder(QObject* parent = 0);
		bool recording() const;
		bool error() const;
		qreal level() const;
		void setBufferSize(const int bufferSize);
		int bufferSize() const;

	public slots:
		void record();
		void stop();

	signals:
		void recordingChanged() const;
		void levelChanged() const;
		void readyBuffer(const QByteArray& data) const;

	private slots:
		void handleStateChanges(const QAudio::State state);
		void handleBuffer();

	public:
		#ifdef QT_QML_LIB
		static void registerQmlType();
		#endif
};

#endif // AUDIORECORDER_H
