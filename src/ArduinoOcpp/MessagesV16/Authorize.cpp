// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/MessagesV16/Authorize.h>
#include <ArduinoOcpp/Core/OcppModel.h>
#include <ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h>

#include <ArduinoOcpp/Debug.h>

using ArduinoOcpp::Ocpp16::Authorize;

//Authorize::Authorize() {
//    snprintf(this->idTag, IDTAG_LEN_MAX + 1, "A0-00-00-00"); //Use a default payload. In the typical use case of this library, you probably you don't even need Authorization at all
//}

Authorize::Authorize(const char *idTagIn) {
    if (idTagIn && strnlen(idTagIn, IDTAG_LEN_MAX + 2) <= IDTAG_LEN_MAX) {
        snprintf(idTag, IDTAG_LEN_MAX + 1, "%s", idTagIn);
    } else {
        AO_DBG_WARN("Format violation of idTag. Use default idTag");
        snprintf(idTag, IDTAG_LEN_MAX + 1, "A0-00-00-00");
    }
}

const char* Authorize::getOcppOperationType(){
    return "Authorize";
}

std::unique_ptr<DynamicJsonDocument> Authorize::createReq() {
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1) + (IDTAG_LEN_MAX + 1)));
    JsonObject payload = doc->to<JsonObject>();
    payload["idTag"] = idTag;
    return doc;
}

void Authorize::processConf(JsonObject payload){
    const char *idTagInfo = payload["idTagInfo"]["status"] | "not specified";

    if (!strcmp(idTagInfo, "Accepted")) {
        AO_DBG_INFO("Request has been accepted");

        //TODO add entry in offline auth cache
    
    } else {
        AO_DBG_INFO("Request has been denied. Reason: %s", idTagInfo);
    }
}

void Authorize::processReq(JsonObject payload){
    /*
     * Ignore Contents of this Req-message, because this is for debug purposes only
     */
}

std::unique_ptr<DynamicJsonDocument> Authorize::createConf(){
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1)));
    JsonObject payload = doc->to<JsonObject>();
    JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
    idTagInfo["status"] = "Accepted";
    return doc;
}
