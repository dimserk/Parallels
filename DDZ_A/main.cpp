#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <omp.h>
#include "mpich/mpi.h"

#define RAND_START 1
#define RAND_STOP 1000

#define MASTER_PROC_RANK 0 
#define TAG 6

using namespace std;

struct command_args_t {
    int seq_num;
    int seq_len;
    bool test_flag;
    string test_filename;
};

static const char* opt_string = "t:m:n:";

int main(int argc, char** argv) {
    int comm_size, comm_rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

    if (comm_rank == MASTER_PROC_RANK) {
        command_args_t master_args;
        master_args.seq_num = 10;
        master_args.seq_len = 30;
        master_args.test_flag = false;
        master_args.test_filename = ""; // TODO empty string

        opterr = 0;
        int opt = getopt(argc, argv, opt_string);
        while (opt != -1) {
            switch (opt) {
            case 't':
                master_args.test_flag = true;
                master_args.test_filename = optarg;
                break;
            
            case 'm':
                master_args.seq_num = atoi(optarg);
                break;

            case 'n':
                master_args.seq_len = atoi(optarg);
                break;

            default:
                cout << "Argument error! (" << argv[optind-1] << ")" << endl;
                break;
            }

            opt = getopt(argc, argv, opt_string);
        }

        #ifdef DEBUG
        cout << endl << "Command args" << endl;
        cout << "Program: " << argv[0] << endl;
        cout << "M: " << master_args.seq_num << endl;
        cout << "N: " << master_args.seq_len << endl;
        cout << "Test mode: " << master_args.test_flag << endl;
        cout << "Test file: " << master_args.test_filename << endl;
        #endif // DEBUG

        int **m_arrays;
        m_arrays = new int*[master_args.seq_num];
        for (int i = 0; i < master_args.seq_num; i++) {
            m_arrays[i] = new int[master_args.seq_len];
        }
        
        int tmp = 1;
        if (master_args.test_flag) {
            ifstream fin(master_args.test_filename);
            if (fin.is_open()) {
                for (int i = 0; i < master_args.seq_num; i++) {
                    for (int j = 0; j < master_args.seq_len; j++) {
                        fin >> tmp;
                        m_arrays[i][j] = tmp;
                    }
                }
            }
            fin.close();
        }
        else {
            for (int i = 0; i < master_args.seq_num; i++) {
                    for (int j = 0; j < master_args.seq_len; j++) {
                        m_arrays[i][j] = RAND_START + rand() % RAND_STOP;
                    }
            }
        }
        
        #ifdef DEBUG 
        cout << endl << "M_Arrays" << endl;
        for (int i = 0; i < master_args.seq_num; i++) {
                for (int j = 0; j < master_args.seq_len; j++) {
                    cout << m_arrays[i][j] << " ";
                }
                cout << endl;
        }
        #endif

        int proc_per_seq = master_args.seq_num / comm_size; // TODO remove this
        MPI_Bcast(&master_args.seq_len, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);
        MPI_Bcast(&proc_per_seq, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);

        int dest_num;
        for(int i = 0; i < master_args.seq_num; i++) {
            dest_num = i % comm_size;
            if (dest_num == MASTER_PROC_RANK) {
                continue;
            }
            MPI_Send(m_arrays[i], master_args.seq_len, MPI_INT, dest_num, TAG, MPI_COMM_WORLD);
        }

        for (int i = 0; i < master_args.seq_num; i++) {
            delete[] m_arrays[i];
        }
        delete[] m_arrays;
    }
    else {
        int local_seq_len, local_seq_num;
        int **local_m_arrays;

        MPI_Bcast(&local_seq_len, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);
        MPI_Bcast(&local_seq_num, 1, MPI_INT, MASTER_PROC_RANK, MPI_COMM_WORLD);

        local_m_arrays = new int*[local_seq_num];
        for (int i = 0; i < local_seq_num; i++) {
            local_m_arrays[i] = new int[local_seq_len];
        }

        #ifdef DEBUG 
        cout << "Proc#" << comm_rank << " seq size: " << local_seq_len << " seq num: " << local_seq_num << endl;
        #endif

        for (int i = 0; i < local_seq_num; i++) {
            MPI_Recv(local_m_arrays[i], local_seq_len, MPI_INT, MASTER_PROC_RANK, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        #ifdef DEBUG 
        int test_sum = 0;
        for (int i = 0; i < local_seq_num; i++) {
            for (int j = 0; j < local_seq_len; j++) {
                test_sum += local_m_arrays[i][j];
            }
        }
        cout << "Proc#" << comm_rank << " crc: " << test_sum << endl;
        #endif

        for (int i = 0; i < local_seq_num; i++) {
            delete[] local_m_arrays[i];
        }
        delete[] local_m_arrays;
    }

    MPI_Finalize();

    return 0;
}