#include <audioplayer.h>
#include <QDebug>
#include <QBuffer>

AudioPlayer::AudioPlayer(QObject* parent)
	: QObject(parent)
	, m_volume(0.8)
{
	m_format.setSampleRate(22050);
	m_format.setChannelCount(1);
	m_format.setSampleSize(16);
	m_format.setCodec("audio/pcm");
	m_format.setByteOrder(QAudioFormat::LittleEndian);
	m_format.setSampleType(QAudioFormat::SignedInt);
}

int AudioPlayer::sampleRate() const
{
	return m_format.sampleRate();
}

void AudioPlayer::setSampleRate(const int rate)
{
	m_format.setSampleRate(rate);
}

int AudioPlayer::channelCount() const
{
	return m_format.channelCount();
}

void AudioPlayer::setChannelCount(const int count)
{
	m_format.setChannelCount(count);
}

int AudioPlayer::sampleSize() const
{
	return m_format.sampleSize();
}

void AudioPlayer::setSampleSize(const int size)
{
	m_format.setSampleSize(size);
}

int AudioPlayer::byteOrder() const
{
	return int(m_format.byteOrder());
}

void AudioPlayer::setByteOrder(const int order)
{
	m_format.setByteOrder(static_cast<QAudioFormat::Endian>(order));
}

int AudioPlayer::sampleType() const
{
	return int(m_format.sampleType());
}

void AudioPlayer::setSampleType(const int type)
{
	m_format.setSampleType(static_cast<QAudioFormat::SampleType>(type));
}

const QString AudioPlayer::codec() const
{
	return m_format.codec();
}

void AudioPlayer::setCodec(const QString& codec)
{
	m_format.setCodec(codec);
}

float AudioPlayer::volume() const
{
	return m_volume;
}

void AudioPlayer::setVolume(const float volume)
{
	m_volume = volume;
}

bool AudioPlayer::error() const
{
	if (m_audioOutput && m_audioOutput->error() != QAudio::NoError) {
		return true;
	} else {
		return false;
	}
}

int AudioPlayer::state() const
{
	if (m_audioOutput) {
		return int(m_audioOutput->state());
	} else {
		return -1;
	}
}

void AudioPlayer::setData(const QByteArray& data)
{
	if (data.isEmpty()) {
		return;
	}

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(m_format)) {
		qWarning() << " AudioPlayer::start() : Raw audio format not supported by backend, cannot play audio.";
		return;
	}

	if (m_audioOutput) {
		m_audioOutput->deleteLater();
	}

	m_data = data;

	m_audioOutput = new QAudioOutput(m_format, this);
	m_audioOutput->setVolume(m_volume);
	connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SIGNAL(stateChanged()));
	connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

	QBuffer* buffer = new QBuffer(m_audioOutput);
	buffer->setData(m_data);
	buffer->open(QBuffer::ReadOnly);
	m_audioOutput->start(buffer);
	m_audioOutput->suspend();
}

const QByteArray&AudioPlayer::data() const
{
	return m_data;
}

void AudioPlayer::play()
{
	if (m_audioOutput) {
		m_audioOutput->resume();
	}
}

void AudioPlayer::pause()
{
	if (m_audioOutput) {
		m_audioOutput->suspend();
	}
}

void AudioPlayer::stop()
{
	if (m_audioOutput) {
		m_audioOutput->stop();
	}
}

void AudioPlayer::handleStateChanged(QAudio::State newState)
{
	switch (newState) {
		case QAudio::IdleState:
		case QAudio::StoppedState: {
			setData(m_data);
			break;
		}

		default:
			break;
	}
}

#ifdef QT_QML_LIB
#include <QQmlEngine>
#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "AudioPlayer"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0
void AudioPlayer::registerQmlType()
{
	qmlRegisterType<AudioPlayer>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
}
#endif
