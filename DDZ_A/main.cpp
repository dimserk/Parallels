#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <regex>
#include <cmath>
#include <omp.h>
#include "mpich/mpi.h"

#define RAND_START 1
#define RAND_STOP 6000

#define SHARED_PARAMS_SIZE 3
#define SEQ_LEN_POSIT 0
#define SEQ_PER_PROC_POSIT 1
#define PROC_NUM_POSIT 2

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

void right_shift(double *arr_src, double *arr_dest, int arr_size, int repeats) {
    double *arr_tmp = new double[arr_size];
    memcpy(arr_tmp, arr_src, sizeof(double) * arr_size);
    for (int i = 0; i < repeats; i++) {
        for (int j = 0; j < arr_size; j++) {
            if (j != 0) {
                arr_dest[j] = arr_tmp[j - 1];
            }
            else {
                arr_dest[j] = 0;
            }
        }
        memcpy(arr_tmp, arr_dest, sizeof(double) * arr_size);
    }
    delete[] arr_tmp; 
}

double* pref_sum(double* m_array, int &seq_len) {
    double *s_arr, *q_arr;

    s_arr = new double[seq_len];
    q_arr = new double[seq_len];
        
    memcpy(s_arr, m_array, sizeof(double) * seq_len);
    memcpy(q_arr, m_array, sizeof(double) * seq_len);

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
                    if (command_line_args.seq_num <= 0 || command_line_args.seq_num % comm_size != 0) {
                        cout << comm_size << ": "<< command_line_args.proc_num % comm_size << endl;
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
                    cout << "Unknown argument: " << argv[comm_line_posit] << endl;
                    return -1;
                } 
            }
        }
        catch (invalid_argument) {
            cout << argv[comm_line_posit-1] << " is not set correctly" << endl;
            return -1;
        }
        catch (out_of_range) {
            cout << "Error, argument value out of range: "  << argv[comm_line_posit] << endl;
            return -1;
        }

        #ifdef DEBUG
        cout << "Command args:" << endl;
        cout << "Program:" << '\t' << argv[0] << endl;
        cout << "M argum:" << '\t' << command_line_args.seq_num << endl;
        cout << "N argum:" << '\t' << command_line_args.seq_len << endl;
        cout << "P argum:" << '\t' << command_line_args.proc_num << endl;
        cout << "Test mode:" << '\t' << command_line_args.test_flag << endl;
        cout << "Test file:" << '\t' << command_line_args.test_filename << endl;
        #endif

        // Проверка обязательных параметров
        if (command_line_args.seq_num == -1) {
            cout << "Not enought arguments: -m is requirement agument!" << endl;
            return -2;
        }
        else if (command_line_args.seq_len == -1) {
            cout << "Not enought arguments: -n is required argument!" << endl;
            return -2;
        }
        else if (command_line_args.proc_num == -1) {
            cout << "Not enought arguments: -p is required argument!" << endl;
            return -2;
        }

        // Выделение памяти под последовательности
        double **m_arrays;
        m_arrays = new double*[command_line_args.seq_num];
        for (int i = 0; i < command_line_args.seq_num; i++) {
            m_arrays[i] = new double[command_line_args.seq_len];
        }
        
        int i, j;
        string input_buf, tmp_str;

        // Заполение массива полседовательностей
        if (command_line_args.test_flag) {
            // В случае запуска в тестовом режиме читаем данные из файла
            ifstream fin(command_line_args.test_filename); 
            if (fin.is_open()) {
                i = 0;
                do {
                    getline(fin, input_buf);
                    if (input_buf.empty()) {
                        continue;
                    }

                    stringstream str_stream(input_buf);
                    j = 0;
                    do {
                        str_stream >> tmp_str;
                        try {
                            m_arrays[i][j++] = stod(tmp_str);
                            if (tmp_str.find(',') != string::npos) {
                                throw invalid_argument("");
                            }
                        }
                        catch (invalid_argument) {
                            cout << "Incorrect value (" << i+1 << ',' << j << ") in input file" << endl;
                            return -3;
                        }
                        catch (out_of_range) {
                            cout << "Incorrect value (" << i+1 << ',' << j << ") in input file" << endl;
                            return -3;
                        }
                    } while (!str_stream.eof());
                    
                    if (j != command_line_args.seq_len) {
                        cout << "Error in line: " << input_buf << " incorrect length" << endl;
                        return -3;
                    }
                    i++;
                } while (!fin.eof());

                if (i != command_line_args.seq_num) {
                    cout << "Error, not enought seqs!" << endl;
                    return -3;
                }
            }
            else {
                cout << "Unanle to open input file: permission denied or no file" << endl;
                return -3;
            }
            
            fin.close();
        }
        else {
            // В случае запуска в режиме эксперемента генереруем случайным образом
            for (int i = 0; i < command_line_args.seq_num; i++) {
                    for (int j = 0; j < command_line_args.seq_len; j++) {
                        m_arrays[i][j] = ((RAND_START + rand()) % RAND_STOP) / 100.0;
                    }
            }
        }
        
        #ifdef DEBUG 
        cout << endl << "M_Arrays" << endl;
        for (int i = 0; i < command_line_args.seq_num; i++) {
                for (int j = 0; j < command_line_args.seq_len; j++) {
                    cout << m_arrays[i][j] << ' ';
                }
                cout << endl;
        }
        cout << endl;
        #endif

        double t_start, t_stop;
        
        t_start = MPI_Wtime();

        // Указываем всем процессам длину последовательностей и количество обрабатываемых послдеовательностей 
        int seq_per_proc = command_line_args.seq_num / comm_size;
        int shared_params[SHARED_PARAMS_SIZE] = {command_line_args.seq_len, seq_per_proc, command_line_args.proc_num};
        for (int i_dest = 1; i_dest < comm_size; i_dest++) {
            MPI_Send(shared_params, SHARED_PARAMS_SIZE, MPI_INT, i_dest, TAG, MPI_COMM_WORLD);
        }

        // Рассылка последовательностей по процессам
        int dest_num, tmp_i = 0;
        int *master_seq_nums = new int[seq_per_proc];
        for(int i = 0; i < command_line_args.seq_num; i++) {
            dest_num = i % comm_size;
            if (dest_num == MASTER_PROC_RANK) { // Кроме нулевого потока
                master_seq_nums[tmp_i++] = i;
                continue;
            }
            MPI_Send(m_arrays[i], command_line_args.seq_len, MPI_DOUBLE, dest_num, TAG, MPI_COMM_WORLD);
        }

        // * Calculations
        double **pref_sum_arr = new double*[command_line_args.seq_num];

        omp_set_num_threads(command_line_args.proc_num);
        #pragma omp parallel for
        for(int i = 0; i < seq_per_proc; i++) {
            pref_sum_arr[master_seq_nums[i]] = pref_sum(m_arrays[master_seq_nums[i]], command_line_args.seq_len);
        }

        #ifdef DEBUG 
        string pref_sum_str;
        for (int i = 0; i < seq_per_proc; i++) {
            pref_sum_str = "Proc#" + to_string(comm_rank) + " pref_sum arr: ";
            for (int j = 0; j < command_line_args.seq_len; j++) {
                pref_sum_str += to_string(pref_sum_arr[master_seq_nums[i]][j]) + ' ';
            }
            printf("%s\n", pref_sum_str.c_str());
        }
        printf("\n");
        #endif

        for (int i = 0; i < command_line_args.seq_num; i++) {
            dest_num = i % comm_size;
            if (dest_num == MASTER_PROC_RANK) {
                continue;
            }
            pref_sum_arr[i] = new double[command_line_args.seq_len];
            MPI_Recv(pref_sum_arr[i], command_line_args.seq_len, MPI_DOUBLE, dest_num, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        t_stop = MPI_Wtime();

        if (command_line_args.test_flag) {
            cout << "Исходные последовательности:" << endl;
            for (int i = 0; i < command_line_args.seq_num; i++) {
                for (int j = 0; j < command_line_args.seq_len; j++) {
                    cout << m_arrays[i][j] << '\t';
                }
                cout << endl;
            }

            cout << "Частные суммы:" << endl;
            for (int i = 0; i < command_line_args.seq_num; i++) {
                for (int j = 0; j < command_line_args.seq_len; j++) {
                    cout << pref_sum_arr[i][j] << '\t';
                }
                cout << endl;
            }
        }
        else {
            cout << "Calculation time = " << t_stop - t_start << endl;
        }

        #ifdef CHECK
        bool check;
        double sum;
        string result;
        int total_pass = 0;
        
        printf("Testing...\n");
        for (int i = 0; i < command_line_args.seq_num; i++) {
            sum = 0;
            check = true;
            for (int j = 0; j < command_line_args.seq_len; j++) {
                sum += m_arrays[i][j];
                check = check && fabs(sum - pref_sum_arr[i][j]) < 0.01;
            }
            if (check) {
                total_pass += 1;
            }
            if (command_line_args.test_flag) {
                result = check ? "ok" : "failed";
                printf("Seq#%d:\t%s\n", i+1, result.c_str());
            }
        }
        printf("Result: %d passed, %d failed\n", total_pass, command_line_args.seq_num - total_pass);
        #endif

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
        double **local_m_arrays;
        int shared_params[SHARED_PARAMS_SIZE];

        MPI_Recv(shared_params, SHARED_PARAMS_SIZE, MPI_INT, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        #ifdef DEBUG
        string recv_params;
        for (int i = 0; i < SHARED_PARAMS_SIZE; i++) {
            recv_params += to_string(shared_params[i]) + ' ';
        }
        printf("Proc#%d recv_params: %s\n",comm_rank, recv_params.c_str());
        #endif

        local_m_arrays = new double*[shared_params[SEQ_PER_PROC_POSIT]];
        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            local_m_arrays[i] = new double[shared_params[SEQ_LEN_POSIT]];
        }

        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            MPI_Recv(local_m_arrays[i], shared_params[SEQ_LEN_POSIT], MPI_DOUBLE, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // * Calculations
        double **pref_sum_arr = new double*[shared_params[SEQ_PER_PROC_POSIT]];

        omp_set_num_threads(shared_params[PROC_NUM_POSIT]);
        #pragma omp parallel for
        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            pref_sum_arr[i] = pref_sum(local_m_arrays[i], shared_params[SEQ_LEN_POSIT]);
        }

        #ifdef DEBUG 
        string pref_sum_str;
        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            pref_sum_str = "Proc#" + to_string(comm_rank) + " pref_sum arr: ";
            for (int j = 0; j < shared_params[SEQ_LEN_POSIT]; j++) {
                pref_sum_str += to_string(pref_sum_arr[i][j]) + " ";
            }
            printf("%s\n", pref_sum_str.c_str());
        }
        #endif

        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            MPI_Send(pref_sum_arr[i], shared_params[SEQ_LEN_POSIT], MPI_DOUBLE, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD);
        }

        for (int i = 0; i < shared_params[SEQ_PER_PROC_POSIT]; i++) {
            delete[] local_m_arrays[i];
            delete[] pref_sum_arr[i];
        }
        delete[] local_m_arrays;
        delete[] pref_sum_arr;
    }

    MPI_Finalize();

    return 0;
}
