#include <iostream>
#include <fstream>
#include <pthread.h>

struct matrix {
    int row, col;
    int **mat;
    
    matrix(int row, int col): row(row), col(col) {
        // allocate matrix space
        mat = new int*[row];
        for (int i = 0; i < row; ++i) {
            mat[i] = new int[col];

            // initialize all cells to 0
            for (int j = 0; j < col; ++j) {
                mat[i][j] = 0;
            }
        }
    }
    
    ~matrix() {
        for (int i = 0; i < row; ++i) {
            delete[]mat[i];
        }
        delete[]mat;
    }

    void print() {
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                std::cout << mat[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

struct arguments {
    matrix *A, *B, *result;
    int row;
    arguments(matrix* A, matrix* B, matrix* result, int row): A(A), B(B), result(result), row(row){}
};

bool create_matrices(matrix*& A, matrix*& B, char *filename) {
    std::ifstream fin;
    fin.open(filename);
    if (!fin.is_open()) {
        return false;
    }
    
    matrix **mat[] = {&A, &B};
    for (int i = 0; i < 2; ++i) {
        int r, c;
        fin >> r >> c;
        *mat[i] = new matrix(r, c);
        for (int j = 0; j < r; ++j) {
            for (int k = 0; k < c; ++k) {
                fin >> (*mat[i])->mat[j][k];
            }
        }
    }

    return true;
}

void* multiply(void *ptr){
    arguments *arg = (arguments*)ptr;
    for (int i = 0; i < arg->B->col; ++i) {
        for (int j = 0; j < arg->A->col; ++j) {
            arg->result->mat[arg->row][i] += arg->A->mat[arg->row][j] * arg->B->mat[j][i];
        }
    }
    return NULL;
}

int main(int argc, char*argv[]) {

    if (argc < 1) {
        std::cerr << "Input file not specified!" << std::endl;
        return 1;
    }
    
    // process inputs
    matrix *A = NULL, *B = NULL;

    if (!create_matrices(A, B, argv[1])) {
        std::cerr << "Invalid input file specified!" << std::endl;
        return 1;
    }

    if (A->col != B->row) {
        std::cerr << "Cols of A != Rows of B!" << std::endl;
        return 1;
    }

    // create result array
    matrix *result = new matrix(A->row, B->col);
    
    pthread_t *threads = new pthread_t[result->row];
    arguments **arg = new arguments*[result->row];

    for (int i = 0; i < result->row; ++i) {
        arg[i] = new arguments(A, B, result, i);
        pthread_create(&threads[i], NULL, multiply, (void*)arg[i]);
    }

    for (int i = 0; i < result->row; ++i) {
        pthread_join(threads[i], NULL);
        delete arg[i];
    }

    result->print();

    delete A, B, result;
    delete[]threads;
    delete[]arg;

    return 0;
}
