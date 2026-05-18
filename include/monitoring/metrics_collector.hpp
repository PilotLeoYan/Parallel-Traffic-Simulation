#pragma once
#include <vector>
#include <map>
#include <mutex>
#include <string>

namespace city::monitoring {
    
struct VehicleMetric {
    int vehicle_id;
    double total_travel_time;  // Desde partida hasta llegada
    double wait_time;          // Tiempo detenido
    double wait_percentage;    // Porcentaje del viaje detenido
};

class MetricsCollector {
public:
    // Registra los datos de un vehículo cuando llega a su destino
    void recordVehicleArrival(int vehicle_id, double total_time, double wait_time);
    
    // Actualiza el número de vehículos esperando por intersección (Congestión)
    void updateCongestion(int x, int y, int waiting_vehicles);
    
    // Muestra la tabla en consola tal como lo pide el PDF
    void exportToTable(const std::string& filename = "") const;

private:
    mutable std::mutex mutex_;
    std::vector<VehicleMetric> vehicle_metrics_;
    std::map<std::string, int> congestion_map_;
    
    // Para determinar el orden de llegada
    int first_vehicle_id_ = -1;
    double first_vehicle_time_ = -1.0;
};

} // namespace traffic_simulation