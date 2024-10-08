#include <omp.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

/* struct Point
{
    double x, y;
    int cluster; // -1: unvisited, 0: noise, >0: cluster ID
};

double distance(const Point &p1, const Point &p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}
 */

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
}

void expandCluster(float **points, long long int size, long long int point_idx, int clusterID, float epsilon, int min_samples)
{
    vector<long long int> neighbors;
    regionQuery(points, size, point_idx, epsilon, neighbors);

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
}

void noise_detection(float **points, float epsilon, int min_samples, long long int size) //! Esto es lo que se debe de modificar
{
    cout << "Step 0" << "\n";
    for (long long int i = 0; i < size; i++)
    {
        points[i][2] = rand() % 2;
    }
    cout << "Complete" << "\n";
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
        point_number++;
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

    const float epsilon = 0.03;
    const int min_samples = 10;
    const long long int size = 4000;
    const string input_file_name = to_string(size) + "_data.csv";
    const string output_file_name = to_string(size) + "_results.csv";
    float **points = new float *[size];

    for (long long int i = 0; i < size; i++)
    {
        points[i] = new float[3]{0.0, 0.0, 0.0};
        // index 0: position x
        // index 1: position y
        // index 2: 0 for noise point, 1 for core point
    }

    load_CSV(input_file_name, points, size);

    noise_detection(points, epsilon, min_samples, size);

    save_to_CSV(output_file_name, points, size);

    for (long long int i = 0; i < size; i++)
    {
        delete[] points[i];
    }
    delete[] points;
    return 0;
}