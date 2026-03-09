#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

using namespace std;

const int NUM_CONTROLADORES = 5;

// Clase Monitor para gestionar el estado de las intersecciones
class GestionTrafico {
private:
    enum Estado { LIBRE, ESPERANDO, CAMBIANDO_LUCES };
    Estado estado[NUM_CONTROLADORES];
    mutex mtx;
    condition_variable cv[NUM_CONTROLADORES];

    // Verifica si el controlador i puede adquirir sus dos zonas (tenedores)
    void intentar_adquirir(int i) {
        int izquierda = (i + NUM_CONTROLADORES - 1) % NUM_CONTROLADORES;
        int derecha = (i + 1) % NUM_CONTROLADORES;

        if (estado[i] == ESPERANDO && 
            estado[izquierda] != CAMBIANDO_LUCES && 
            estado[derecha] != CAMBIANDO_LUCES) {
            
            estado[i] = CAMBIANDO_LUCES;
            cv[i].notify_one();
        }
    }

public:
    GestionTrafico() {
        for (int i = 0; i < NUM_CONTROLADORES; ++i) {
            estado[i] = LIBRE;
        }
    }

    void tomar_zonas(int i) {
        unique_lock<mutex> lock(mtx);
        estado[i] = ESPERANDO;
        cout << "[Controlador " << i << "] Esperando zonas de confluencia..." << endl;
        
        intentar_adquirir(i);
        
        while (estado[i] != CAMBIANDO_LUCES) {
            cv[i].wait(lock);
        }
        cout << "[Controlador " << i << "] 🟢 ZONAS ADQUIRIDAS. Sincronizando semáforos..." << endl;
    }

    void liberar_zonas(int i) {
        unique_lock<mutex> lock(mtx);
        estado[i] = LIBRE;
        cout << "[Controlador " << i << "] 🔴 Cambio completado. Zonas liberadas." << endl;

        // Avisar a los vecinos (izquierda y derecha) que pueden intentar de nuevo
        intentar_adquirir((i + NUM_CONTROLADORES - 1) % NUM_CONTROLADORES);
        intentar_adquirir((i + 1) % NUM_CONTROLADORES);
    }
};

// Función que ejecuta cada hilo (Controlador de Tráfico)
void controlador_interseccion(int id, GestionTrafico &siget) {
    for (int ciclo = 0; ciclo < 3; ++ciclo) {
        // Simula análisis de flujo de tráfico
        this_thread::sleep_for(chrono::milliseconds(1000 + (rand() % 1000)));
        
        siget.tomar_zonas(id);
        
        // Simula el tiempo de cambio físico de luces
        this_thread::sleep_for(chrono::milliseconds(2000));
        
        siget.liberar_zonas(id);
    }
}

int main() {
    GestionTrafico siget;
    vector<thread> hilos;

    cout << "--- SIMULACIÓN DE CONCURRENCIA SIGET ---" << endl;
    cout << "Iniciando controladores de zona..." << endl << endl;

    for (int i = 0; i < NUM_CONTROLADORES; ++i) {
        hilos.push_back(thread(controlador_interseccion, i, ref(siget)));
    }

    for (auto &t : hilos) {
        t.join();
    }

    cout << "\nSimulación finalizada con éxito." << endl;
    return 0;
}