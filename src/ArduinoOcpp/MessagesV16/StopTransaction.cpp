// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/MessagesV16/StopTransaction.h>
#include <ArduinoOcpp/Core/OcppModel.h>
#include <ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h>
#include <ArduinoOcpp/Tasks/Metering/MeteringService.h>
#include <ArduinoOcpp/Debug.h>

using ArduinoOcpp::Ocpp16::StopTransaction;

StopTransaction::StopTransaction(int connectorId) : connectorId(connectorId) {

}

const char* StopTransaction::getOcppOperationType(){
    return "StopTransaction";
}

void StopTransaction::initiate() {

    if (ocppModel && ocppModel->getMeteringService()) {
        auto meteringService = ocppModel->getMeteringService();
        meterStop = (int) meteringService->readEnergyActiveImportRegister(connectorId);
    }

    if (ocppModel) {
        otimestamp = ocppModel->getOcppTime().getOcppTimestampNow();
    } else {
        otimestamp = MIN_TIME;
    }

    if (ocppModel && ocppModel->getConnectorStatus(connectorId)){
        auto connector = ocppModel->getConnectorStatus(connectorId);
        connector->setTransactionId(-1); //immediate end of transaction
        if (connector->getSessionIdTag()) {
            AO_DBG_DEBUG("Ending EV user session triggered by StopTransaction");
            connector->endSession();
        }
    }

    AO_DBG_INFO("StopTransaction initiated!");
}

std::unique_ptr<DynamicJsonDocument> StopTransaction::createReq() {
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(4) + (JSONDATE_LENGTH + 1)));
    JsonObject payload = doc->to<JsonObject>();

    if (meterStop >= 0)
        payload["meterStop"] = meterStop; //TODO meterStart is required to be in Wh, but measuring unit is probably inconsistent in implementation

    if (otimestamp > MIN_TIME) {
        char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
        otimestamp.toJsonString(timestamp, JSONDATE_LENGTH + 1);
        payload["timestamp"] = timestamp;
    }
    
    if (ocppModel && ocppModel->getConnectorStatus(connectorId)){
        auto connector = ocppModel->getConnectorStatus(connectorId);
        payload["transactionId"] = connector->getTransactionIdSync();
    }

    return doc;
}

void StopTransaction::processConf(JsonObject payload) {

    if (ocppModel && ocppModel->getConnectorStatus(connectorId)){
        auto connector = ocppModel->getConnectorStatus(connectorId);
        connector->setTransactionIdSync(-1);
    }

    AO_DBG_INFO("Request has been accepted!");
}


void StopTransaction::processReq(JsonObject payload) {
    /**
     * Ignore Contents of this Req-message, because this is for debug purposes only
     */
}

std::unique_ptr<DynamicJsonDocument> StopTransaction::createConf(){
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1)));
    JsonObject payload = doc->to<JsonObject>();

    JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
    idTagInfo["status"] = "Accepted";

    return doc;
}
