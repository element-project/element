/*
    This file is part of the Kushview Modules for JUCE
    Copyright (c) 2014-2019  Kushview, LLC.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#if JUCE_DEBUG
 #define JACK_LOGGING_ENABLED 1
#endif

#if JACK_LOGGING_ENABLED
namespace {
void jack_Log (const String& s)
{
    Logger::writeToLog(s);
}

void dumpJackErrorMessage (const jack_status_t status)
{
    if (status & JackServerFailed || status & JackServerError)  jack_Log ("Unable to connect to JACK server");
    if (status & JackVersionError)      jack_Log ("Client's protocol version does not match");
    if (status & JackInvalidOption)     jack_Log ("The operation contained an invalid or unsupported option");
    if (status & JackNameNotUnique)     jack_Log ("The desired client name was not unique");
    if (status & JackNoSuchClient)      jack_Log ("Requested client does not exist");
    if (status & JackInitFailure)       jack_Log ("Unable to initialize client");
}
}
#else
#define dumpJackErrorMessage(a) {}
#define jack_Log(...) {}
#endif

class JackDevice   : public AudioIODevice
{
public:
    JackDevice (JackClient& client_, const String& deviceName,
                const String& inId, const String& outId)
        : AudioIODevice (deviceName, "JACK"),
          inputId (inId),
          outputId (outId),
          callback (nullptr),
          client (client_)
    {}

    ~JackDevice()
    {
        close();
    }

    
    StringArray getOutputChannelNames() override { return getChannelNames (false); }
    StringArray getInputChannelNames()  override { return getChannelNames (true); }

    Array<double> getAvailableSampleRates() override
    {
        Array<double> rates;
        const double _rates[] = { 44100.0, 48000.0 };
        rates.addArray (_rates, 2);
        return rates;
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> sizes;
        const int _sizes[] = { 256, 512, 1024, 2048 };
        sizes.addArray(_sizes, 4);
        return sizes;
    }
    
    int getDefaultBufferSize()          override { return client.getBufferSize(); }

    //=========================================================================

    String open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                 double /* sampleRate */, int /* bufferSizeSamples */) override
    {
        lastError = client.open (0);
        if (lastError.isNotEmpty())
        {
            jack_Log (lastError);
            return lastError;
        }
        
        jack_on_shutdown (client, JackDevice::shutdownCallback, this);    
        jack_set_error_function (JackDevice::errorCallback);
        jack_set_port_connect_callback (client, JackDevice::portConnectCallback, this);
        jack_set_port_registration_callback (client, JackDevice::portRegistrationCallback, this);
        jack_set_process_callback (client, JackDevice::processCallback, this);
        jack_set_thread_init_callback (client, JackDevice::threadInitCallback, this);
        jack_set_xrun_callback (client, JackDevice::xrunCallback, this);

        // client.registerPort ("audio_1", Jack::audioPort, JackPortIsOutput);
        // client.registerPort ("audio_2", Jack::audioPort, JackPortIsOutput);

        return lastError;
    }

    void close() override
    {
        lastError = client.close();
    }

    bool isOpen() override { return client.isOpen(); }

    void start (AudioIODeviceCallback* newCallback) override
    {
        if (client.isOpen() && newCallback != callback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            AudioIODeviceCallback* const oldCallback = callback;
            {
                const ScopedLock sl (callbackLock);
                callback = newCallback;

                if (callback != nullptr)
                    client.activate();
            }

            if (oldCallback != nullptr)
                oldCallback->audioDeviceStopped();
        }
    }

    void stop() override
    {
        start (nullptr);
        client.deactivate();
    }

    bool isPlaying() override { return callback != nullptr; }

    String getLastError()             override { return lastError; }

    //=========================================================================

    int getCurrentBufferSizeSamples() override { return client.getBufferSize(); }
    double getCurrentSampleRate()     override { return client.getSampleRate(); }
    int getCurrentBitDepth()          override { return 16; }
    
    BigInteger getActiveOutputChannels() const override { return 2; }
    BigInteger getActiveInputChannels()  const override { return 2; }

    int getOutputLatencyInSamples() override
    {
        int latency = 0;
        return latency;
    }

    int getInputLatencyInSamples() override
    {
        int latency = 0;
        return latency;
    }

    //=========================================================================

    bool hasControlPanel() const override { return false; }
    bool showControlPanel() override { return false; }
    bool setAudioPreprocessingEnabled (bool) override { return true; }
    int getXRunCount() const noexcept override { return 0; }

    //=========================================================================
    
    void portRegistration (jack_port_id_t portId, const bool wasRegistered)
    {
        auto* port = jack_port_by_id (client, portId);
        if (wasRegistered)
        {
            DBG ("[KV] port registered: " << jack_port_name (port));
        }
        else
        {
            DBG ("[KV] port deregistered: " << jack_port_name (port));
        }
    }

private:
    String inputId, outputId;

    StringArray getChannelNames (bool forInput) const
    {
        StringArray names;
        return names;
    }

    void process (jack_nframes_t nframes)
    {
        const ScopedLock sl (callbackLock);
        if (callback != nullptr)
        {
            // callback->audioDeviceIOCallback (
            //     nullptr, 0, nullptr, 0, static_cast<int> (nframes));
        }
    }

    static int processCallback (jack_nframes_t nframes, void* arg)
    {
        (static_cast<JackDevice*>(arg))->process (nframes);
        return 0;
    }

    static void portConnectCallback (jack_port_id_t, jack_port_id_t, int, void* arg)
    {
        jack_Log ("JackIODevice::portConnectCallback");
    }

    static void portRegistrationCallback (jack_port_id_t port, int reg, void *arg)
    {
        ((JackDevice*)arg)->portRegistration (port, reg > 0);
    }

    static void threadInitCallback (void* /* callbackArgument */)
    {
        jack_Log ("JackIODevice::threadInitCallback");
    }

    static void shutdownCallback (void* arg)
    {
        jack_Log ("JackIODevice::shutdown");
    }

    static void errorCallback (const char* msg)
    {
        jack_Log ("JackIODevice::errorCallback " + String (msg));
    }

    static void sendDeviceChangedCallback() {}

    static int xrunCallback (void* arg)
    {
        ignoreUnused (arg);
        return 0;
    }

    JackClient& client;

    String lastError;
    AudioIODeviceCallback* callback;
    CriticalSection callbackLock;
};

class JackDeviceType  : public AudioIODeviceType
{
public:
    JackDeviceType (JackClient* client_)
        : AudioIODeviceType ("JACK"),
          hasScanned (false),
          client (client_)
    {
        if (client == nullptr)
        {
            clientIsManaged = true;
            client = new JackClient();
        }

        inputNames.add ("default");
    }

    ~JackDeviceType()
    {
        deleteClientIfNeeded();
    }

    void deleteClientIfNeeded()
    {
        if (client != nullptr && clientIsManaged)
            client = nullptr;
        else
            client.release();
    }

    void scanForDevices() { hasScanned = true; }

    StringArray getDeviceNames (bool /* forInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return inputNames;
    }

    int getDefaultDeviceIndex (bool /* forInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return new JackDevice (*client, "clientname", "input", "output");
    }

    void portConnectionChange() { callDeviceChangeListeners(); }

private:
    StringArray inputNames, outputNames, inputIds, outputIds;
    bool hasScanned;
    ScopedPointer<JackClient> client;
    bool clientIsManaged;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JackDeviceType)
};

AudioIODeviceType* Jack::createAudioIODeviceType (JackClient* client)
{
    return new JackDeviceType (client);
}
