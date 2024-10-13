#include <omp.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

double calculate_distance(float *point1, float *point2)
{
    return sqrt(pow(point1[0] - point2[0], 2) + pow(point1[1] - point2[1], 2));
}

void regionQuery(float **points, long long int size, long long int point_idx, float epsilon, vector<long long int> &neighbors)
{
    for (long long int i = 0; i < size; i++)
    {
        if (i != point_idx)
        {
            float dist = calculate_distance(points[point_idx], points[i]);
            if (dist <= epsilon)
            {
                neighbors.push_back(i);
            }
        }
    }
    // Depuración: Mostrar cuántos vecinos se encontraron para cada punto
    // cout << "Punto " << point_idx << " encontró " << neighbors.size() << " vecinos dentro de epsilon.\n";
}

void expandCluster(float **points, long long int size, long long int point_idx, int clusterID, float epsilon, int min_samples)
{
    vector<long long int> neighbors;
    regionQuery(points, size, point_idx, epsilon, neighbors);

    // Depuración: Mostrar cuántos vecinos se encontraron antes de decidir si es ruido
    // cout << "Punto " << point_idx << " tiene " << neighbors.size() << " vecinos.\n";

    if (neighbors.size() < min_samples)
    {
        points[point_idx][2] = 0; // Mark as noise
        return;
    }

    points[point_idx][2] = clusterID;

    for (size_t i = 0; i < neighbors.size(); i++)
    {
        long long int neighbor_idx = neighbors[i];
        if (points[neighbor_idx][2] == -1 || points[neighbor_idx][2] == 0)
        { // Unvisited or noise
            points[neighbor_idx][2] = clusterID;
            vector<long long int> neighbor_neighbors;
            regionQuery(points, size, neighbor_idx, epsilon, neighbor_neighbors);
            if (neighbor_neighbors.size() >= min_samples)
            {
                neighbors.insert(neighbors.end(), neighbor_neighbors.begin(), neighbor_neighbors.end());
            }
        }
    }
}

void identify_and_count_noise_points(float **points, long long int size)
{
    int noise_count = 0; // Contador de puntos de ruido

    for (long long int i = 0; i < size; i++)
    {
        // Revisar si el punto fue etiquetado como ruido (etiqueta 0)
        if (points[i][2] == 0)
        {
            noise_count++;
            // Imprimir coordenadas de los puntos de ruido (opcional)
            // cout << "Punto de ruido: (" << points[i][0] << ", " << points[i][1] << ")\n";
        }
    }

    // Imprimir el número total de puntos de ruido
    cout << "Número total de puntos de ruido: " << noise_count << endl;
}

//* La epsilon la define el usuario
void dbscan(float **points, long long int size, float epsilon, int min_samples)
{
    int clusterID = 1;
    for (long long int i = 0; i < size; i++)
    {
        if (points[i][2] == -1)
        { // Unvisited
            expandCluster(points, size, i, clusterID, epsilon, min_samples);
            clusterID++;
        }
    }
    identify_and_count_noise_points(points, size);
}

void load_CSV(string file_name, float **points, long long int size)
{
    ifstream in(file_name);
    if (!in)
    {
        cerr << "Couldn't read file: " << file_name << "\n";
    }
    long long int point_number = 0;
    while (!in.eof() && (point_number < size))
    {
        char *line = new char[12];
        streamsize row_size = 12;
        in.read(line, row_size);
        string row = line;
        // cout << stof(row.substr(0, 5)) << " - " << stof(row.substr(6, 5)) << "\n";
        points[point_number][0] = stof(row.substr(0, 5));
        points[point_number][1] = stof(row.substr(6, 5));
        points[point_number][2] = -1; // Inicializar como no visitado
        point_number++;
        delete[] line;
    }
}

void save_to_CSV(string file_name, float **points, long long int size)
{
    fstream fout;
    fout.open(file_name, ios::out);
    for (long long int i = 0; i < size; i++)
    {
        fout << points[i][0] << ","
             << points[i][1] << ","
             << points[i][2] << "\n";
    }
}

int main(int argc, char **argv)
{
    const float epsilon = 0.03; // Incrementar epsilon para una vecindad aún más grande
    const int min_samples = 10; // Reducir min_samples para facilitar la formación de clusters
    const long long int size = 2000;
    const string input_file_name = to_string(size) + "_data.csv";
    const string output_file_name = to_string(size) + "_results_serial.csv";
    float **points = new float *[size];

    // TODO: Preguntar octavio para que sirve esto
    // Primera prueba
    for (long long int i = 0; i < size; i++)
    {
        points[i] = new float[3]{0.0, 0.0, -1}; // Inicializar como no visitado
        // index 0: position x
        // index 1: position y
        // index 2: -1 para no visitado, 0 para ruido, clusterID para puntos de cluster
    }

    load_CSV(input_file_name, points, size);
    int tiempo_ejecucion = 0;
    auto start = high_resolution_clock::now();
    dbscan(points, size, epsilon, min_samples);
    auto end = high_resolution_clock::now();
    save_to_CSV(output_file_name, points, size);
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Tiempo de ejecución del algoritmo DBSCAN: " << duration.count() << " microsegundos" << endl;
    for (long long int i = 0; i < size; i++)
    {
        delete[] points[i];
    }
    delete[] points;
    return 0;
}
