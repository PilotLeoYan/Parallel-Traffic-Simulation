#include "monitoring/metrics_collector.hpp"
#include <iostream>
#include <iomanip>

namespace city::monitoring {

void MetricsCollector::recordVehicleArrival(int vehicle_id, double total_time, double wait_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Calcular porcentaje del viaje detenido (Tiempo en semáforos/tráfico)
    double wait_percentage = (total_time > 0) ? (wait_time / total_time) * 100.0 : 0.0;
    
    vehicle_metrics_.push_back({vehicle_id, total_time, wait_time, wait_percentage});

    // El primer vehículo que registra su llegada es el "Vehículo que llegó primero"
    if (first_vehicle_id_ == -1) {
        first_vehicle_id_ = vehicle_id;
        first_vehicle_time_ = total_time;
    }
}

void MetricsCollector::updateCongestion(int x, int y, int waiting_vehicles) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string coord_key = "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    
    if (waiting_vehicles > congestion_map_[coord_key]) {
        congestion_map_[coord_key] = waiting_vehicles;
    }
}

void MetricsCollector::exportToTable(const std::string& filename) const {
    (void)filename;
    std::lock_guard<std::mutex> lock(mutex_);

    if (vehicle_metrics_.empty()) {
        std::cout << "No hay métricas registradas (ningún vehículo llegó a su destino).\n";
        return;
    }

    // Calcular el Tiempo Promedio de Viaje (Evaluación general)
    double total_time_sum = 0.0;
    for (const auto& vm : vehicle_metrics_) {
        total_time_sum += vm.total_travel_time;
    }
    double avg_time = total_time_sum / vehicle_metrics_.size();

    std::cout << "\n=================================================================================\n";
    std::cout << "                             MÉTRICAS Y RESULTADOS FINALES                       \n";
    std::cout << "=================================================================================\n";
    
    // Vehículo que llegó primero
    std::cout << "[!] Vehículo que llegó primero: ID " << first_vehicle_id_ 
              << " con un tiempo de " << std::fixed << std::setprecision(2) << first_vehicle_time_ << "s\n";
              
    // Tiempo promedio de viaje
    std::cout << "[!] Tiempo promedio de viaje (Evaluación General): " << avg_time << "s\n\n";

    // Tabla de Vehículos
    std::cout << std::left << std::setw(12) << "Vehiculo" 
              << std::setw(22) << "Tiempo Total (s)" 
              << std::setw(22) << "Tiempo Espera (s)" 
              << std::setw(20) << "% Viaje Detenido" << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    for (const auto& vm : vehicle_metrics_) {
        std::cout << std::left << std::setw(12) << vm.vehicle_id 
                  << std::setw(22) << std::fixed << std::setprecision(2) << vm.total_travel_time 
                  << std::setw(22) << vm.wait_time 
                  << vm.wait_percentage << "%\n";
    }

    std::cout << "\n---------------------------------------------------------------------------------\n";
    std::cout << "                             REPORTE DE CONGESTIÓN                               \n";
    std::cout << "---------------------------------------------------------------------------------\n";
    
    bool has_congestion = false;
    for (const auto& [intersection, count] : congestion_map_) {
        if (count > 0) {
            std::cout << "-> Intersección " << intersection << ": " << count << " vehículos en espera.\n";
            has_congestion = true;
        }
    }
    if (!has_congestion) {
        std::cout << "No se detectó congestión en las intersecciones (tráfico fluido).\n";
    }
    std::cout << "=================================================================================\n\n";
}

} // namespace traffic_simulation