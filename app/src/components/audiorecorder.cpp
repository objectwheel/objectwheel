#include <audiorecorder.h>
#include <QIODevice>
#include <QDataStream>
#include <QtEndian>

AudioRecorder::AudioRecorder(QObject* parent)
	: QObject(parent)
	, m_audioInput(nullptr)
	, m_audioDevice(nullptr)
	, m_level(0)
{
	QAudioFormat format;
	format.setSampleRate(16000);
	format.setChannelCount(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	if (!QAudioDeviceInfo::defaultInputDevice().isFormatSupported(format)) qFatal("AudioRecorder : Error occurred.");

	m_audioInput = new QAudioInput(format, this);
	m_audioInput->setBufferSize(4096);
	connect(m_audioInput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanges(QAudio::State)));
}

bool AudioRecorder::recording() const
{
	if (m_audioInput->state() == QAudio::StoppedState) {
		return false;
	} else {
		return true;
	}
}

bool AudioRecorder::error() const
{
	if (m_audioInput->error() == QAudio::NoError) {
		return false;
	} else {
		return true;
	}
}

qreal AudioRecorder::level() const
{
	return m_level;
}

void AudioRecorder::record()
{
	m_audioDevice = m_audioInput->start();
	connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(handleBuffer()));
	emit recordingChanged();
}

void AudioRecorder::stop()
{
	m_audioDevice = nullptr;
	m_audioInput->stop();
	emit recordingChanged();
}

void AudioRecorder::setBufferSize(const int bufferSize)
{
	m_audioInput->setBufferSize(bufferSize);
}

int AudioRecorder::bufferSize() const
{
	return m_audioInput->bufferSize();
}

void AudioRecorder::handleStateChanges(const QAudio::State state)
{
	if (m_audioInput->error() != QAudio::NoError && state == QAudio::StoppedState) {
		m_audioDevice = nullptr;
		emit recordingChanged();
	}
}

void AudioRecorder::handleBuffer()
{
	if (m_audioDevice) {
		auto data = m_audioDevice->readAll();
		QDataStream stream(data);
		qint16 value = 0;
		qint16 max_value = 0;
		while(!stream.atEnd()) {
			stream.readRawData((char*)&value, sizeof(qint16));
			value = qFromLittleEndian<qint16>(value);
			max_value = qMax(value, max_value);
		}
		max_value = qMin(max_value, qint16(32767));
		m_level = qreal(max_value) / 32768;
		emit levelChanged();
		emit readyBuffer(data);
	}
}

#ifdef QT_QML_LIB
#include <QQmlEngine>
#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "AudioRecorder"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0
void AudioRecorder::registerQmlType()
{
	qmlRegisterType<AudioRecorder>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
}
#endif
