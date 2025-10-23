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

    IDeckLinkAttributes* deckLinkAttributes = nullptr;
    int64_t outputConnections = 0;

    if (deckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes) == S_OK) {
        if (deckLinkAttributes->GetInt(BMDDeckLinkVideoOutputConnections, &outputConnections) == S_OK) {
            deckLinkAttributes->Release();
            deckLink->Release();

            // Count the number of output connections by checking bits
            int outputCount = 0;
            for (int i = 0; i < 32; i++) {
                if (outputConnections & (1LL << i)) {
                    outputCount++;
                }
            }

            return Napi::Number::New(env, outputCount);
        }
        deckLinkAttributes->Release();
    }

    deckLink->Release();
    Napi::TypeError::New(env, "Unable to get device output count").ThrowAsJavaScriptException();
    return env.Null();
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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "getDeviceTemperature"), Napi::Function::New(env, GetDeviceTemperature));
    exports.Set(Napi::String::New(env, "getAvailableDevices"), Napi::Function::New(env, GetAvailableDevices));
    exports.Set(Napi::String::New(env, "getDeviceOutputs"), Napi::Function::New(env, GetDeviceOutputs));
    return exports;
}

NODE_API_MODULE(decklink_temp, Init)
