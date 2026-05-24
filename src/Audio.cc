#include "cthugha.h"
#include "Audio.h"
#include "Sound.h"
#include "imath.h"
#include "cth_buffer.h"
#include "network.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#if WITH_DSP == 1
#ifdef HAVE_LINUX_SOUNDCARD_H
#include <linux/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#endif

AudioInput::AudioInput()
    : error(0) { }

AudioInput::~AudioInput() { }

int AudioInput::rawBufferSize(int frameRawSize, int /*samplesRequested*/) const {
    return frameRawSize;
}

AudioOutput::~AudioOutput() { }

AudioProcessor::AudioProcessor(AudioInput* input_, int takeOwnership)
    : input(input_)
    , inputOwned(takeOwnership)
    , tmpData(NULL)
    , tmpSize(0)
    , rawSize(0)
    , bytesPerSample(0) {
    size = 1 << ilog2(max(BUFF_WIDTH, BUFF_HEIGHT));
    data = new char2[1024];
    memset(data, 0, 1024 * sizeof(char2));
    memset(dataProc, 0, 1024 * sizeof(char2));
    setTmpData();
}

AudioProcessor::~AudioProcessor() {
    delete[] tmpData;
    tmpData = NULL;

    delete[] data;
    data = NULL;

    if (inputOwned)
        delete input;
    input = NULL;
}

void AudioProcessor::setTmpData() {
    bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;
    rawSize = bytesPerSample * size;
    int requestedTmpSize = input ? input->rawBufferSize(rawSize, size) : rawSize;
    if (requestedTmpSize < rawSize)
        requestedTmpSize = rawSize;

    if (tmpSize < requestedTmpSize) {
        CTH_TRACE("audio processor: resizing raw buffer from %d to %d bytes (frame=%d)\n",
            tmpSize, requestedTmpSize, rawSize);
        delete[] tmpData;
        tmpSize = requestedTmpSize;
        tmpData = new char[tmpSize];
    }
}

void AudioProcessor::operator()() {
    if (input == NULL)
        return;

    bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;
    rawSize = bytesPerSample * size;
    setTmpData();

    int r = input->read(tmpData, rawSize, size);
    if (r < 0)
        r = 0;

    if (r >= 1024)
        convert(data, tmpData + (r - 1024) * bytesPerSample, 1024);
    else {
        memcpy(data, data + r, sizeof(char2) * (1024 - r));
        convert(data + 1024 - r, tmpData, r);
    }
}

void AudioProcessor::change() {
    if (input)
        input->update();
    tmpSize = 0;
    setTmpData();
}

void AudioProcessor::convert(char2* dst, void* src, int n) {
    unsigned char* data_u8 = (unsigned char*)src;
    char* data_s8 = (char*)src;
    unsigned short* data_u16 = (unsigned short*)src;
    short* data_s16 = (short*)src;

    int cInc = (soundChannels == 1) ? 0 : 1;

    switch (soundFormat) {
    case SF_u8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u8) - 128;
            data_u8 += cInc;
            dst[i][0] = int(*data_u8) - 128;
            data_u8++;
        }
        break;

    case SF_s8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_s8);
            data_s8 += cInc;
            dst[i][0] = int(*data_s8);
            data_s8++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) >> 8;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) >> 8;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) & 255;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) & 255;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) >> 8) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) >> 8) - 128;
            data_s16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) & 255) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) & 255) - 128;
            data_s16++;
        }
        break;

    default:
        CTH_ERROR("internal error: wrong sound format.\n");
    }
}

AudioRandomInput::AudioRandomInput()
    : AudioInput()
    , v1(0)
    , v2(0)
    , maxdv(2) {
    update();
}

int AudioRandomInput::read(char* dst, int rawSize, int samplesRequested) {
    int samples = min(samplesRequested, rawSize / int(sizeof(char2)));
    samples = min(samples, 256);

    if (samples <= 0)
        return 0;

    char2* sound_data = (char2*)dst;

    sound_data[0][0] = 144;
    sound_data[0][1] = 112;
    for (int x = 1; x < samples; x++) {
        if (rand() % 256 > sound_data[x - 1][0])
            v1 += rand() % maxdv;
        else
            v1 -= rand() % maxdv;

        if (rand() % 256 > sound_data[x - 1][1])
            v2 += rand() % maxdv;
        else
            v2 -= rand() % maxdv;

        sound_data[x][0] = sound_data[x - 1][0] + v1;
        sound_data[x][1] = sound_data[x - 1][1] + v2;
    }

    return samples;
}

void AudioRandomInput::update() {
    soundFormat.setValue(SF_u8);
    soundChannels.setValue(2);
}

#if WITH_NETWORK == 1

AudioNetInput::AudioNetInput()
    : AudioInput()
    , handle(-1) {
    CTH_INFO("Initializing net-sound...\n");
    CTH_TRACE("audio net input: init host=`%s'\n", SoundDeviceNet::sound_hostname);

    update();

    if ((handle = make_socket(SOCK_DGRAM, CLT_PORT)) < 0) {
        error = 1;
        return;
    }

    fcntl(handle, F_SETFL, O_NONBLOCK);
    netRequest(0);
}

void AudioNetInput::netRequest(int request) {
    struct sockaddr_in my_s_addr;
    struct hostent* hostinfo;
    int request_socket;
    char req[65];

    switch (request) {
    case 0:
        sprintf(req, "connect %d", CLT_PORT);
        break;
    case 1:
        sprintf(req, "disconnect %d", CLT_PORT);
        break;
    default:
        error = 1;
        return;
    }

    CTH_INFO("  Requesting: `%s'.\n", req);
    CTH_TRACE("audio net input: request `%s' to `%s'\n", req, SoundDeviceNet::sound_hostname);

    if ((request_socket = make_socket(SOCK_STREAM, CLT_PORT2)) < 0)
        CTH_ERRNO(errno, "Can not create request socket.");

    my_s_addr.sin_family = AF_INET;
    my_s_addr.sin_port = htons(SRV_PORT);
    hostinfo = gethostbyname(SoundDeviceNet::sound_hostname);
    if (hostinfo == NULL) {
        CTH_ERRNO(errno, "Could not find host `%s'.", SoundDeviceNet::sound_hostname);
        close(request_socket);
        error = 1;
        return;
    }
    my_s_addr.sin_addr = *(struct in_addr*)hostinfo->h_addr;

    if (connect(request_socket, (struct sockaddr*)&my_s_addr, sizeof(struct sockaddr_in)) < 0) {
        CTH_ERRNO(errno, "Can not connect to server `%s'.", SoundDeviceNet::sound_hostname);
        close(request_socket);
        return;
    }

    CTH_INFO("  Sending request `%s'\n", req);
    strcat(req, "\n");
    if (send(request_socket, req, 64, 0) <= 0)
        CTH_ERRNO(errno, "Can not send request.");

    sleep(1);

    if (shutdown(request_socket, 2))
        CTH_ERRNO(errno, "Can not shutdown request-socket.");

    if (close(request_socket))
        CTH_ERRNO(errno, "Can not close request-socket.");
}

int AudioNetInput::read(char* dst, int /*rawSize*/, int /*samplesRequested*/) {
    int r, rmax = 0;

    while ((r = recv(handle, dst, 1024, 0)) >= 0)
        if (r > rmax)
            rmax = r;

    return rmax / 2;
}

int AudioNetInput::rawBufferSize(int frameRawSize, int /*samplesRequested*/) const {
    return max(frameRawSize, 1024);
}

void AudioNetInput::update() {
    soundFormat.setValue(SF_u8);
    soundChannels.setValue(2);
}

AudioNetInput::~AudioNetInput() {
    if (handle >= 0) {
        netRequest(1);
        close(handle);
    }
}

#else

AudioNetInput::AudioNetInput()
    : AudioInput()
    , handle(-1) {
    CTH_ERROR("Network code was disabled at compile time.\n");
    error = 1;
}

AudioNetInput::~AudioNetInput() { }
void AudioNetInput::netRequest(int) { }
int AudioNetInput::read(char*, int, int) { return 0; }
int AudioNetInput::rawBufferSize(int frameRawSize, int) const { return frameRawSize; }
void AudioNetInput::update() { }

#endif

#if WITH_DSP == 1

AudioDSPInput::AudioDSPInput()
    : AudioInput()
    , handle(-1)
    , dmaBuffer(NULL)
    , dmaSize(0)
    , sampleWindow(1 << ilog2(max(BUFF_WIDTH, BUFF_HEIGHT))) {
    init();
}

void AudioDSPInput::setFragment() {
    int soundDSPFragment = (int(soundDSPFragments) << 16) | int(soundDSPFragmentSize);
    if (ioctl(handle, SNDCTL_DSP_SETFRAGMENT, &soundDSPFragment) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFRAGMENT failed.");

    soundDSPFragments.setValue(soundDSPFragment >> 16);
    soundDSPFragmentSize.setValue(soundDSPFragment & 0x7fff);

    if ((1 << soundDSPFragmentSize) * 2 * int(soundChannels) < sampleWindow)
        CTH_WARN("  sound fragment size is not set big enough.\n");
}

void AudioDSPInput::setChannels() {
    int channels = int(soundChannels) - 1;
    if (ioctl(handle, SNDCTL_DSP_STEREO, &channels) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed");
    soundChannels.setValue(channels + 1);
}

void AudioDSPInput::setSampleRate() {
    if (ioctl(handle, SNDCTL_DSP_SPEED, &(soundSampleRate.value)) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed");
}

void AudioDSPInput::setFormat() {
    int sound_format;

    switch (soundFormat) {
    case SF_u8:
        sound_format = AFMT_U8;
        break;
    case SF_s8:
        sound_format = AFMT_S8;
        break;
    case SF_u16_le:
        sound_format = AFMT_U16_LE;
        break;
    case SF_s16_le:
        sound_format = AFMT_S16_LE;
        break;
    case SF_u16_be:
        sound_format = AFMT_U16_BE;
        break;
    case SF_s16_be:
        sound_format = AFMT_S16_BE;
        break;
    default:
        CTH_ERROR("Internal error: unknown sound format.\n");
        sound_format = AFMT_U8;
    }

    CTH_TRACE("audio dsp input: setting sound format: %d   ", sound_format);

    if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0) {
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed. Trying 8bit unsigned");

        sound_format = AFMT_U8;
        if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed.");
    }

    CTH_TRACE("returned: %d\n", sound_format);

    switch (sound_format) {
    case AFMT_U8:
        soundFormat.setValue(SF_u8);
        break;
    case AFMT_S8:
        soundFormat.setValue(SF_s8);
        break;
    case AFMT_U16_LE:
        soundFormat.setValue(SF_u16_le);
        break;
    case AFMT_S16_LE:
        soundFormat.setValue(SF_s16_le);
        break;
    case AFMT_U16_BE:
        soundFormat.setValue(SF_u16_be);
        break;
    case AFMT_S16_BE:
        soundFormat.setValue(SF_s16_be);
        break;
    default:
        CTH_ERROR("Unknown sound format returned by SNDCTL_DSP_SETFMT %d.\n", sound_format);
        error = 1;
    }
}

void AudioDSPInput::init() {
    CTH_DEBUG("  setting %s for reading...\n", SoundDeviceDSP::dev_dsp);
    CTH_TRACE("audio dsp input: init method=%d sample-window=%d\n", int(soundDSPMethod), sampleWindow);

    if (handle >= 0)
        close(handle);
    handle = -1;

    if ((handle = open(SoundDeviceDSP::dev_dsp, O_RDONLY)) < 0) {
        CTH_ERRNO(errno, "Can't open `%s' for reading.", SoundDeviceDSP::dev_dsp);
        error = 1;
        return;
    }

    switch (int(soundDSPMethod)) {
    case 0:
        CTH_INFO("   Using sound method 0 - optimal fragment size\n");
        soundDSPFragmentSize.setValue(ilog2(sampleWindow) - 1);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;

    case 1:
        CTH_INFO("   Using sound method 1 - small fragment size\n");
        soundDSPFragments.setValue(2);
        soundDSPFragmentSize.setValue(4);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;

    case 2: {
        CTH_INFO("   Using sound method 2 - old version\n");
        int sound_div = 4;
        if (ioctl(handle, SNDCTL_DSP_SUBDIVIDE, &sound_div) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SUBDIVIDE failed.");

        int dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_STEREO, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed.");

        dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_SPEED, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed.");

        int sound_blkSize;
        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE failed.");

        setChannels();
        setSampleRate();
        setFormat();

        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE");
        break;
    }

    case 3:
        CTH_INFO("   Using sound method 3 - primitiv version\n");
        setFormat();
        setChannels();
        setSampleRate();
        break;

    case 4: {
        CTH_INFO("   Using sound method 4 - directly using DMA buffer\n");
        setFormat();
        setChannels();
        setSampleRate();

        soundDSPFragments.setValue(2);
        soundDSPFragmentSize.setValue(10);
        setFragment();

        struct audio_buf_info info;
        if (ioctl(handle, SNDCTL_DSP_GETISPACE, &info) == -1) {
            CTH_ERROR("ioctl: SNDCTL_DSP_GETISPACE failed.");
            error = 1;
            break;
        }

        dmaSize = info.fragstotal * info.fragsize;
        if (dmaSize < 2048)
            CTH_ERROR("Fragment size changed. New value (%d) is too small.\n"
                    "Please use a different sound method.\n",
                dmaSize);

        dmaBuffer = (char*)mmap(NULL, dmaSize, PROT_READ, MAP_FILE | MAP_SHARED, handle, 0);
        if (dmaBuffer == (char*)-1) {
            CTH_ERROR("mmap failed.\n");
            dmaBuffer = NULL;
            dmaSize = 0;
            error = 1;
            break;
        }

        int tmp = 0;
        ioctl(handle, SNDCTL_DSP_SETTRIGGER, &tmp);

        tmp = PCM_ENABLE_INPUT;
        ioctl(handle, SNDCTL_DSP_SETTRIGGER, &tmp);
        break;
    }

    default:
        CTH_ERROR("Unknown sound method %d.", int(soundDSPMethod));
        CTH_ERROR("   available sound methods:\n"
                "   0: sophisticated 1 (optimal fragment size)\n"
                "   1: sophisticated 2 (small fragments)\n"
                "   2: simple (small DMA buffer)\n"
                "   3: primitiv\n"
                "   4: directly use DMA buffer\n");
        error = 1;
        return;
    }
}

int AudioDSPInput::read(char* dst, int rawSize, int samplesRequested) {
    int r = 0;
    int bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;

    switch (int(soundDSPMethod)) {
    case 0: {
        audio_buf_info bi;
        const int nr_read = int(soundChannels) * samplesRequested / (1 << soundDSPFragmentSize);

        if (ioctl(handle, SNDCTL_DSP_GETISPACE, &bi) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETISPACE failed.");

        if (bi.fragments > nr_read) {
            for (int i = 0; i < bi.fragments - nr_read; i++)
                ::read(handle, dst, (1 << soundDSPFragmentSize));
            r = (1 << soundDSPFragmentSize) * nr_read;
        } else
            r = (1 << soundDSPFragmentSize) * bi.fragments;

        if (r == 0)
            r = (1 << soundDSPFragmentSize);

        if (::read(handle, dst, r) < 0)
            CTH_ERRNO(errno, "reading sound failed.");
        break;
    }

    case 1: {
        unsigned char* sbuff;
        int nr_read;
        for (nr_read = 0, sbuff = (unsigned char*)dst; nr_read < rawSize; nr_read += 32) {
            if (::read(handle, sbuff, 16) < 0)
                CTH_ERRNO(errno, "sound_read < 0");
            sbuff += 16;
            if (::read(handle, sbuff, 16) < 0)
                CTH_ERRNO(errno, "sound_read < 0");
            sbuff += 16;
        }
        r = nr_read;
        break;
    }

    case 2:
    case 3:
        r = ::read(handle, dst, rawSize);
        if (r < 0)
            CTH_ERRNO(errno, "get_sound: read < 0.");
        break;

    case 4:
        if (dmaBuffer != NULL) {
            memcpy(dst, dmaBuffer, 2048);
            r = 2048;
        }
        break;
    }

    if (int(soundDSPSync))
        ioctl(handle, SNDCTL_DSP_RESET);

    return r / bytesPerSample;
}

int AudioDSPInput::rawBufferSize(int frameRawSize, int samplesRequested) const {
    switch (int(soundDSPMethod)) {
    case 0:
        return max(frameRawSize, (1 << soundDSPFragmentSize) * 4);
    case 1:
        return max(frameRawSize, samplesRequested * 4 + 32);
    case 4:
        return max(frameRawSize, 2048);
    default:
        return frameRawSize;
    }
}

void AudioDSPInput::update() {
    if (dmaBuffer != NULL) {
        munmap(dmaBuffer, dmaSize);
        dmaBuffer = NULL;
        dmaSize = 0;
    }
    init();
}

int AudioDSPInput::initInputControls() {
    CTH_INFO("Initializing OSS mixer device...\n");
    return init_mixer();
}

AudioDSPInput::~AudioDSPInput() {
    if (dmaBuffer != NULL)
        munmap(dmaBuffer, dmaSize);
    dmaBuffer = NULL;
    dmaSize = 0;

    if (handle >= 0)
        close(handle);
}

#else

AudioDSPInput::AudioDSPInput()
    : AudioInput()
    , handle(-1)
    , dmaBuffer(NULL)
    , dmaSize(0)
    , sampleWindow(0) {
    CTH_ERROR("DSP device was disabled at compile time.\n");
    error = 1;
}

AudioDSPInput::~AudioDSPInput() { }
void AudioDSPInput::setFragment() { }
void AudioDSPInput::setChannels() { }
void AudioDSPInput::setSampleRate() { }
void AudioDSPInput::setFormat() { }
void AudioDSPInput::init() { }
int AudioDSPInput::read(char*, int, int) { return 0; }
int AudioDSPInput::rawBufferSize(int frameRawSize, int) const { return frameRawSize; }
void AudioDSPInput::update() { }
int AudioDSPInput::initInputControls() { return 0; }

#endif
