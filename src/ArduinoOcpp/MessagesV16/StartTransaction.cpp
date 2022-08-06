// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/MessagesV16/StartTransaction.h>
#include <ArduinoOcpp/Core/OcppModel.h>
#include <ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h>
#include <ArduinoOcpp/Tasks/Metering/MeteringService.h>
#include <ArduinoOcpp/Debug.h>

using ArduinoOcpp::Ocpp16::StartTransaction;

StartTransaction::StartTransaction(int connectorId) : connectorId(connectorId) {
    
}

StartTransaction::StartTransaction(int connectorId, const char *idTag) : connectorId(connectorId) {
    if (idTag && strnlen(idTag, IDTAG_LEN_MAX + 2) <= IDTAG_LEN_MAX)
        snprintf(this->idTag, IDTAG_LEN_MAX + 1, "%s", idTag);
    else
        AO_DBG_ERR("Format violation");
}

const char* StartTransaction::getOcppOperationType(){
    return "StartTransaction";
}

void StartTransaction::initiate() {
    if (ocppModel && ocppModel->getMeteringService()) {
        auto meteringService = ocppModel->getMeteringService();
        meterStart = (int) meteringService->readEnergyActiveImportRegister(connectorId);
    }

    if (ocppModel) {
        otimestamp = ocppModel->getOcppTime().getOcppTimestampNow();
    } else {
        otimestamp = MIN_TIME;
    }

    if (ocppModel && ocppModel->getConnectorStatus(connectorId)) {
        auto connector = ocppModel->getConnectorStatus(connectorId);

        if (*idTag == '\0') {
            const char *sessionIdTag = connector->getSessionIdTag();
            if (sessionIdTag) {
                snprintf(idTag, IDTAG_LEN_MAX + 1, "%s", sessionIdTag);
            } else {
                AO_DBG_WARN("Try to start transaction without providing idTag. Initialize session with default idTag");
                connector->beginSession(nullptr);
                sessionIdTag = connector->getSessionIdTag(); //returns default idTag now
                if (sessionIdTag)
                    snprintf(idTag, IDTAG_LEN_MAX + 1, "%s", sessionIdTag);
            }
        } else {
            //idTag has been overriden
            connector->beginSession(idTag);
        }

        if (connector->getTransactionId() >= 0) {
            AO_DBG_WARN("Started transaction while OCPP already presumes a running transaction");
        }
        connector->setTransactionId(0); //pending
        transactionRev = connector->getTransactionWriteCount();
    }

    AO_DBG_INFO("StartTransaction initiated");
}

std::unique_ptr<DynamicJsonDocument> StartTransaction::createReq() {
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(5) + (JSONDATE_LENGTH + 1) + (IDTAG_LEN_MAX + 1)));
    JsonObject payload = doc->to<JsonObject>();

    payload["connectorId"] = connectorId;
    if (meterStart >= 0) {
        payload["meterStart"] = meterStart;
    }

    if (otimestamp > MIN_TIME) {
        char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
        otimestamp.toJsonString(timestamp, JSONDATE_LENGTH + 1);
        payload["timestamp"] = timestamp;
    }

    payload["idTag"] = idTag;

    return doc;
}

void StartTransaction::processConf(JsonObject payload) {

    const char* idTagInfoStatus = payload["idTagInfo"]["status"] | "not specified";
    int transactionId = payload["transactionId"] | -1;

    ConnectorStatus *connector = nullptr;
    if (ocppModel)
        connector = ocppModel->getConnectorStatus(connectorId);
    
    if (connector){
        if (transactionRev == connector->getTransactionWriteCount()) {
            
            if (!strcmp(idTagInfoStatus, "Accepted")) {
                AO_DBG_INFO("Request has been accepted");
                connector->setTransactionId(transactionId);
            } else {
                AO_DBG_INFO("Request has been denied. Reason: %s", idTagInfoStatus);
                //connector->setTransactionId(-1);
                connector->endSession(); //something is wrong with the idTag. Abort session
            }
        }
        connector->setTransactionIdSync(transactionId);

        AO_DBG_DEBUG("Local txId = %i, remote txId = %i", connector->getTransactionId(), connector->getTransactionIdSync());
    }

}


void StartTransaction::processReq(JsonObject payload) {

  /**
   * Ignore Contents of this Req-message, because this is for debug purposes only
   */

}

std::unique_ptr<DynamicJsonDocument> StartTransaction::createConf(){
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2)));
    JsonObject payload = doc->to<JsonObject>();

    JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
    idTagInfo["status"] = "Accepted";
    payload["transactionId"] = 123456; //sample data for debug purpose

    return doc;
}
