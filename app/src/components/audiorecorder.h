#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QAudioRecorder>

class QAudioDecoder;

class AudioRecorder : public QObject
{
	public:
		enum Error {
			NoError,
			ResourceError,
			FormatError,
			OutOfSpaceError,
			DecodeError
		};
		Q_ENUMS(Error)

	private:
		Q_OBJECT
		Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
		Q_PROPERTY(Error error READ hasError)
		Q_PROPERTY(QByteArray data READ data)

	private:
		QString m_componentPath;
		QAudioRecorder* m_audioRecorder;
		QAudioDecoder* m_audioDecoder;
		bool m_recording;
		QByteArray m_data;
		Error m_error;

	public:
		explicit AudioRecorder(QObject* parent = 0);
		bool isRecording() const;
		Error hasError() const;
		const QByteArray& data() const;
		static void registerQmlType();

	private:
		void initPath();

	public slots:
		void record();
		void stop();

	private slots:
		void handleError();
		void startDecoding();
		void handleDecodingBuffer();
		void handleDecodingFinish();
		void handleDecodingError();

	signals:
		void recordingChanged();

};

#endif // AUDIORECORDER_H
