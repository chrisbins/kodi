/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/AudioEngine/Interfaces/AESink.h"
#include "cores/AudioEngine/Utils/AEDeviceInfo.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "threads/CriticalSection.h"
#include "threads/SystemClock.h"
#include "threads/Thread.h"

#include <deque>
#include <set>

#include <androidjni/AudioTrack.h>

class CAESinkAUDIOTRACK : public IAESink
{
public:
  virtual const char *GetName() { return "AUDIOTRACK"; }

  CAESinkAUDIOTRACK();
  virtual ~CAESinkAUDIOTRACK();

  virtual bool Initialize(AEAudioFormat &format, std::string &device);
  virtual void Deinitialize();
  bool IsInitialized();

  virtual void         GetDelay        (AEDelayStatus& status);
  virtual double       GetLatency      ();
  virtual double       GetCacheTotal   ();
  virtual unsigned int AddPackets      (uint8_t **data, unsigned int frames, unsigned int offset);
  virtual void         AddPause        (unsigned int millis);
  virtual void         Drain           ();
  static void          EnumerateDevicesEx(AEDeviceInfoList &list, bool force = false);
  static void Register();
  static IAESink* Create(std::string &device, AEAudioFormat &desiredFormat);

protected:
  static jni::CJNIAudioTrack *CreateAudioTrack(int stream, int sampleRate, int channelMask, int encoding, int bufferSize);
  static bool IsSupported(int sampleRateInHz, int channelConfig, int audioFormat);
  static bool VerifySinkConfiguration(int sampleRate,
                                      int channelMask,
                                      int encoding,
                                      bool isRaw = false);
  static void UpdateAvailablePCMCapabilities();
  static void UpdateAvailablePassthroughCapabilities(bool isRaw = false);

  int AudioTrackWrite(char* audioData, int offsetInBytes, int sizeInBytes);
  int AudioTrackWrite(char* audioData, int sizeInBytes, int64_t timestamp);

private:
  jni::CJNIAudioTrack  *m_at_jni;
  int     m_jniAudioFormat;

  double                m_duration_written;
  unsigned int          m_min_buffer_size;
  uint64_t              m_headPos;
  uint64_t m_timestampPos = 0;
  // Moving Average computes the weighted average delay over
  // a fixed size of delay values - current size: 20 values
  double                GetMovingAverageDelay(double newestdelay);

  // We maintain our linear weighted average delay counter in here
  // The n-th value (timely oldest value) is weighted with 1/n
  // the newest value gets a weight of 1
  std::deque<double>   m_linearmovingaverage;

  static CAEDeviceInfo m_info;
  static CAEDeviceInfo m_info_raw;
  static CAEDeviceInfo m_info_iec;
  static std::set<unsigned int>       m_sink_sampleRates;
  static bool m_sinkSupportsFloat;

  AEAudioFormat      m_format;
  int16_t           *m_alignedS16;
  unsigned int       m_sink_frameSize;
  unsigned int       m_sink_sampleRate;
  bool               m_passthrough;
  double             m_audiotrackbuffer_sec;
  int                m_encoding;
  double m_pause_ms = 0.0;
  double m_delay = 0.0;
  double m_hw_delay = 0.0;
  CJNIAudioTimestamp m_timestamp;
  XbmcThreads::EndTime m_stampTimer;

  std::vector<float> m_floatbuf;
  std::vector<int16_t> m_shortbuf;
  std::vector<char> m_charbuf;
};
