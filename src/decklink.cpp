#include <napi.h>
#include "DeckLinkAPI.h"

Napi::Value GetAvailableDevices(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!deckLinkIterator) {
        Napi::TypeError::New(env, "Unable to create DeckLink iterator").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = nullptr;
    int deviceCount = 0;

    while (deckLinkIterator->Next(&deckLink) == S_OK) {
        if (deckLink) {
            deviceCount++;
            deckLink->Release();
        }
    }

    deckLinkIterator->Release();
    return Napi::Number::New(env, deviceCount);
}

Napi::Value GetDeviceOutputs(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Device index required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!deckLinkIterator) {
        Napi::TypeError::New(env, "Unable to create DeckLink iterator").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = nullptr;
    HRESULT result = S_OK;
    int currentIndex = 0;
    while (currentIndex <= deviceIndex) {
        result = deckLinkIterator->Next(&deckLink);
        if (result != S_OK || !deckLink) {
            deckLinkIterator->Release();
            Napi::TypeError::New(env, "Device index out of range").ThrowAsJavaScriptException();
            return env.Null();
        }
        if (currentIndex < deviceIndex) {
            deckLink->Release();
            deckLink = nullptr;
        }
        currentIndex++;
    }
    deckLinkIterator->Release();

    // Check if device has output capability by trying to get the output interface
    IDeckLinkOutput* deckLinkOutput = nullptr;
    int outputCount = 0;

    if (deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&deckLinkOutput) == S_OK) {
        outputCount = 1; // If we can get the output interface, device has at least 1 output
        deckLinkOutput->Release();
    }

    deckLink->Release();
    return Napi::Number::New(env, outputCount);
}

Napi::Value GetDeviceTemperature(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Device index required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!deckLinkIterator) {
        Napi::TypeError::New(env, "Unable to create DeckLink iterator").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = nullptr;
    HRESULT result = S_OK;
    int currentIndex = 0;
    while (currentIndex <= deviceIndex) {
        result = deckLinkIterator->Next(&deckLink);
        if (result != S_OK || !deckLink) {
            deckLinkIterator->Release();
            Napi::TypeError::New(env, "Device index out of range").ThrowAsJavaScriptException();
            return env.Null();
        }
        if (currentIndex < deviceIndex) {
            deckLink->Release();
            deckLink = nullptr;
        }
        currentIndex++;
    }
    deckLinkIterator->Release();

    IDeckLinkStatus* deckLinkStatus = nullptr;
    int64_t temperature = 0;
    if (deckLink->QueryInterface(IID_IDeckLinkStatus, (void**)&deckLinkStatus) == S_OK) {
        if (deckLinkStatus->GetInt(bmdDeckLinkStatusDeviceTemperature, &temperature) == S_OK) {
            deckLinkStatus->Release();
            deckLink->Release();
            return Napi::Number::New(env, temperature); // tenths of a degree Celsius
        }
        deckLinkStatus->Release();
    }
    deckLink->Release();
    Napi::TypeError::New(env, "Unable to get device temperature").ThrowAsJavaScriptException();
    return env.Null();
}

// Helper to get IDeckLink* for a given device index. Caller must Release() the returned pointer.
static IDeckLink* GetDeckLinkByIndex(Napi::Env env, int deviceIndex) {
    IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!deckLinkIterator) {
        Napi::TypeError::New(env, "Unable to create DeckLink iterator").ThrowAsJavaScriptException();
        return nullptr;
    }

    IDeckLink* deckLink = nullptr;
    int currentIndex = 0;
    while (currentIndex <= deviceIndex) {
        HRESULT result = deckLinkIterator->Next(&deckLink);
        if (result != S_OK || !deckLink) {
            deckLinkIterator->Release();
            Napi::TypeError::New(env, "Device index out of range").ThrowAsJavaScriptException();
            return nullptr;
        }
        if (currentIndex < deviceIndex) {
            deckLink->Release();
            deckLink = nullptr;
        }
        currentIndex++;
    }
    deckLinkIterator->Release();
    return deckLink;
}

Napi::Value GetDuplexMode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Device index required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = GetDeckLinkByIndex(env, deviceIndex);
    if (!deckLink) return env.Null();

    IDeckLinkProfileAttributes* attrs = nullptr;
    if (deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&attrs) == S_OK) {
        int64_t duplexMode = 0;
        if (attrs->GetInt(BMDDeckLinkDuplex, &duplexMode) == S_OK) {
            attrs->Release();
            deckLink->Release();
            // Return string representation
            const char* modeStr = "unknown";
            switch ((BMDDuplexMode)duplexMode) {
                case bmdDuplexFull:     modeStr = "full"; break;
                case bmdDuplexHalf:     modeStr = "half"; break;
                case bmdDuplexSimplex:  modeStr = "simplex"; break;
                case bmdDuplexInactive: modeStr = "inactive"; break;
            }
            return Napi::String::New(env, modeStr);
        }
        attrs->Release();
    }

    deckLink->Release();
    Napi::TypeError::New(env, "Unable to get duplex mode").ThrowAsJavaScriptException();
    return env.Null();
}

Napi::Value SetDuplexMode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Device index and profile string required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    std::string profileStr = info[1].As<Napi::String>().Utf8Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    BMDProfileID targetProfile;
    if (profileStr == "one-sub-full") {
        targetProfile = bmdProfileOneSubDeviceFullDuplex;
    } else if (profileStr == "one-sub-half") {
        targetProfile = bmdProfileOneSubDeviceHalfDuplex;
    } else if (profileStr == "two-sub-full") {
        targetProfile = bmdProfileTwoSubDevicesFullDuplex;
    } else if (profileStr == "two-sub-half") {
        targetProfile = bmdProfileTwoSubDevicesHalfDuplex;
    } else if (profileStr == "four-sub-half") {
        targetProfile = bmdProfileFourSubDevicesHalfDuplex;
    } else {
        Napi::TypeError::New(env, "Invalid profile. Use: one-sub-full, one-sub-half, two-sub-full, two-sub-half, four-sub-half").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = GetDeckLinkByIndex(env, deviceIndex);
    if (!deckLink) return env.Null();

    IDeckLinkProfileManager* profileManager = nullptr;
    if (deckLink->QueryInterface(IID_IDeckLinkProfileManager, (void**)&profileManager) != S_OK) {
        deckLink->Release();
        Napi::TypeError::New(env, "Device does not support profile management").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLinkProfile* profile = nullptr;
    if (profileManager->GetProfile(targetProfile, &profile) != S_OK || !profile) {
        profileManager->Release();
        deckLink->Release();
        Napi::TypeError::New(env, "Profile not available on this device").ThrowAsJavaScriptException();
        return env.Null();
    }

    HRESULT result = profile->SetActive();
    profile->Release();
    profileManager->Release();
    deckLink->Release();

    if (result != S_OK) {
        Napi::TypeError::New(env, "Failed to activate profile").ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::Boolean::New(env, true);
}

Napi::Value GetSdiLevel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Device index required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = GetDeckLinkByIndex(env, deviceIndex);
    if (!deckLink) return env.Null();

    IDeckLinkConfiguration* config = nullptr;
    if (deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&config) == S_OK) {
        bool isLevelA = false;
        if (config->GetFlag(bmdDeckLinkConfigSMPTELevelAOutput, &isLevelA) == S_OK) {
            config->Release();
            deckLink->Release();
            return Napi::String::New(env, isLevelA ? "A" : "B");
        }
        config->Release();
    }

    deckLink->Release();
    Napi::TypeError::New(env, "Unable to get SDI level configuration").ThrowAsJavaScriptException();
    return env.Null();
}

Napi::Value SetSdiLevel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Device index and level string ('A' or 'B') required").ThrowAsJavaScriptException();
        return env.Null();
    }
    int deviceIndex = info[0].As<Napi::Number>().Int32Value();
    std::string levelStr = info[1].As<Napi::String>().Utf8Value();
    if (deviceIndex < 0) {
        Napi::TypeError::New(env, "Device index must be >= 0").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool levelA;
    if (levelStr == "A" || levelStr == "a") {
        levelA = true;
    } else if (levelStr == "B" || levelStr == "b") {
        levelA = false;
    } else {
        Napi::TypeError::New(env, "Invalid level. Use 'A' or 'B'").ThrowAsJavaScriptException();
        return env.Null();
    }

    IDeckLink* deckLink = GetDeckLinkByIndex(env, deviceIndex);
    if (!deckLink) return env.Null();

    // Check if device supports Level A output
    IDeckLinkProfileAttributes* attrs = nullptr;
    if (deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&attrs) == S_OK) {
        bool supportsLevelA = false;
        attrs->GetFlag(BMDDeckLinkSupportsSMPTELevelAOutput, &supportsLevelA);
        attrs->Release();
        if (!supportsLevelA) {
            deckLink->Release();
            Napi::TypeError::New(env, "Device does not support SMPTE Level A output switching").ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    IDeckLinkConfiguration* config = nullptr;
    if (deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&config) != S_OK) {
        deckLink->Release();
        Napi::TypeError::New(env, "Unable to get device configuration").ThrowAsJavaScriptException();
        return env.Null();
    }

    HRESULT result = config->SetFlag(bmdDeckLinkConfigSMPTELevelAOutput, levelA);
    if (result != S_OK) {
        config->Release();
        deckLink->Release();
        Napi::TypeError::New(env, "Failed to set SDI level").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Persist to device preferences
    config->WriteConfigurationToPreferences();
    config->Release();
    deckLink->Release();

    return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "getDeviceTemperature"), Napi::Function::New(env, GetDeviceTemperature));
    exports.Set(Napi::String::New(env, "getAvailableDevices"), Napi::Function::New(env, GetAvailableDevices));
    exports.Set(Napi::String::New(env, "getDeviceOutputs"), Napi::Function::New(env, GetDeviceOutputs));
    exports.Set(Napi::String::New(env, "getDuplexMode"), Napi::Function::New(env, GetDuplexMode));
    exports.Set(Napi::String::New(env, "setDuplexMode"), Napi::Function::New(env, SetDuplexMode));
    exports.Set(Napi::String::New(env, "getSdiLevel"), Napi::Function::New(env, GetSdiLevel));
    exports.Set(Napi::String::New(env, "setSdiLevel"), Napi::Function::New(env, SetSdiLevel));
    return exports;
}

NODE_API_MODULE(decklink_temp, Init)
