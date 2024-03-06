//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-2024 various contributors
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// https://opensource.org/license/artistic-2-0/
//
//=============================================================================
#include "media/video/videoplayer.h"

#ifndef AGS_NO_VIDEO_PLAYER

//-----------------------------------------------------------------------------
// VideoPlayer
//-----------------------------------------------------------------------------
namespace AGS
{
namespace Engine
{

using namespace Common;

VideoPlayer::~VideoPlayer()
{
    Stop();
}

HError VideoPlayer::Open(std::unique_ptr<Common::Stream> data_stream,
    const String &name, int flags)
{
    return Open(std::move(data_stream), name, flags, Size(), 0);
}

HError VideoPlayer::Open(std::unique_ptr<Common::Stream> data_stream,
    const String &name, int flags,
    const Size &target_sz, int target_depth)
{
    // We request a target depth from decoder, but it may ignore our request,
    // so we have to check actual "native" frame's depth after
    HError err = OpenImpl(std::move(data_stream), name, flags, target_depth);
    if (!err)
        return err;

    _name = name;
    _flags = flags;
    // Start the audio stream
    if (HasAudio())
    {
        if ((_audioFormat > 0) && (_audioChannels > 0) && (_audioFreq > 0))
        {
            _audioOut.reset(new OpenAlSource(_audioFormat, _audioChannels, _audioFreq));
        }
    }
    // Setup video
    if (HasVideo())
    {
        _targetDepth = target_depth > 0 ? target_depth : _frameDepth;
        SetTargetFrame(target_sz);
    }

    _frameTime = 1000 / _frameRate;
    return HError::None();
}

void VideoPlayer::SetTargetFrame(const Size &target_sz)
{
    _targetSize = target_sz.IsNull() ? _frameSize : target_sz;

    // Create helper bitmaps in case of stretching or color depth conversion
    if ((_targetSize != _frameSize) || (_targetDepth != _frameDepth))
    {
        _vframeBuf.reset(new Bitmap(_frameSize.Width, _frameSize.Height, _frameDepth));
    }
    else
    {
        _vframeBuf.reset();
    }

    // If we are decoding a 8-bit frame in a hi-color game, and stretching,
    // then create a hi-color buffer, as bitmap lib cannot stretch with depth change
    if ((_targetSize != _frameSize) && (_frameDepth == 8) && (_targetDepth > 8))
    {
        _hicolBuf.reset(BitmapHelper::CreateBitmap(_frameSize.Width, _frameSize.Height, _targetDepth));
    }
    else
    {
        _hicolBuf.reset();
    }

    // TODO: reset the buffered queue, and seek back?

    if (_videoReadyFrame)
    {
        auto old_frame = std::move(_videoReadyFrame);
        _videoReadyFrame.reset(new Bitmap(_targetSize.Width, _targetSize.Height, _targetDepth));
        _videoReadyFrame->StretchBlt(_videoReadyFrame.get(), RectWH(_targetSize));
    }
    else
    {
        _videoReadyFrame.reset(new Bitmap(_targetSize.Width, _targetSize.Height, _targetDepth));
        _videoReadyFrame->ClearTransparent();
    }
}

void VideoPlayer::Stop()
{
    if (IsPlaybackReady(_playState)) // keep any error state
        _playState = PlayStateStopped;

    // Shutdown openal source
    _audioOut.reset();
    // Close video decoder and free resources
    CloseImpl();

    _vframeBuf = nullptr;
    _hicolBuf = nullptr;
    _videoFramePool = std::stack<std::unique_ptr<Bitmap>>();
    _videoFrameQueue = std::deque<std::unique_ptr<Bitmap>>();
    _videoReadyFrame = nullptr;
}

void VideoPlayer::Play()
{
    if (!IsValid())
        return;

    switch (_playState)
    {
    case PlayStatePaused: Resume(); /* fall-through */
    case PlayStateInitial: _playState = PlayStatePlaying; break;
    default: break; // TODO: support rewind/replay from stop/finished state?
    }

    if (_audioOut)
        _audioOut->Play();
}

void VideoPlayer::Pause()
{
    if (_playState != PlayStatePlaying) return;

    if (_audioOut)
        _audioOut->Pause();
    _playState = PlayStatePaused;
}

void VideoPlayer::Resume()
{
    if (_playState != PlayStatePaused) return;

    if (_audioOut)
        _audioOut->Resume();
    _playState = PlayStatePlaying;
}

void VideoPlayer::Seek(float pos_ms)
{
    // TODO
}

std::unique_ptr<Common::Bitmap> VideoPlayer::GetReadyFrame()
{
    return std::move(_videoReadyFrame);
}

void VideoPlayer::ReleaseFrame(std::unique_ptr<Common::Bitmap> frame)
{
    _videoFramePool.push(std::move(frame));
}

bool VideoPlayer::Poll()
{
    if (!IsPlaybackReady(_playState))
        return false;

    // Buffer always when ready, even if we are paused
    if (HasVideo())
        BufferVideo();
    if (HasAudio())
        BufferAudio();

    if (_playState != PlayStatePlaying)
        return false;

    bool res_video = HasVideo() && ProcessVideo();
    bool res_audio = HasAudio() && ProcessAudio();

    // Stop if nothing is left to process, or if there was error
    if (_playState == PlayStateError)
        return false;
    if (!res_video && !res_audio)
    {
        _playState = PlayStateFinished;
        return false;
    }
    return true;
}

void VideoPlayer::BufferVideo()
{
    if (_videoFrameQueue.size() >= _videoQueueMax)
        return;

    // Get one frame from the pool, if present, otherwise allocate a new one
    std::unique_ptr<Bitmap> target_frame;
    if (_videoFramePool.empty())
    {
        target_frame.reset(new Bitmap(_targetSize.Width, _targetSize.Height, _targetDepth));
    }
    else
    {
        target_frame = std::move(_videoFramePool.top());
        _videoFramePool.pop();
    }

    // Try to retrieve one video frame from decoder
    const bool must_conv = (_targetSize != _frameSize || _targetDepth != _frameDepth);
    Bitmap *usebuf = must_conv ? _vframeBuf.get() : target_frame.get();
    if (!NextVideoFrame(usebuf))
    { // failed to get frame, so move prepared target frame into the pool for now
        _videoFramePool.push(std::move(target_frame));
        return;
    }

    // Convert frame if necessary
    if (must_conv)
    {
        // Use intermediate hi-color buffer if necessary
        if (_hicolBuf)
        {
            _hicolBuf->Blit(usebuf);
            usebuf = _hicolBuf.get();
        }
        
        if (_targetSize == _frameSize)
            target_frame->Blit(usebuf);
        else
            target_frame->StretchBlt(usebuf, RectWH(_targetSize));
    }

    // Push final frame to the queue
    _videoFrameQueue.push_back(std::move(target_frame));
}

void VideoPlayer::BufferAudio()
{
    if (_audioFrame)
        return; // still got one queued

    _audioFrame = NextAudioFrame();
}

bool VideoPlayer::ProcessVideo()
{
    // If has got a ready video frame, then test whether it's time to drop it
    if (_videoReadyFrame)
    {
        if (true /* test timestamp! */)
        {
            _videoFramePool.push(std::move(_videoReadyFrame));
        }
    }

    // If has not ready video frame, then test whether it's time to get
    // a new one from queue
    if (!_videoReadyFrame && _videoFrameQueue.size() > 0)
    {
        if (true /* test timestamp! */)
        {
            _videoReadyFrame = std::move(_videoFrameQueue.front());
            _videoFrameQueue.pop_front();
        }
    }

    // We are good so long as there's either a ready frame, or frames left in queue
    return _videoReadyFrame || !_videoFrameQueue.empty();
}

bool VideoPlayer::ProcessAudio()
{
    if (!_audioFrame)
        return false;

    assert(_audioOut);
    if (_audioOut->PutData(_audioFrame) > 0u)
    {
        _audioFrame = SoundBuffer(); // clear received buffer
    }
    _audioOut->Poll();
    return true;
}

} // namespace Engine
} // namespace AGS

#else // AGS_NO_VIDEO_PLAYER



#endif // !AGS_NO_VIDEO_PLAYER
