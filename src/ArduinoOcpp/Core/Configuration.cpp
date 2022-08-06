// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/Debug.h>

#include <string.h>
#include <vector>
#include <ArduinoJson.h>

#if defined(ESP32) && !defined(AO_DEACTIVATE_FLASH)
#include <LITTLEFS.h>
#define USE_FS LITTLEFS
#else
#include <FS.h>
#define USE_FS SPIFFS
#endif

namespace ArduinoOcpp {

FilesystemOpt configurationFilesystemOpt = FilesystemOpt::Use_Mount_FormatOnFail;

template<class T>
std::shared_ptr<Configuration<T>> createConfiguration(const char *key, T value) {

    std::shared_ptr<Configuration<T>> configuration = std::make_shared<Configuration<T>>();
        
    if (!configuration->setKey(key)) {
        AO_DBG_ERR("Cannot set key! Abort");
        return nullptr;
    }

    *configuration = value;

    return configuration;
}

std::shared_ptr<Configuration<const char *>> createConfiguration(const char *key, const char *value) {

    std::shared_ptr<Configuration<const char*>> configuration = std::make_shared<Configuration<const char*>>();
        
    if (!configuration->setKey(key)) {
        AO_DBG_ERR("Cannot set key! Abort");
        return nullptr;
    }

    if (!configuration->setValue(value, strlen(value) + 1)) {
        AO_DBG_ERR("Cannot set value! Abort");
        return nullptr;
    }

    return configuration;
}

std::shared_ptr<ConfigurationContainer> createConfigurationContainer(const char *filename) {
    //create non-persistent Configuration store (i.e. lives only in RAM) if
    //     - Flash FS usage is switched off OR
    //     - Filename starts with "/volatile"
    if (!configurationFilesystemOpt.accessAllowed() ||
                 !strncmp(filename, CONFIGURATION_VOLATILE, strlen(CONFIGURATION_VOLATILE))) {
        return std::static_pointer_cast<ConfigurationContainer>(std::make_shared<ConfigurationContainerVolatile>(filename));
    } else {
        //create persistent Configuration store. This is the normal case
        return std::static_pointer_cast<ConfigurationContainer>(std::make_shared<ConfigurationContainerFlash>(filename));
    }
}

std::vector<std::shared_ptr<ConfigurationContainer>> configurationContainers;

void addConfigurationContainer(std::shared_ptr<ConfigurationContainer> container) {
    configurationContainers.push_back(container);
}

std::vector<std::shared_ptr<ConfigurationContainer>>::iterator getConfigurationContainersBegin() {
    return configurationContainers.begin();
}

std::vector<std::shared_ptr<ConfigurationContainer>>::iterator getConfigurationContainersEnd() {
    return configurationContainers.end();
}

std::shared_ptr<ConfigurationContainer> getContainer(const char *filename) {
    std::vector<std::shared_ptr<ConfigurationContainer>>::iterator container = std::find_if(configurationContainers.begin(), configurationContainers.end(),
        [filename](std::shared_ptr<ConfigurationContainer> &elem) {
            return !strcmp(elem->getFilename(), filename);
        });

    if (container != configurationContainers.end()) {
        return *container;
    } else {
        return nullptr;
    }
}

template<class T>
std::shared_ptr<Configuration<T>> declareConfiguration(const char *key, T defaultValue, const char *filename, bool remotePeerCanWrite, bool remotePeerCanRead, bool localClientCanWrite, bool rebootRequiredWhenChanged) {
    //already existent? --> stored in last session --> do set default content, but set writepermission flag
    
    std::shared_ptr<ConfigurationContainer> container = getContainer(filename);
    
    if (!container) {
        AO_DBG_INFO("init new configurations container: %s", filename);

        container = createConfigurationContainer(filename);
        configurationContainers.push_back(container);

        if (!container->load()) {
            AO_DBG_WARN("Cannot load file contents. Path will be overwritten");
        }
    }

    std::shared_ptr<AbstractConfiguration> configuration = container->getConfiguration(key);

    if (configuration && strcmp(configuration->getSerializedType(), SerializedType<T>::get())) {
        AO_DBG_ERR("conflicting declared types. Override previous declaration");
        container->removeConfiguration(configuration);
        configuration->setToBeRemoved();
        configuration = nullptr;
    }

    std::shared_ptr<Configuration<T>> configurationConcrete = std::static_pointer_cast<Configuration<T>>(configuration);

    if (configurationConcrete && configurationConcrete->toBeRemoved()) {
        (*configurationConcrete) = defaultValue;
    }

    if (!configurationConcrete) {
        configurationConcrete = createConfiguration<T>(key, defaultValue);
        configuration = std::static_pointer_cast<AbstractConfiguration>(configurationConcrete);

        if (!configuration) {
            AO_DBG_ERR("Cannot find configuration stored from previous session and cannot create new one! Abort");
            return nullptr;
        }
        container->addConfiguration(configuration);
    }

    if (!remotePeerCanWrite)
        configuration->revokePermissionRemotePeerCanWrite();
    if (!remotePeerCanRead)
        configuration->revokePermissionRemotePeerCanRead();
    if (!localClientCanWrite)
        configuration->revokePermissionLocalClientCanWrite();
    if (rebootRequiredWhenChanged)
        configuration->requireRebootWhenChanged();

    return configurationConcrete;
}

namespace Ocpp16 {

std::shared_ptr<AbstractConfiguration> getConfiguration(const char *key) {
    std::shared_ptr<AbstractConfiguration> result = nullptr;

    for (auto container = configurationContainers.begin(); container != configurationContainers.end(); container++) {
        result = (*container)->getConfiguration(key);
        if (result)
            return result;
    }
    return nullptr;
}

std::shared_ptr<std::vector<std::shared_ptr<AbstractConfiguration>>> getAllConfigurations() { //TODO maybe change to iterator?
    auto result = std::make_shared<std::vector<std::shared_ptr<AbstractConfiguration>>>();

    for (auto container = configurationContainers.begin(); container != configurationContainers.end(); container++) {
        for (auto config = (*container)->configurationsIteratorBegin(); config != (*container)->configurationsIteratorEnd(); config++) {
            if ((*config)->permissionRemotePeerCanRead()) {
                result->push_back(*config);
            }
        }
    }

    return result;
}

} //end namespace Ocpp16

bool configuration_inited = false;

bool configuration_init(FilesystemOpt fsOpt) {
    if (configuration_inited)
        return true; //configuration_init() already called; tolerate multiple calls so user can use this store for
                     //credentials outside ArduinoOcpp which need to be loaded before OCPP_initialize()
    bool loadRoutineSuccessful = true;
#ifndef AO_DEACTIVATE_FLASH

    configurationFilesystemOpt = fsOpt;

    if (fsOpt.mustMount()) { 
#if defined(ESP32)
        if(!LITTLEFS.begin(fsOpt.formatOnFail())) {
            AO_DBG_ERR("Error while mounting LITTLEFS");
            loadRoutineSuccessful = false;
        }
#else
        //ESP8266
        SPIFFSConfig cfg;
        cfg.setAutoFormat(fsOpt.formatOnFail());
        SPIFFS.setConfig(cfg);

        if (!SPIFFS.begin()) {
            AO_DBG_ERR("Unable to initialize: unable to mount SPIFFS");
            loadRoutineSuccessful = false;
        }
#endif
    } //end fs mount

    std::shared_ptr<ConfigurationContainer> containerDefault = nullptr;
    for (auto container = configurationContainers.begin(); container != configurationContainers.end(); container++) {
        if (!strcmp((*container)->getFilename(), CONFIGURATION_FN)) {
            containerDefault = (*container);
            break;
        }
    }

    if (containerDefault) {
        AO_DBG_DEBUG("Found default container before calling configuration_init(). If you added\n" \
                           "        the container manually, please ensure to call load(). If not, it is a hint\n" \
                           "        that declareConfiguration() was called too early\n");
    } else {
        containerDefault = createConfigurationContainer(CONFIGURATION_FN);
        if (!containerDefault->load()) {
            AO_DBG_ERR("Loading default configurations file failed");
            loadRoutineSuccessful = false;
        }
        configurationContainers.push_back(containerDefault);
    }


#endif //ndef AO_DEACTIVATE_FLASH
    configuration_inited = loadRoutineSuccessful;
    return loadRoutineSuccessful;
}

bool configuration_save() {
    bool success = true;
#ifndef AO_DEACTIVATE_FLASH


    for (auto container = configurationContainers.begin(); container != configurationContainers.end(); container++) {
        if (!(*container)->save()) {
            success = false;
        }
    }

#endif //ndef AO_DEACTIVATE_FLASH
    return success;
}

template std::shared_ptr<Configuration<int>> createConfiguration(const char *key, int value);
template std::shared_ptr<Configuration<float>> createConfiguration(const char *key, float value);
template std::shared_ptr<Configuration<const char *>> createConfiguration(const char *key, const char * value);

template std::shared_ptr<Configuration<int>> declareConfiguration(const char *key, int defaultValue, const char *filename, bool remotePeerCanWrite, bool remotePeerCanRead, bool localClientCanWrite, bool rebootRequiredWhenChanged);
template std::shared_ptr<Configuration<float>> declareConfiguration(const char *key, float defaultValue, const char *filename, bool remotePeerCanWrite, bool remotePeerCanRead, bool localClientCanWrite, bool rebootRequiredWhenChanged);
template std::shared_ptr<Configuration<const char *>> declareConfiguration(const char *key, const char *defaultValue, const char *filename, bool remotePeerCanWrite, bool remotePeerCanRead, bool localClientCanWrite, bool rebootRequiredWhenChanged);

} //end namespace ArduinoOcpp
