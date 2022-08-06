// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef DIAGNOSTICS_STATUS
#define DIAGNOSTICS_STATUS

namespace ArduinoOcpp {
namespace Ocpp16 {

enum class DiagnosticsStatus {
    Idle,
    Uploaded,
    UploadFailed,
    Uploading
};

}
} //end namespace ArduinoOcpp
#endif
