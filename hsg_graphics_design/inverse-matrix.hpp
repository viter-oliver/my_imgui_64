#pragma once
#include <vector>
namespace extendable_matrix{
template <class T>
struct Matrix {
    int maxRow;
    int maxCol;
    std::vector<std::vector<T>> matrixdata;
    
    Matrix(int row, int col) {
        matrixdata.resize(col, std::vector<T>(row, 0));
        maxRow = row;
        maxCol = col;
    }
    Matrix(int msize):Matrix(msize,msize){}
    std::vector<T>& operator[](int row) {
        return matrixdata[row];
    }
    bool multiply(std::vector<T>& input, std::vector<T>& output) {
        
        if (input.size() != maxCol || input.size() != maxRow) {
            return false;
        }
        auto vector_multiply = [](std::vector<T>& lv, std::vector<T>& rv)->T {
            if (lv.size() != rv.size()) {
                return 0;
            }
            T sm = 0;
            for (int ix = 0; ix < lv.size(); ix++) {
                sm += (lv[ix] * rv[ix]);
            }
            return sm;
        };
        for (int ix = 0; ix < maxCol; ++ix) {
            output[ix] = vector_multiply(matrixdata[ix], input);
        }
        return true;
    }
    
    //	calculate minor of matrix OR build new matrix : k-had = minor
    void Minor(int colMatrix, Matrix<T>& newMinorMatrix) {
        int col, row, row2 = 0, col2 = 0;
        auto sizeMatrix = maxRow;
        for (row = 1; row < sizeMatrix; row++) {
            for (col = 0; col < sizeMatrix; col++) {
                if (col == colMatrix) {
                    continue;
                }
                newMinorMatrix[row2][col2] = matrixdata[row][col];
                col2++;
                if (col2 == (sizeMatrix - 1)) {
                    row2++;
                    col2 = 0;
                }
            }
        }
        //return;
    }// end function
        //	calculate determinte of matrix

    T Determinte(int size) {
        int col;
        T sum = 0;  
        if (size == 1) {
            sum= matrixdata[0][0];
        }
        else if (size == 2) {
            sum=(matrixdata[0][0] * matrixdata[1][1] - matrixdata[0][1] * matrixdata[1][0]);
        }
        else {
            Matrix<T> newMinorMatrix(size,size);
            for (col = 0; col < size; col++) {
                Minor(col, newMinorMatrix);	// function
                sum += (float)(matrixdata[0][col] * pow(-1, col) * newMinorMatrix.Determinte(size - 1));	// function
            }
        }
        return sum;
    }// end function
    void TransposeCalculate(Matrix<T>& coutMatrix,T determinte) {
        int row, col;
        for (row = 0; row < maxRow; row++) {
            for (col = 0; col < maxCol; col++) {
                //transposeMatrix[row][col] = cofactorMatrix[col][row];
                coutMatrix[row][col] = matrixdata[col][row] / determinte; // adjoint method
            }
        }
        //return;
    }// end function
    void Transpose(Matrix<T>& transposeMatrix) {
        int row, col;
        for (row = 0; row < maxRow; row++) {
            for (col = 0; col < maxCol; col++) {
                transposeMatrix[row][col] = cofactorMatrix[col][row];
            }
        }
    }
    bool Inverse(Matrix<T>& coutMatrix) {
        T determinte = Determinte(maxRow);
        if (determinte == 0) {
            //cout << "\n Inverse of entered matrix is not possible \n";
            return false;
        }
        else if (maxRow == 1) {
            coutMatrix[0][0] = 1;
        }
        else {
            Cofactor(coutMatrix, determinte);	// function
        }
        return true;
        //return;
    }// end function

    //---------------------------------------------------
    //	calculate cofactor of matrix
    void Cofactor(Matrix<T>& coutMatrix,T determinte) {
        Matrix<T> minorMatrix(maxRow, maxCol), cofactorMatrix(maxRow, maxCol);
        int col3, row3, row2, col2, row, col;
        for (row3 = 0; row3 < maxRow; row3++) {
            for (col3 = 0; col3 < maxCol; col3++) {
                row2 = 0;
                col2 = 0;
                for (row = 0; row < maxRow; row++) {
                    for (col = 0; col < maxCol; col++) {
                        if (row != row3 && col != col3) {
                            minorMatrix[row2][col2] = matrixdata[row][col];
                            if (col2 < (maxCol - 2)) {
                                col2++;
                            }
                            else {
                                col2 = 0;
                                row2++;
                            }
                        }
                    }
                }
                cofactorMatrix[row3][col3] = pow(-1, (row3 + col3)) * minorMatrix.Determinte(maxCol - 1);
            }
        }
        cofactorMatrix.TransposeCalculate(coutMatrix, determinte);	// function
        //return;
    }// end function

    void getCofactor(Matrix<T>& temp, int p, int q, int n) {
        int i = 0, j = 0;
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (row != p && col != q) {
                    temp[i][j++] = matrixdata[row][col];
                    if (j == n - 1) {
                        j = 0;
                        i++;
                    }
                }
            }
        }
    }
    T determinant(int n) {
        if (n == 1) {
            return matrixdata[0][0];
        }
        if (n == 2) {
            return ((matrixdata[0][0] * matrixdata[1][1]) -
                (matrixdata[1][0] * matrixdata[0][1]));
        }
        T D = 0;
        Matrix<T> temp(maxRow - 1, maxCol - 1);
        float sign = 1.f;
        for (int f = 0; f < n; f++) {
            getCofactor(temp, 0, f, n);
            D += sign * matrixdata[0][f] * temp.determinant(n - 1);
            sign = -sign;
        }
        return D;
    }
    void adjoint(Matrix& adj) {
        if (matrixdata.size() == 1) {
            adj.matrixdata[0][0] = 1;
            return;
        }
        float sign = 1.f;
        Matrix<T> temp(maxRow, maxCol);

        for (int i = 0; i < matrixdata.size(); i++) {
            for (int j = 0; j < matrixdata.size(); j++) {
                getCofactor(temp, i, j, matrixdata.size());
                sign = ((i + j) % 2 == 0) ? 1.f : -1.f;
                adj.matrixdata[i][j] =
                    (sign) * (temp.determinant(matrixdata.size() - 1));
            }
        }
    }
    bool inverse(Matrix<T>& inverse) {
        T det = determinant(matrixdata.size());
        if (det >= -0.00000001f && det <= 0.00000001f) {
            // cout << "Singular matrix, can't find its inverse";
            return false;
        }
        Matrix<T> adj(maxRow, maxCol);
        adjoint(adj);
        for (int i = 0; i < matrixdata.size(); i++)
            for (int j = 0; j < matrixdata.size(); j++)
                inverse.matrixdata[i][j] = adj.matrixdata[i][j] / det;
        return true;
    }
};
}