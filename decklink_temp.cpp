#include <napi.h>
#include "DeckLinkAPI.h"

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
    return exports;
}

NODE_API_MODULE(decklink_temp, Init)
