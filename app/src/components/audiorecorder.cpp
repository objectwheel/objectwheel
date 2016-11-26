#include <audiorecorder.h>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QAudioDecoder>
#include <QAudioRecorder>
#include <QStandardPaths>
#include <QCoreApplication>

#ifdef QT_QML_LIB
#include <QQmlEngine>
#endif

#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "AudioRecorder"
#define COMPONENT_DIR "/components/audio-recorder"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0

#define AUDIO_WAV_FILE "/audio.wav"

AudioRecorder::AudioRecorder(QObject* parent)
	: QObject(parent)
	, m_audioRecorder(new QAudioRecorder(this))
	, m_audioDecoder(new QAudioDecoder(this))
	, m_recording(false)
	, m_error(NoError)
{
	initPath();

	QAudioEncoderSettings audioSettings;
	audioSettings.setCodec("audio/PCM");
	audioSettings.setQuality(QMultimedia::HighQuality);
	audioSettings.setSampleRate(16000);
	audioSettings.setChannelCount(1);

	m_audioRecorder->setEncodingSettings(audioSettings);
	m_audioRecorder->setContainerFormat("wav");
	m_audioRecorder->setOutputLocation(QUrl::fromLocalFile(m_componentPath + AUDIO_WAV_FILE));
	connect(m_audioRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(handleError()));

	QAudioFormat format;
	format.setSampleRate(16000);
	format.setChannelCount(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	m_audioDecoder->setAudioFormat(format);
	m_audioDecoder->setSourceFilename(m_componentPath + AUDIO_WAV_FILE);
	connect(m_audioDecoder, SIGNAL(bufferReady()), this, SLOT(handleDecodingBuffer()));
	connect(m_audioDecoder, SIGNAL(finished()), this, SLOT(handleDecodingFinish()));
	connect(m_audioDecoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(handleDecodingError()));
}

AudioRecorder::Error AudioRecorder::hasError() const
{
	return m_error;
}

bool AudioRecorder::isRecording() const
{
	return m_recording;
}

const QByteArray& AudioRecorder::data() const
{
	return m_data;
}

void AudioRecorder::record()
{
	if (!m_recording) {
		m_audioRecorder->record();
		m_error = NoError;
		m_recording = true;
		emit recordingChanged();
	}
}

void AudioRecorder::stop()
{
	if (m_recording && m_audioDecoder->state() == QAudioDecoder::StoppedState) {
		m_audioRecorder->stop();
		m_audioDecoder->stop();
		startDecoding();
	}
}

void AudioRecorder::handleError()
{
	m_error = static_cast<Error>(m_audioDecoder->error());
	m_recording = false;
	emit recordingChanged();
}

void AudioRecorder::startDecoding()
{
	if (hasError() == NoError) {
		m_data.clear();
		m_audioDecoder->start();
	}
}

void AudioRecorder::handleDecodingBuffer()
{
	auto buffer = m_audioDecoder->read();
	auto data = buffer.constData<char>();
	auto length = buffer.byteCount();
	QDataStream out(&m_data, QIODevice::Append);
	out.writeRawData(data, length);
}

void AudioRecorder::handleDecodingFinish()
{
	m_recording = false;
	emit recordingChanged();
}

void AudioRecorder::handleDecodingError()
{
	m_data.clear();
	m_error = DecodeError;
	m_recording = false;
	emit recordingChanged();
}

void AudioRecorder::initPath()
{
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
	m_componentPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0);
#else
	m_componentPath = QCoreApplication::applicationDirPath();
#endif
	m_componentPath += COMPONENT_DIR;
	Q_ASSERT(QDir(m_componentPath).removeRecursively());
	Q_ASSERT(QDir().mkpath(m_componentPath));
}

void AudioRecorder::registerQmlType()
{
#ifdef QT_QML_LIB
	qmlRegisterType<AudioRecorder>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
#else
	qWarning("WARNING! AudioRecorder::registerQmlType() : QtQml module not included.");
#endif
}
