// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/MessagesV16/MeterValues.h>
#include <ArduinoOcpp/Core/OcppModel.h>
#include <ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h>
#include <ArduinoOcpp/Debug.h>

using ArduinoOcpp::Ocpp16::MeterValues;

//can only be used for echo server debugging
MeterValues::MeterValues() {
    
}

MeterValues::MeterValues(const std::vector<OcppTimestamp> *sampleTime, const std::vector<float> *energy, const std::vector<float> *power, int connectorId, int transactionId) 
      : connectorId{connectorId}, transactionId{transactionId} {
    if (sampleTime)
        this->sampleTime = std::vector<OcppTimestamp>(*sampleTime);
    if (energy)
        this->energy = std::vector<float>(*energy);
    if (power)
        this->power = std::vector<float>(*power);
}

MeterValues::~MeterValues(){

}

const char* MeterValues::getOcppOperationType(){
    return "MeterValues";
}

std::unique_ptr<DynamicJsonDocument> MeterValues::createReq() {

    int numEntries = sampleTime.size();

    const size_t VALUE_MAXPRECISION = 10;
    const size_t VALUE_MAXSIZE = VALUE_MAXPRECISION + 7; 
    char value_str [VALUE_MAXSIZE] = {'\0'};

    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(
        JSON_OBJECT_SIZE(3) //connectorID, transactionId, meterValue entry
        + JSON_ARRAY_SIZE(numEntries) //metervalue array
        + numEntries * JSON_OBJECT_SIZE(1) //sampledValue entry
        + numEntries * (JSON_OBJECT_SIZE(1) + (JSONDATE_LENGTH + 1)) //timestamp
        + numEntries * JSON_ARRAY_SIZE(2) //sampledValue
        + 2 * numEntries * (JSON_OBJECT_SIZE(1) + VALUE_MAXSIZE) //value
        + 2 * numEntries * JSON_OBJECT_SIZE(1) //measurand
        + 2 * numEntries * JSON_OBJECT_SIZE(1) //unit
        + 230)); //"safety space"
    JsonObject payload = doc->to<JsonObject>();
    
    payload["connectorId"] = connectorId;
    JsonArray meterValues = payload.createNestedArray("meterValue");
    for (size_t i = 0; i < sampleTime.size(); i++) {
        JsonObject meterValue = meterValues.createNestedObject();
        char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
        OcppTimestamp otimestamp = sampleTime.at(i);
        otimestamp.toJsonString(timestamp, JSONDATE_LENGTH + 1);
        meterValue["timestamp"] = timestamp;
        JsonArray sampledValue = meterValue.createNestedArray("sampledValue");
        if (energy.size() >= i + 1) {
            JsonObject sampledValue_1 = sampledValue.createNestedObject();
            snprintf(value_str, VALUE_MAXSIZE, "%.*g", VALUE_MAXPRECISION, energy.at(i));
            sampledValue_1["value"] = value_str;
            sampledValue_1["measurand"] = "Energy.Active.Import.Register";
            sampledValue_1["unit"] = "Wh";
        }
        if (power.size() >= i + 1) {
            JsonObject sampledValue_2 = sampledValue.createNestedObject();
            snprintf(value_str, VALUE_MAXSIZE, "%.*g", VALUE_MAXPRECISION, power.at(i));
            sampledValue_2["value"] = value_str;
            sampledValue_2["measurand"] = "Power.Active.Import";
            sampledValue_2["unit"] = "W";
        }
    }

    if (ocppModel && ocppModel->getConnectorStatus(connectorId)) {
        auto connector = ocppModel->getConnectorStatus(connectorId);
        if (connector->getTransactionIdSync() >= 0) {
            payload["transactionId"] = connector->getTransactionIdSync();
        }
    }

    return doc;
}

void MeterValues::processConf(JsonObject payload) {
    AO_DBG_DEBUG("Request has been confirmed");
}


void MeterValues::processReq(JsonObject payload) {

    /**
     * Ignore Contents of this Req-message, because this is for debug purposes only
     */

}

std::unique_ptr<DynamicJsonDocument> MeterValues::createConf(){
    return createEmptyDocument();
}
