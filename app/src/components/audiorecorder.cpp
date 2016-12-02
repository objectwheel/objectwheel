#include <audiorecorder.h>
#include <QIODevice>

AudioRecorder::AudioRecorder(QObject* parent)
	: QObject(parent)
	, m_audioInput(nullptr)
	, m_audioDevice(nullptr)
{
	QAudioFormat format;
	format.setSampleRate(16000);
	format.setChannelCount(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	Q_ASSERT(QAudioDeviceInfo::defaultInputDevice().isFormatSupported(format));

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

void AudioRecorder::handleBuffer() const
{
	if (m_audioDevice) {
		emit readyBuffer(m_audioDevice->readAll());
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
