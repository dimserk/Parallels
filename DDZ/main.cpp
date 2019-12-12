#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cmath>
#include <limits>
#include <vector>
#include <omp.h>

//#include <stdexcept>

using namespace std;

#define RAND_START 1
#define RAND_STOP 1000
#define PRECISION 0.01

#define PROC_NUM_POSITION 1
#define POINTS_NUM_POSITION 2
#define CLUSTERS_NUM_POSITION 3

class CommandLineArgs {
    public:
        int proc_num; // Количество omp процессов
        int point_num; // Количество точек
        int clust_num; // Количество кластеров
        bool test_flag; // Флаг тестового режима
        string test_filename; // Имя входного файла
    
        CommandLineArgs(){ // Значения по умолчанию
            this->proc_num = -1;
            this->point_num = -1;
            this->clust_num = -1;
            this->test_flag = false;
        }
};

class Point {
    public:
        double x;
        double y;

    Point() {
        x = 0;
        y = 0;
    }
    Point(double _x, double _y) {
        x = _x;
        y = _y;
    }
};

Point* arr_generator(int arr_size) {
    Point *arr = new Point[arr_size];
    for (int i = 0; i < arr_size; i++) {
        arr[i].x = ((RAND_START + rand()) % RAND_STOP) * PRECISION;
        arr[i].y = ((RAND_START + rand()) % RAND_STOP) * PRECISION;
    }
    return arr;
}

bool f_equality(double &val1, double &val2) {
    return fabs(val1 - val2) < PRECISION;
}

int main (int argc, char** argv) {

    // * =====================================
    // * Обработка аргументов командной строки
    // * =====================================

    CommandLineArgs command_line_args;

    int comm_line_position;
    string argm_name;

    try {
        for (comm_line_position = 1; comm_line_position < argc; comm_line_position++) {
            if(strcmp("-t", argv[comm_line_position]) == 0) {
                command_line_args.test_flag = true;
                command_line_args.test_filename = argv[++comm_line_position];

                if (command_line_args.test_filename.find(".csv") == string::npos) {
                    cerr << "#Error# Input file should have CSV extension" << endl;
                    return -1;
                }
                continue;
            }
            else if (comm_line_position == PROC_NUM_POSITION) {
                argm_name = "proc_num";
                command_line_args.proc_num = stoi(argv[comm_line_position]);

                if (command_line_args.proc_num <= 0 || 
                    command_line_args.proc_num > omp_get_max_threads()) {
                    throw invalid_argument("Invalid proces number");
                }
                continue;
            }
            else if (comm_line_position == POINTS_NUM_POSITION) {
                argm_name = "points_num";
                command_line_args.point_num = stoi(argv[comm_line_position]);

                if (command_line_args.point_num <= 0) {
                    throw invalid_argument("Invalid point number");
                }
                continue;
            }
            else if (comm_line_position == CLUSTERS_NUM_POSITION) {
                argm_name = "clusters_num";
                command_line_args.clust_num = stoi(argv[comm_line_position]);

                if (command_line_args.clust_num <= 0) {
                    throw invalid_argument("Invalid cluster number");
                }
                continue;
            }
            else {
                cerr << "#Error# Unknown argument: " << argv[comm_line_position] << endl;
                return -1;
            } 
        }
    }
    catch (invalid_argument) {
        cerr << "#Error# Argument " << argm_name << " is not set correctly" << endl;
        return -1;
    }
    catch (out_of_range) {
        cerr << "#Error# Argument " << argm_name << " has overflowed value" << endl;
        return -1;
    }

    #ifdef DEBUG_P
    cout << "Command line args:" << endl;
    cout << "M points num:" << '\t' << command_line_args.point_num << endl;
    cout << "K cluster num:" << '\t' << command_line_args.clust_num << endl;
    cout << "Test mode:" << '\t' << command_line_args.test_flag << endl;
    cout << "Input file:" << '\t' << command_line_args.test_filename << endl;
    cout << endl;
    #endif

    // Проверка наличия обязательных аргументов
    argm_name.clear();
    if (command_line_args.proc_num == -1) {
        argm_name += "proc_num ";
    }
    if (command_line_args.point_num == -1) {
        argm_name += "points_num ";
    }
    if (command_line_args.clust_num == -1) {
        argm_name += "clusters_num ";
    }

    if (!argm_name.empty()) {
        cerr << "#Error# Argument(s) " << argm_name << "required" << endl;
        return -2;
    }

    // * ================
    // * Генерация данных
    // * ================

    //Генерация центроидов (автоматическая при любом режиме работы)
    Point *clusters_arr = arr_generator(command_line_args.clust_num);

    // Генерация точек
    int point_num = 0;
    Point *points_arr;

    if (command_line_args.test_flag) {
        // Режим тестирования
        ifstream fin(command_line_args.test_filename);
        points_arr = new Point[command_line_args.point_num];

        try {
            if (fin.is_open()) {
                string input_buf;

                while(getline(fin, input_buf)){
                    if (input_buf.empty()) {
                        continue;
                    }

                    string x_coord, y_coord, etc;
                    stringstream point_declar(input_buf);
                    getline(point_declar, x_coord, ',');
                    getline(point_declar, y_coord, ',');
                    getline(point_declar, etc, ',');

                    if (!etc.empty()) {
                        cerr << "#Error# Invalid declaration: " << input_buf << endl;
                        throw exception();
                    }
                        
                    try {
                        points_arr[point_num].x = stod(x_coord);
                        points_arr[point_num].y = stod(y_coord);
                        point_num++;
                    }
                    catch (invalid_argument) {
                        cerr << "#Error# Invalid point declaration: " << input_buf
                            << " (invalid coord)" << endl;
                        throw exception();
                    }
                    catch (out_of_range) {
                        cerr << "#Error# Invalid point declaration: " << input_buf
                            << " (overflowed coord)" << endl;
                        throw exception();
                    }
                }

                if (command_line_args.point_num != point_num) {
                    cerr << "#Error# No match in points decalaration: " << point_num
                        << '/' << command_line_args.point_num << endl;
                    throw exception();
                }

                fin.close();
            }
            else {
                // Обработка случая не открывшегося файла
                string reason;
                if (fin.bad()) {
                    reason = "System error occured while reading file";
                } else if (fin.fail()) {
                    reason = "No such file or has no permission";
                } else {
                    reason = "Problem with i/o hardware";
                }

                cerr << "#Error# Can not open " << command_line_args.test_filename 
                    << " (" << reason << ')' << endl;
                return -3;                
            }
        }
        catch (exception) { 
            // Завершение работы с очисткой памяти в случае непавильных входных данных
            fin.close();

            delete[] points_arr;
            delete[] clusters_arr;
            return -3;
        }
    }
    else {
        // Генерация случайных точек в режиме эксперемента
        points_arr = arr_generator(command_line_args.point_num);
    }

    #ifdef DEBUG_P
    cout << "Points:" << endl;
    for (int i = 0; i < command_line_args.point_num; i++) {
        cout << "X:" << points_arr[i].x << " Y:" << points_arr[i].y << endl;
    }
    cout << "Clusters:" << endl;      
    for (int i = 0; i < command_line_args.clust_num; i++) {
        cout << "X:" << clusters_arr[i].x << " Y:" << clusters_arr[i].y << endl;
    }
    cout << endl;
    #endif

    // * =============
    // * Обсчёт данных
    // * =============

    bool same_clusters_flag;
    int min_clust;
    double min_dist, tmp_dist, n_x, n_y, x_sum, y_sum, c_start, c_stop;
    vector<int> *belong_list = new vector<int>[command_line_args.clust_num];

    omp_lock_t update_lock;
    omp_init_lock(&update_lock);
    
    omp_set_num_threads(command_line_args.proc_num);

    c_start = omp_get_wtime();

    while (true) {
        same_clusters_flag = true;

        // Кластеризация точек
        #pragma omp parallel for private(min_dist, tmp_dist, min_clust) shared(belong_list)
        for (int i = 0; i < command_line_args.point_num; i++) {
            min_dist = numeric_limits<double>::max();

            // Определение кластера точки
            for (int j = 0; j < command_line_args.clust_num; j++) {
                tmp_dist = pow(pow(abs(points_arr[i].x - clusters_arr[j].x), 2) 
                    + pow(abs(points_arr[i].y - clusters_arr[j].y), 2), 0.5);

                if (tmp_dist < min_dist) {
                    min_dist = tmp_dist;
                    min_clust = j;
                }
            }

            // Фиксация кластера точки
            omp_set_lock(&update_lock);
            belong_list[min_clust].push_back(i); 
            omp_unset_lock(&update_lock);
        }

        #ifdef DEBUG_P
        cout << "Clustering result:" << endl;
        for (int i = 0; i < command_line_args.clust_num; i++) {
            cout << "Cluster:" << i+1 << '(' << clusters_arr[i].x << ',' << clusters_arr[i].y
                << ") size: " << belong_list[i].size() << endl;
            for (int j = 0; j < belong_list[i].size(); j++) {
                cout << points_arr[belong_list[i][j]].x << ":" << points_arr[belong_list[i][j]].y << endl;
            }
        }
        cout << endl;
        #endif

        // Перерасчёт координат центроидов
        #pragma omp parallel for private(x_sum, y_sum, n_x, n_y) shared(iter_flag)
        for (int i = 0; i < command_line_args.clust_num; i++) {
            x_sum = y_sum = 0;
            for (int j = 0; j < belong_list[i].size(); j++) {
                x_sum += points_arr[belong_list[i][j]].x;
                y_sum += points_arr[belong_list[i][j]].y;
            }

            if (belong_list[i].size() == 0) {
                continue;
            }

            n_x = (double)x_sum / belong_list[i].size();
            n_y = (double)y_sum / belong_list[i].size();

            // Определение изменений координат центроидов
            omp_set_lock(&update_lock);
            same_clusters_flag = same_clusters_flag
                && (f_equality(clusters_arr[i].x, n_x) && f_equality(clusters_arr[i].y, n_y));
            omp_unset_lock(&update_lock);

            clusters_arr[i].x = n_x;
            clusters_arr[i].y = n_y;
         }

        #ifdef DEBUG_P
        cout << "New centroid coordinates:" << endl;
        for (int i = 0; i < command_line_args.clust_num; i++) {
            cout << "Cluster:" << i << '(' << clusters_arr[i].x << ',' << clusters_arr[i].y << ')' << endl;
        }
        cout << endl;
        #endif

        // Если координаты центроидов не изменились, то заканчиваем вычисления
        if (same_clusters_flag) {
            break;
        }

        // Очистка получившихся кластеров
        for (int i = 0; i < command_line_args.clust_num; i++) {
            belong_list[i].clear();
        }
    }

    c_stop = omp_get_wtime();

    omp_destroy_lock(&update_lock);

    // * =================
    // * Вывод результатов
    // * =================

    cout.precision(2);

    if (command_line_args.test_flag) {
        for (int i = 0; i < command_line_args.clust_num; i++) {
            cout << "Centroid:" << fixed << i+1 << " (" << clusters_arr[i].x
                 << ':' << clusters_arr[i].y << ')' << endl;
            for (int j = 0; j < belong_list[i].size(); j++) {
                cout << " point (" << points_arr[belong_list[i][j]].x <<  ':'
                    << points_arr[belong_list[i][j]].x << ')' << endl;
            }
        }
    }
    else {
        cout << "Time of calculation is " << fixed << c_stop - c_start << endl;
    }

    #ifdef BENCHMARK
    ofstream out("result.csv", ios_base::app);
    string time = to_string(c_stop - c_start);
    out << command_line_args.point_num << ';'
        << command_line_args.clust_num << ';'
        << command_line_args.proc_num << ';'
        << time.replace(time.find("."), 1, ",") << endl;
    out.close();
    #endif

    // * ==============
    // * Очистка памяти
    // * ==============

    delete[] belong_list;
    delete[] points_arr;
    delete[] clusters_arr;

    return 0;
}
