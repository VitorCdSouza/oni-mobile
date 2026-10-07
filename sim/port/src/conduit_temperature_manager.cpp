#include "conduit_temperature_manager.h"

float ConduitTemperatureManager::CalculateEnergyFlow(bool insulated, float contents_temperature,
                                                     float contents_conductivity, float conduit_temperature,
                                                     float conduit_conductivity, float scale) {
    float flow = (contents_temperature - conduit_temperature) * scale;
    if (insulated) {
        // minss semantics: the second operand wins unless the first is strictly smaller.
        float conductivity = contents_conductivity < conduit_conductivity ? contents_conductivity
                                                                          : conduit_conductivity;
        return flow * conductivity;
    }
    return flow * (contents_conductivity + conduit_conductivity) * 0.5f;
}
