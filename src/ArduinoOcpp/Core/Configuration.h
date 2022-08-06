// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ArduinoOcpp/Core/ConfigurationKeyValue.h>
#include <ArduinoOcpp/Core/ConfigurationOptions.h>
#include <ArduinoOcpp/Core/ConfigurationContainerFlash.h>

#include <memory>
#include <vector>

#define CONFIGURATION_FN "/arduino-ocpp.cnf"
#define CONFIGURATION_VOLATILE "/volatile"

namespace ArduinoOcpp {

template <class T>
std::shared_ptr<Configuration<T>> declareConfiguration(const char *key, T defaultValue, const char *filename = CONFIGURATION_FN, bool remotePeerCanWrite = true, bool remotePeerCanRead = true, bool localClientCanWrite = true, bool rebootRequiredWhenChanged = false);

void addConfigurationContainer(std::shared_ptr<ConfigurationContainer> container);
std::vector<std::shared_ptr<ConfigurationContainer>>::iterator getConfigurationContainersBegin();
std::vector<std::shared_ptr<ConfigurationContainer>>::iterator getConfigurationContainersEnd();


namespace Ocpp16 {

    std::shared_ptr<AbstractConfiguration> getConfiguration(const char *key);
    std::shared_ptr<std::vector<std::shared_ptr<AbstractConfiguration>>> getAllConfigurations();
}

bool configuration_init(FilesystemOpt fsOpt = FilesystemOpt::Use_Mount_FormatOnFail);
bool configuration_save();

} //end namespace ArduinoOcpp
#endif
