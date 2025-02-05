/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AudioDestinationMac_h
#define AudioDestinationMac_h

#include "AudioBus.h"
#include "AudioDestination.h"
#include "MediaSession.h"
#include <AudioUnit/AudioUnit.h>
#include <wtf/RefPtr.h>

namespace WebCore {

// An AudioDestination using CoreAudio's default output AudioUnit

class AudioDestinationMac : public AudioDestination, public MediaSessionClient {
public:
    AudioDestinationMac(AudioIOCallback&, float sampleRate);
    virtual ~AudioDestinationMac();

private:
    void configure();

    // DefaultOutputUnit callback
    static OSStatus inputProc(void* userData, AudioUnitRenderActionFlags*, const AudioTimeStamp*, UInt32 busNumber, UInt32 numberOfFrames, AudioBufferList* ioData);

    OSStatus render(UInt32 numberOfFrames, AudioBufferList* ioData);
    void setIsPlaying(bool);

    virtual MediaSession::MediaType mediaType() const override { return MediaSession::WebAudio; }
    virtual MediaSession::MediaType presentationType() const { return MediaSession::WebAudio; }
    virtual bool canReceiveRemoteControlCommands() const override { return false; }
    virtual void didReceiveRemoteControlCommand(MediaSession::RemoteControlCommandType) override { }
    virtual bool overrideBackgroundPlaybackRestriction() const override { return false; }

    virtual void start() override;
    virtual void stop() override;
    virtual bool isPlaying() override { return m_isPlaying; }

    virtual void pausePlayback() override { stop(); }
    virtual void resumePlayback() override { start(); }

    virtual float sampleRate() const override { return m_sampleRate; }

    AudioUnit m_outputUnit;
    AudioIOCallback& m_callback;
    RefPtr<AudioBus> m_renderBus;

    float m_sampleRate;
    bool m_isPlaying;

#if USE(AUDIO_SESSION)
    std::unique_ptr<MediaSession> m_mediaSession;
#endif
};

} // namespace WebCore

#endif // AudioDestinationMac_h
