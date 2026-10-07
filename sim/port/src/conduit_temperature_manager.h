// Heat exchange between pipe contents and the pipe structures (ConduitTemperatureManager_*).
#pragma once

struct BuildingTemperatureInfo;

class ConduitTemperatureManager {
public:
    // Energy flowing between the contents and the conduit in one step.
    // insulated: use the smaller of the two conductivities instead of their average.
    float CalculateEnergyFlow(bool insulated, float contents_temperature, float contents_conductivity,
                              float conduit_temperature, float conduit_conductivity, float scale);
};
