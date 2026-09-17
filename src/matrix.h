#ifndef MATRIX_H
#define MATRIX_H
#include <cmath>
#include <iostream>

class matrix {
    private: float data[4][4]{};

    public: matrix() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                data[i][j] = 0;
            }
        }
    }

    float& operator()(int row, int col)
    {
        return data[row][col];
    }
    void print() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << this->data[i][j] << "\t";
            }
            std::cout <<"\n";
        }
    }
};

static matrix identity_matrix() {
    matrix m = matrix();
    for (int i = 0; i < 4; ++i) {
        m(i,i) = 1;
    }
    return m;
}

static matrix matricial_product(matrix row_m, matrix col_m){
    matrix p = matrix();
    for (int k = 0; k < 4; ++k) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                p(k, i) = p(k, i) + (row_m(k, j) * col_m(j, i));
            }
        }
    }
    return p;
}
static matrix translation(float dx, float dy, float dz) {
    matrix m = identity_matrix();
    m(0,3) = dx;
    m(1,3) = dy;
    m(2,3) = dz;
    return m;
}
static matrix rotation_around_axis(float rotation_angle, char axis) {
    matrix m = identity_matrix();
    float sin_a = std::sin(rotation_angle);
    float cos_a = std::cos(rotation_angle);
    if (axis == 'x') {
        m(1, 1) = cos_a;
        m(1, 2) = sin_a * -1;
        m(2, 1) = sin_a;
        m(2, 2) = cos_a;
    }
    if (axis == 'y') {
        m(0, 0) = cos_a;
        m(0, 2) = sin_a;
        m(2, 0) = sin_a * -1;
        m(2, 2) = cos_a;
    }
    if (axis == 'z') {
        m(0, 0) =  cos_a;
        m(0, 1) =  sin_a * -1;
        m(1, 0) =  sin_a;
        m(1, 1) =  cos_a;
    }
    return m;
}
static matrix scaling(float scaling_factor_x, float scaling_factor_y, float scaling_factor_z) {
    matrix m = identity_matrix();
    m(0,0) = scaling_factor_x;
    m(1,1) = scaling_factor_y;
    m(2,2) = scaling_factor_z;
    return m;
}
static matrix shear(char axis, float hx, float hy, float hz){
    matrix m = identity_matrix();
    if (axis == 'x') {
        m(1, 0) = hy;
        m(2, 0) = hz;
    }
    if (axis == 'y') {
        m(0,1) = hx;
        m(2,1) = hz;
    }
    if (axis == 'z') {
        m(0,2) = hx;
        m(1,2) = hz;
    }
    return m;
}

#endif // MATRIX_H
