#include <audioplayer.h>
#include <QDebug>
#include <QtCore/QBuffer>

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

void AudioPlayer::setSampleRate(int rate)
{
	m_format.setSampleRate(rate);
}

int AudioPlayer::channelCount() const
{
	return m_format.channelCount();
}

void AudioPlayer::setChannelCount(int count)
{
	m_format.setChannelCount(count);
}

int AudioPlayer::sampleSize() const
{
	return m_format.sampleSize();
}

void AudioPlayer::setSampleSize(int size)
{
	m_format.setSampleSize(size);
}

int AudioPlayer::byteOrder() const
{
	return int(m_format.byteOrder());
}

void AudioPlayer::setByteOrder(int order)
{
	m_format.setByteOrder(static_cast<QAudioFormat::Endian>(order));
}

int AudioPlayer::sampleType() const
{
	return int(m_format.sampleType());
}

void AudioPlayer::setSampleType(int type)
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

qreal AudioPlayer::volume() const
{
	return m_volume;
}

void AudioPlayer::setVolume(qreal volume)
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

	_data = data;

	m_audioOutput = new QAudioOutput(m_format, this);
	m_audioOutput->setVolume(m_volume);
	connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SIGNAL(stateChanged()));
	connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

	QBuffer* buffer = new QBuffer(m_audioOutput);
	buffer->setData(_data);
	buffer->open(QBuffer::ReadOnly);
	m_audioOutput->start(buffer);
	m_audioOutput->suspend();
}

const QByteArray&AudioPlayer::data() const
{
	return _data;
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

void AudioPlayer::resume()
{
	if (m_audioOutput) {
		m_audioOutput->resume();
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
			setData(_data);
			break;
		}

		default:
			break;
	}
}
