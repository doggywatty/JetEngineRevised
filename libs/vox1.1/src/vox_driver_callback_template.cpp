// libs/vox1.1/src/vox_driver_callback_template.cpp
// The mixing core: every driver pulls its samples from DriverCallbackInterface::FillBuffer.
// The sources read their queued buffers with a linear interpolation at the pitch of the source (the track's rate over the
// driver's rate, times the pitch of the emitter and the doppler shift), with a gain that goes from the last one to the new one
// across the buffer (so that a change of gain does not click) and, for the 3D sounds, a distance gain and a constant power pan.
#include <algorithm>
#include <cmath>
#include "vox_driver.h"

namespace vox {

DriverCallbackInterface::DriverCallbackInterface()
    : m_sampleRate(44100), m_nextSourceId(0), m_masterGain(1.0f), m_masterGainStep(0.0f) {
    SetDefaultParameter();
}

DriverCallbackInterface::~DriverCallbackInterface() {
    for (std::list<DriverCallbackSourceInterface*>::iterator it = m_activeSources.begin(); it != m_activeSources.end(); ++it) delete *it;
}

void DriverCallbackInterface::SetDefaultParameter() {
    m_3DGeneralParameters = Vox3DGeneralParameters();
    m_listener = ListenerParameters();
}

void DriverCallbackInterface::Init(void *) { SetDefaultParameter(); }

void DriverCallbackInterface::Update(float dt) {
    ScopedLock lock(m_mutex);
    for (std::list<DriverCallbackSourceInterface*>::iterator it = m_activeSources.begin(); it != m_activeSources.end(); ++it) (*it)->Update(dt);
}

// the 3D parameters come as the ids of Vox3DGeneralParameterId and a pointer to the value; the listener ones (ids 100..)
// are the position, the velocity and the orientation (6 floats)
void DriverCallbackInterface::Set3DParameter(int id, const void *value) {
    ScopedLock lock(m_mutex);
    const float *f = static_cast<const float*>(value);
    switch (id) {
        case k3DGeneralDopplerFactor: m_3DGeneralParameters.dopplerFactor = *f; break;
        case k3DGeneralSpeedOfSound: m_3DGeneralParameters.speedOfSound = *f; break;
        case k3DGeneralDistanceModel: m_3DGeneralParameters.distanceModel = *static_cast<const unsigned*>(value); break;
        case k3DGeneralEnhanced3d: m_3DGeneralParameters.enhanced3d = *static_cast<const unsigned*>(value); break;
        case 100: m_listener.position = VoxVector3f(f[0], f[1], f[2]); break;
        case 101: m_listener.velocity = VoxVector3f(f[0], f[1], f[2]); break;
        case 102: m_listener.lookAt = VoxVector3f(f[0], f[1], f[2]); m_listener.up = VoxVector3f(f[3], f[4], f[5]); break;
        default: break;
    }
}

DriverSourceInterface *DriverCallbackInterface::CreateDriverSource() {
    ScopedLock lock(m_mutex);
    DriverCallbackSourceInterface *s = new DriverCallbackSourceInterface(m_sampleRate);
    m_activeSources.push_back(s);
    m_nextSourceId++;
    return s;
}

void DriverCallbackInterface::DestroyDriverSource(DriverSourceInterface *source) {
    ScopedLock lock(m_mutex);
    DriverCallbackSourceInterface *s = static_cast<DriverCallbackSourceInterface*>(source);
    m_activeSources.remove(s);
    delete s;
}

void DriverCallbackInterface::SetMasterGain(float gain) {
    ScopedLock lock(m_mutex);
    m_masterGainStep = gain;  // the target; FillBuffer ramps to it
}

void DriverCallbackInterface::FillBuffer(short *out, int frames) {
    ScopedLock lock(m_mutex);
    m_mixBuffer.assign((size_t)frames * 2, 0);
    for (std::list<DriverCallbackSourceInterface*>::iterator it = m_activeSources.begin(); it != m_activeSources.end(); ++it)
        (*it)->FillBuffer(&m_mixBuffer[0], frames, m_listener, m_3DGeneralParameters);
    const float from = m_masterGain, to = m_masterGainStep;
    for (int i = 0; i < frames; i++) {
        const float g = from + (to - from) * (float)(i + 1) / (float)frames;
        for (int c = 0; c < 2; c++) {
            const int v = (int)(m_mixBuffer[i * 2 + c] * g);
            out[i * 2 + c] = (short)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        }
    }
    m_masterGain = to;
}

DriverCallbackSourceInterface::DriverCallbackSourceInterface(int sampleRate)
    : m_driverSampleRate(sampleRate), m_state(kSourceStopped), m_userGain(1.0f), m_previousLeftGain(0.0f), m_previousRightGain(0.0f),
      m_userPitch(1.0f), m_numBuffer(4), m_bufferSize(16384), m_readPosition(0), m_processedBytes(0), m_bytesPerFrame(4), m_is3D(false) {
    m_lastFrame[0] = m_lastFrame[1] = 0;
}

DriverCallbackSourceInterface::~DriverCallbackSourceInterface() {}

void DriverCallbackSourceInterface::Init(const TrackParams &track, const DriverSourceParam &param) {
    ScopedLock lock(m_mutex);
    m_trackParams = track;
    m_numBuffer = param.numBuffer;
    m_bufferSize = param.bufferSize;
    m_bytesPerFrame = (track.numChannels > 0 ? track.numChannels : 1) * 2;
    m_buffers.clear();
    m_readPosition = 0;
    m_processedBytes = 0;
    m_state = kSourceStopped;
}

void DriverCallbackSourceInterface::Play() { ScopedLock lock(m_mutex); m_state = kSourcePlaying; m_previousLeftGain = m_previousRightGain = -1.0f; }

void DriverCallbackSourceInterface::Stop() {
    ScopedLock lock(m_mutex);
    m_state = kSourceStopped;
    m_buffers.clear();
    m_readPosition = 0;
}

void DriverCallbackSourceInterface::Pause() { ScopedLock lock(m_mutex); if (m_state == kSourcePlaying) m_state = kSourcePaused; }

void DriverCallbackSourceInterface::Reset() {
    ScopedLock lock(m_mutex);
    m_buffers.clear();
    m_readPosition = 0;
    m_processedBytes = 0;
    m_state = kSourceStopped;
}

int DriverCallbackSourceInterface::GetState() { ScopedLock lock(m_mutex); return m_state; }

long DriverCallbackSourceInterface::GetByteOffset() {
    ScopedLock lock(m_mutex);
    long b = m_processedBytes;
    if (!m_buffers.empty()) b += (long)m_readPosition * m_bytesPerFrame;
    return b;
}

bool DriverCallbackSourceInterface::NeedData() { ScopedLock lock(m_mutex); return (int)m_buffers.size() < m_numBuffer; }

void DriverCallbackSourceInterface::UploadData(const void *data, int bytes) {
    if (bytes < m_bytesPerFrame) return;
    ScopedLock lock(m_mutex);
    Buffer b;
    b.frames = bytes / m_bytesPerFrame;
    b.data.assign(static_cast<const short*>(data), static_cast<const short*>(data) + (size_t)b.frames * (m_bytesPerFrame / 2));
    m_buffers.push_back(b);
}

int DriverCallbackSourceInterface::GetQueuedBytes() {
    ScopedLock lock(m_mutex);
    long frames = 0;
    for (std::list<Buffer>::iterator it = m_buffers.begin(); it != m_buffers.end(); ++it) frames += it->frames;
    frames -= (long)m_readPosition;
    return (int)(frames * m_bytesPerFrame);
}

void DriverCallbackSourceInterface::SetGain(float gain) { ScopedLock lock(m_mutex); m_userGain = gain; }

void DriverCallbackSourceInterface::SetPitch(float pitch) { ScopedLock lock(m_mutex); m_userPitch = pitch; }

float DriverCallbackSourceInterface::GetGain() { return m_userGain; }

float DriverCallbackSourceInterface::GetPitch() { return m_userPitch; }

// the ids are those of Vox3DEmitterParameterId; the vectors are 3 floats
void DriverCallbackSourceInterface::Set3DParameter(int id, const void *value) {
    ScopedLock lock(m_mutex);
    const float *f = static_cast<const float*>(value);
    m_is3D = true;
    switch (id) {
        case k3DEmitterRelativeToListener: m_3DParameters.relativeToListener = *static_cast<const int*>(value); break;
        case k3DEmitterMaxDistance: m_3DParameters.maxDistance = *f; break;
        case k3DEmitterReferenceDistance: m_3DParameters.referenceDistance = *f; break;
        case k3DEmitterRolloffFactor: m_3DParameters.rolloffFactor = *f; break;
        case k3DEmitterInnerConeAngle: m_3DParameters.innerConeAngle = *f; break;
        case k3DEmitterOuterConeAngle: m_3DParameters.outerConeAngle = *f; break;
        case k3DEmitterOuterConeGain: m_3DParameters.outerConeGain = *f; break;
        case k3DEmitterPosition: m_position = VoxVector3f(f[0], f[1], f[2]); break;
        case k3DEmitterVelocity: m_velocity = VoxVector3f(f[0], f[1], f[2]); break;
        case k3DEmitterDirection: m_direction = VoxVector3f(f[0], f[1], f[2]); break;
        default: break;
    }
}

void DriverCallbackSourceInterface::Update(float) {}

// the gain of the distance by the model of the listener's general parameters
float DriverCallbackSourceInterface::GetDistanceGain(const VoxVector3f &rel, const Vox3DGeneralParameters &general) const {
    const float d = sqrtf(rel.x * rel.x + rel.y * rel.y + rel.z * rel.z);
    const float ref = m_3DParameters.referenceDistance, maxd = m_3DParameters.maxDistance, roll = m_3DParameters.rolloffFactor;
    float dist = d;
    const unsigned model = general.distanceModel;
    const bool clamped = model == 2 || model == 4 || model == 6;
    if (clamped) dist = std::min(std::max(d, ref), maxd);
    switch (model) {
        case 1: case 2: return ref / (ref + roll * (dist - ref));
        case 3: case 4: return maxd > ref ? 1.0f - roll * (dist - ref) / (maxd - ref) : 1.0f;
        case 5: case 6: return ref > 0 && dist > 0 ? powf(dist / ref, -roll) : 1.0f;
        default: return 1.0f;
    }
}

// a constant power pan from the side of the listener the sound is on (mono sources only)
void DriverCallbackSourceInterface::GetStereoPanning(const VoxVector3f &rel, float &left, float &right) const {
    const float d = sqrtf(rel.x * rel.x + rel.y * rel.y + rel.z * rel.z);
    float pan = 0.0f;
    if (d > 1e-5f) pan = rel.x / d;  // rel is in the listener's space: +x is the right
    pan = std::min(1.0f, std::max(-1.0f, pan));
    const float a = (pan + 1.0f) * 0.25f * 3.14159265f;  // 0..pi/2
    left = cosf(a);
    right = sinf(a);
}

// the shift of the pitch by the speeds of the source and the listener along the line between them
float DriverCallbackSourceInterface::GetDopplerPitch(const ListenerParameters &l, const Vox3DGeneralParameters &g) const {
    if (g.dopplerFactor <= 0.0f || g.speedOfSound <= 0.0f) return 1.0f;
    VoxVector3f sl(m_position.x - l.position.x, m_position.y - l.position.y, m_position.z - l.position.z);
    if (m_3DParameters.relativeToListener) sl = m_position;
    const float d = sqrtf(sl.x * sl.x + sl.y * sl.y + sl.z * sl.z);
    if (d < 1e-5f) return 1.0f;
    const float vl = (l.velocity.x * sl.x + l.velocity.y * sl.y + l.velocity.z * sl.z) / d;
    const float vs = (m_velocity.x * sl.x + m_velocity.y * sl.y + m_velocity.z * sl.z) / d;
    const float c = g.speedOfSound, f = g.dopplerFactor;
    const float a = std::min(vl, c / f - 1.0f), b = std::min(vs, c / f - 1.0f);
    return (c - f * a) / (c - f * b);
}

void DriverCallbackSourceInterface::FillBuffer(int *mix, int frames, const ListenerParameters &listener, const Vox3DGeneralParameters &general) {
    ScopedLock lock(m_mutex);
    if (m_state != kSourcePlaying || m_buffers.empty() || m_driverSampleRate <= 0) return;
    const int channels = m_trackParams.numChannels > 0 ? m_trackParams.numChannels : 1;
    float pitch = m_userPitch;
    float gainL = m_userGain, gainR = m_userGain;
    if (m_is3D && channels == 1) {
        VoxVector3f rel = m_position;
        if (!m_3DParameters.relativeToListener) {
            rel = VoxVector3f(m_position.x - listener.position.x, m_position.y - listener.position.y, m_position.z - listener.position.z);
            // into the space of the listener: x right, y up, z behind
            const VoxVector3f f = listener.lookAt, u = listener.up;
            const VoxVector3f r(f.y * u.z - f.z * u.y, f.z * u.x - f.x * u.z, f.x * u.y - f.y * u.x);
            rel = VoxVector3f(rel.x * r.x + rel.y * r.y + rel.z * r.z, rel.x * u.x + rel.y * u.y + rel.z * u.z, -(rel.x * f.x + rel.y * f.y + rel.z * f.z));
        }
        const float g = GetDistanceGain(rel, general);
        float l, r;
        GetStereoPanning(rel, l, r);
        gainL *= g * l * 1.41421356f;
        gainR *= g * r * 1.41421356f;
        pitch *= GetDopplerPitch(listener, general);
    } else if (m_is3D) {
        VoxVector3f rel = m_3DParameters.relativeToListener ? m_position : VoxVector3f(m_position.x - listener.position.x, m_position.y - listener.position.y, m_position.z - listener.position.z);
        const float g = GetDistanceGain(rel, general);
        gainL *= g;
        gainR *= g;
    }
    float prevL = m_previousLeftGain < 0 ? gainL : m_previousLeftGain, prevR = m_previousRightGain < 0 ? gainR : m_previousRightGain;
    const double ratio = (double)m_trackParams.sampleRate / (double)m_driverSampleRate * pitch;
    for (int i = 0; i < frames && !m_buffers.empty(); i++) {
        Buffer &b = m_buffers.front();
        const int i0 = (int)m_readPosition;
        const float frac = (float)(m_readPosition - i0);
        const short *s0 = &b.data[(size_t)i0 * channels];
        const short *s1 = s0;
        if (i0 + 1 < b.frames) s1 = &b.data[(size_t)(i0 + 1) * channels];
        else if (m_buffers.size() > 1) {
            std::list<Buffer>::iterator next = m_buffers.begin();
            ++next;
            s1 = &next->data[0];
        }
        float l = s0[0] + (s1[0] - s0[0]) * frac, r = l;
        if (channels > 1) r = s0[1] + (s1[1] - s0[1]) * frac;
        const float t = (float)(i + 1) / (float)frames;
        const float gl = prevL + (gainL - prevL) * t, gr = prevR + (gainR - prevR) * t;
        mix[i * 2] += (int)(l * gl);
        mix[i * 2 + 1] += (int)(r * gr);
        m_readPosition += ratio;
        while (!m_buffers.empty() && m_readPosition >= m_buffers.front().frames) {
            m_readPosition -= m_buffers.front().frames;
            m_processedBytes += (long)m_buffers.front().frames * m_bytesPerFrame;
            m_buffers.pop_front();
        }
        if (m_buffers.empty()) m_readPosition = 0;
    }
    m_previousLeftGain = gainL;
    m_previousRightGain = gainR;
}

}  // namespace vox
