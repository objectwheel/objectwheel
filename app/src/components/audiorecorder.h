#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QAudioInput>

class QIODevice;

class AudioRecorder : public QObject
{
		Q_OBJECT
		Q_PROPERTY(bool error READ error)
		Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
		Q_PROPERTY(int bufferSize WRITE setBufferSize)

	private:
		QAudioInput* m_audioInput;
		QIODevice* m_audioDevice;

	public:
		explicit AudioRecorder(QObject* parent = 0);
		bool recording() const;
		bool error() const;
		void setBufferSize(const int bufferSize);

	public slots:
		void record();
		void stop();

	signals:
		void recordingChanged() const;
		void readyBuffer(const QByteArray& data) const;

	private slots:
		void handleStateChanges(const QAudio::State state);
		void handleBuffer() const;

	public:
		#ifdef QT_QML_LIB
		static void registerQmlType();
		#endif
};

#endif // AUDIORECORDER_H
