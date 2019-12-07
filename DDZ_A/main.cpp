#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <cmath>
#include <omp.h>
#include "mpich/mpi.h"

#define RAND_START 1
#define RAND_STOP 600

#define MASTER_PROC_RANK 0 
#define TAG 6

using namespace std;

struct command_line_args_t {
    int seq_num;
    int seq_len;
    int proc_num;
    bool test_flag;
    string test_filename;
};

void right_shift(int *arr_src, int *arr_dest, int arr_size, int repeats) {
    int *arr_tmp = new int[arr_size];
    memcpy(arr_tmp, arr_src, sizeof(int) * arr_size);
    for (int i = 0; i < repeats; i++) {
        for (int j = 0; j < arr_size; j++) {
            if (j != 0) {
                arr_dest[j] = arr_tmp[j - 1];
            }
            else {
                arr_dest[j] = 0;
            }
        }
        memcpy(arr_tmp, arr_dest, sizeof(int) * arr_size);
    }
    delete[] arr_tmp; 
}

int* pref_sum(int* m_array, int &seq_len) {
    int *s_arr, *q_arr;

    s_arr = new int[seq_len];
    q_arr = new int[seq_len];
        
    memcpy(s_arr, m_array, sizeof(int) * seq_len);
    memcpy(q_arr, m_array, sizeof(int) * seq_len);

    for (int j = 0; j < log2(seq_len); j++) {
        right_shift(s_arr, q_arr, seq_len, pow(2, j));
        for (int k = 0; k < seq_len; k++) {
            s_arr[k] = s_arr[k] + q_arr[k];
        }
    }

    delete[] q_arr;

    return s_arr;
}

int main(int argc, char** argv) {
    int comm_size, comm_rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

    if (comm_rank == MASTER_PROC_RANK) { // Код управляющего процесса (0)
        // Настройки по умолчанию
        command_line_args_t command_line_args;
        command_line_args.seq_num = -1;
        command_line_args.seq_len = -1;
        command_line_args.proc_num = -1;
        command_line_args.test_flag = false;
        //command_line_args.test_filename = ""; // TODO empty string

        int comm_line_posit;
        try {
            for (comm_line_posit  = 1; comm_line_posit < argc; comm_line_posit++) {
                if(strcmp("-t", argv[comm_line_posit]) == 0) {
                    command_line_args.test_flag = true;
                    command_line_args.test_filename = argv[++comm_line_posit];
                    continue;
                }
                else if (strcmp("-p", argv[comm_line_posit]) == 0) {
                    command_line_args.proc_num = stoi(argv[++comm_line_posit]);
                    if (command_line_args.proc_num <= 0 || command_line_args.proc_num > 8) {
                        throw invalid_argument("Incorrect kernel number");
                    }
                    continue;
                }
                else if (strcmp("-m", argv[comm_line_posit]) == 0) {
                    command_line_args.seq_num = stoi(argv[++comm_line_posit]);
                    if (command_line_args.seq_num <= 0 || command_line_args.proc_num % comm_size != 0) {
                        throw invalid_argument("Sequence number should be above 0");
                    }
                    continue;
                }
                else if (strcmp("-n", argv[comm_line_posit]) == 0) {
                    command_line_args.seq_len = stoi(argv[++comm_line_posit]);
                    if (command_line_args.seq_len <= 0) {
                        throw invalid_argument("Sequence length should be above 0");
                    }
                    continue;
                }
                else {
                    cout << "Error, Unknown command line argument: " << argv[comm_line_posit] << endl;
                    return -1;
                } 
            }
        }
        catch (invalid_argument) {
            cout << "Error, invalid argumnet value: " << argv[comm_line_posit] << endl;
            return -1;
        }
        catch (out_of_range) {
            cout << "Error, argument value out of range: "  << argv[comm_line_posit] << endl;
            return -1;
        }

        if (command_line_args.seq_num == -1) {
            cout << "Error, sequence number is required argument!" << endl;
            return -2;
        }
        else if (command_line_args.seq_len == -1) {
            cout << "Error, sequence length is required argument!" << endl;
            return -2;
        }
        else if (command_line_args.proc_num == -1) {
            cout << "Error, sequence number is required argument!" << endl;
            return -2;
        }

        #ifdef DEBUG
        cout << "Command args:" << endl;
        cout << "Program: " << argv[0] << endl;
        cout << "M: " << command_line_args.seq_num << endl;
        cout << "N: " << command_line_args.seq_len << endl;
        cout << "P: " << command_line_args.proc_num << endl;
        cout << "Test mode: " << command_line_args.test_flag << endl;
        cout << "Test file: " << command_line_args.test_filename << endl;
        #endif

        // Выделение памяти под последовательности
        int **m_arrays;
        m_arrays = new int*[command_line_args.seq_num];
        for (int i = 0; i < command_line_args.seq_num; i++) {
            m_arrays[i] = new int[command_line_args.seq_len];
        }
        
        int i, j;
        string input_buf;
        regex num_re("([-.,\\w]+)");
        smatch match;

        // Заполение массива полседовательностей
        if (command_line_args.test_flag) {
            // В случае запуска в тестовом режиме читаем данные из файла
            ifstream fin(command_line_args.test_filename); 
            if (fin.is_open()) {
                i = 0;
                while (!fin.eof()) {
                    getline(fin, input_buf);
                    if (input_buf.empty()) {
                        continue;
                    }
                    sregex_iterator next(input_buf.begin(), input_buf.end(), num_re), end;
                    ptrdiff_t match_count(distance(next, end));
                    if (int(match_count) == command_line_args.seq_len) {
                        j = 0;
                        while (next != end) {
                            smatch match = *next++;
                            try {
                                m_arrays[i][j++] = stoi(match.str());
                                if (match.str().find(',') != string::npos) {
                                    throw invalid_argument("");
                                }
                            }
                            catch (invalid_argument) {
                                cout << "Error in line: " << input_buf << " (" << i+1 << ',' << j << ") invalid argument" << endl;
                                return -3;
                            }
                            catch (out_of_range) {
                                cout << "Error in line: " << input_buf << " (" << i+1 << ',' << j << ") int overflow" << endl;
                                return -1;
                            }
                        }
                        i++;
                    }
                    else {
                        cout << "Error in line: " << input_buf << " incorrect length" << endl;
                        return -3; 
                    }
                }
                if (i != command_line_args.seq_num) {
                    cout << "Error, not enought seqs!" << endl;
                    return -3;
                }
            }
            else {
                cout << "Error, file: " << command_line_args.test_filename << " can't be opened!" << endl;
                return -3;
            }
            
            fin.close();
        }
        else {
            // В случае запуска в режиме эксперемента генереруем случайным образом
            for (int i = 0; i < command_line_args.seq_num; i++) {
                    for (int j = 0; j < command_line_args.seq_len; j++) {
                        m_arrays[i][j] = RAND_START + rand() % RAND_STOP;
                    }
            }
        }
        
        #ifdef DEBUG 
        cout << endl << "M_Arrays" << endl;
        for (int i = 0; i < command_line_args.seq_num; i++) {
                for (int j = 0; j < command_line_args.seq_len; j++) {
                    cout << m_arrays[i][j] << " ";
                }
                cout << endl;
        }
        cout << endl;
        #endif

        clock_t c_start, c_stop;
        double calc_time;

        c_start = clock();

        // Указываем всем процессам длину последовательностей и количество обрабатываемых послдеовательностей 
        int seq_per_proc = command_line_args.seq_num / comm_size;
        MPI_Bcast(&command_line_args.seq_len, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);
        MPI_Bcast(&seq_per_proc, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);

        // Рассылка последовательностей по процессам
        int dest_num, tmp_i = 0;
        int *master_seq_nums = new int[seq_per_proc];
        for(int i = 0; i < command_line_args.seq_num; i++) {
            dest_num = i % comm_size;
            if (dest_num == MASTER_PROC_RANK) { // Кроме нулевого потока
                master_seq_nums[tmp_i++] = i;
                continue;
            }
            MPI_Send(m_arrays[i], command_line_args.seq_len, MPI_INT, dest_num, TAG, MPI_COMM_WORLD);
        }

        // * Calculations
        int **pref_sum_arr = new int*[command_line_args.seq_num];

        #pragma omp parallel for
        for(int i = 0; i < seq_per_proc; i++) {
            pref_sum_arr[master_seq_nums[i]] = pref_sum(m_arrays[master_seq_nums[i]], command_line_args.seq_len);
        }

        #ifdef DEBUG 
        string pref_sum_str;
        for (int i = 0; i < seq_per_proc; i++) {
            pref_sum_str = "Proc#" + to_string(comm_rank) + " pref_sum arr: ";
            for (int j = 0; j < command_line_args.seq_len; j++) {
                pref_sum_str += to_string(pref_sum_arr[i][j]) + " ";
            }
            printf("%s\n", pref_sum_str.c_str());
        }
        #endif

        for (int i = 0; i < command_line_args.seq_num; i++) {
            dest_num = i % comm_size;
            if (dest_num == MASTER_PROC_RANK) {
                continue;
            }
            pref_sum_arr[i] =  new int[command_line_args.seq_len];
            MPI_Recv(pref_sum_arr[i], command_line_args.seq_len, MPI_INT, dest_num, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        c_stop = clock();
        calc_time = (double)(c_stop - c_start) / CLOCKS_PER_SEC;

        if (command_line_args.test_flag) {
            for (int i = 0; i < command_line_args.seq_num; i++) {
                for (int j = 0; j < command_line_args.seq_len; j++) {
                    cout << pref_sum_arr[i][j] << " ";
                }
                cout << endl;
            }
        }
        cout << "Time of calculation is " << calc_time << "ms" << endl;

        // Очистка всех динамических массивов
        for (int i = 0; i < command_line_args.seq_num; i++) {
            delete[] m_arrays[i];
            delete[] pref_sum_arr[i];
        }
        delete[] m_arrays;
        delete[] master_seq_nums; 
        delete[] pref_sum_arr;
    }
    else { // Код процессов, осуществляющих обработку
        int local_seq_len, local_seq_num;
        int **local_m_arrays;

        MPI_Bcast(&local_seq_len, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);
        MPI_Bcast(&local_seq_num, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);

        local_m_arrays = new int*[local_seq_num];
        for (int i = 0; i < local_seq_num; i++) {
            local_m_arrays[i] = new int[local_seq_len];
        }

        #ifdef DEBUG 
        printf("Proc#%d seq_len: %d seq_num: %d\n", comm_rank, local_seq_len, local_seq_num);
        #endif

        for (int i = 0; i < local_seq_num; i++) {
            MPI_Recv(local_m_arrays[i], local_seq_len, MPI_INT, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // * Calculations
        int **pref_sum_arr = new int*[local_seq_num];

        #pragma omp parallel for
        for (int i = 0; i < local_seq_num; i++) {
            pref_sum_arr[i] = pref_sum(local_m_arrays[i], local_seq_len);
        }

        #ifdef DEBUG 
        string pref_sum_str;
        for (int i = 0; i < local_seq_num; i++) {
            pref_sum_str = "Proc#" + to_string(comm_rank) + " pref_sum arr: ";
            for (int j = 0; j < local_seq_len; j++) {
                pref_sum_str += to_string(pref_sum_arr[i][j]) + " ";
            }
            printf("%s\n", pref_sum_str.c_str());
        }
        #endif

        for (int i = 0; i < local_seq_num; i++) {
            MPI_Send(pref_sum_arr[i], local_seq_len, MPI_INT, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD);
        }

        for (int i = 0; i < local_seq_num; i++) {
            delete[] local_m_arrays[i];
            delete[] pref_sum_arr[i];
        }
        delete[] local_m_arrays;
        delete[] pref_sum_arr;
    }

    MPI_Finalize();

    return 0;
}
