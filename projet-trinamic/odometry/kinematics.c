#include <stdio.h>
#include <math.h>
#include <assert.h>

#define ROWS 4
#define COLS 3

#define EPSILON 1e-4

#define ASSERT_FLOAT_EQ(a, b) assert(fabs((a) - (b)) < EPSILON)
#define ASSERT_FLOAT_NEQ(a, b) assert(fabs((a) - (b)) > EPSILON)

void transpose(float A[ROWS][COLS], float AT[COLS][ROWS]) {
    for (int i = 0; i < COLS; i++)
        for (int j = 0; j < ROWS; j++)
            AT[i][j] = A[j][i];
}

void multiply3x3_3x4(float A[3][3], float B[3][4], float result[3][4]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++) {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++)
                result[i][j] += A[i][k] * B[k][j];
        }
}

void multiply_transpose(float A[COLS][ROWS], float B[ROWS][COLS], float result[COLS][COLS]) {
    for (int i = 0; i < COLS; i++)
        for (int j = 0; j < COLS; j++) {
            result[i][j] = 0;
            for (int k = 0; k < ROWS; k++)
                result[i][j] += A[i][k] * B[k][j];
        }
}

int inverse3x3(float A[3][3], float Ainv[3][3]) {
    float det =
        A[0][0]*(A[1][1]*A[2][2] - A[1][2]*A[2][1]) -
        A[0][1]*(A[1][0]*A[2][2] - A[1][2]*A[2][0]) +
        A[0][2]*(A[1][0]*A[2][1] - A[1][1]*A[2][0]);

    if (fabs(det) < 1e-6) {
        printf("Erreur : matrice non inversible.\n");
        return 0;
    }

    float invDet = 1.0 / det;

    Ainv[0][0] =  (A[1][1]*A[2][2] - A[1][2]*A[2][1]) * invDet;
    Ainv[0][1] = -(A[0][1]*A[2][2] - A[0][2]*A[2][1]) * invDet;
    Ainv[0][2] =  (A[0][1]*A[1][2] - A[0][2]*A[1][1]) * invDet;
    Ainv[1][0] = -(A[1][0]*A[2][2] - A[1][2]*A[2][0]) * invDet;
    Ainv[1][1] =  (A[0][0]*A[2][2] - A[0][2]*A[2][0]) * invDet;
    Ainv[1][2] = -(A[0][0]*A[1][2] - A[0][2]*A[1][0]) * invDet;
    Ainv[2][0] =  (A[1][0]*A[2][1] - A[1][1]*A[2][0]) * invDet;
    Ainv[2][1] = -(A[0][0]*A[2][1] - A[0][1]*A[2][0]) * invDet;
    Ainv[2][2] =  (A[0][0]*A[1][1] - A[0][1]*A[1][0]) * invDet;
	return 1;
}

void compute_pseudoinverse(float J[4][3], float J_pinv[3][4]) {
    float JT[3][4], JTJ[3][3], JTJ_inv[3][3];

    transpose(J, JT);                    
    multiply_transpose(JT, J, JTJ);   

    if (!inverse3x3(JTJ, JTJ_inv)) {
        printf("Erreur : JT*J non inversible.\n");
        return;
    }

    multiply3x3_3x4(JTJ_inv, JT, J_pinv); // J+ = (JTJ)^-1 * JT
}

void measure_position(float theta1, float theta2, float theta3, float theta4, float *x, float *y, float *theta){
	
	float l = 8.2; // distance entre une roue et le centre du robot 
    float r = 2.5; // diametre d'une roue 

    float J[4][3] = {
        {-0.5,         sqrtf(3)/2,  l},
        {-0.5,        -sqrtf(3)/2,  l},
        { sqrtf(2)/2, -sqrtf(2)/2,  l},
        { sqrtf(2)/2,  sqrtf(2)/2,  l}
    };

	float J_pinv[3][4];

    compute_pseudoinverse(J, J_pinv);

    // Appliquer facteur 1/r
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 4; j++){
            J_pinv[i][j] *= (1.0 / r);
		}
	}

	//compute J_pinv * [theat1, theta2, theta3, theta4]

    float thetas[4] = {theta1, theta2, theta3, theta4};

    // Calcul du produit J_pinv * thetas 
    float result[3] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            result[i] += J_pinv[i][j] * thetas[j];
        }
    }

    // Retour des valeurs calculées
    *x = result[0];
    *y = result[1];
    *theta = result[2];
}

int main() {

	float tests[4][4] = {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, -1.0f},
		{-1.0f, 1.0f, -1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
    };
	float dx, dy, dtheta;

	int i = 0;
	measure_position(tests[i][0], tests[i][1], tests[i][2], tests[i][3], &dx, &dy, &dtheta);
	printf("Le robot tourne en rond : ");
	printf("dx=%.4f, dy=%.4f, dTheta=%.4f\n", dx, dy, dtheta);
	ASSERT_FLOAT_EQ(dx, 0);
	ASSERT_FLOAT_EQ(dy, 0);
	ASSERT_FLOAT_NEQ(dtheta, 0);
	printf("OK \n");

	i++;
	measure_position(tests[i][0], tests[i][1], tests[i][2], tests[i][3], &dx, &dy, &dtheta);
	printf("Le robot se translate 1: ");
	printf("dx=%.4f, dy=%.4f, dTheta=%.4f\n", dx, dy, dtheta);
	ASSERT_FLOAT_EQ(dx, 0);
	ASSERT_FLOAT_NEQ(dy, 0);
	ASSERT_FLOAT_EQ(dtheta, 0);
	printf("OK \n");

	i++;
	measure_position(tests[i][0], tests[i][1], tests[i][2], tests[i][3], &dx, &dy, &dtheta);
	printf("Le robot se translate 2: ");
	printf("dx=%.4f, dy=%.4f, dTheta=%.4f\n", dx, dy, dtheta);
	ASSERT_FLOAT_EQ(dx, 0);
	ASSERT_FLOAT_NEQ(dy, 0);
	ASSERT_FLOAT_EQ(dtheta, 0);
	printf("OK \n");

	i++;
	measure_position(tests[i][0], tests[i][1], tests[i][2], tests[i][3], &dx, &dy, &dtheta);
	printf("Le robot ne bouge pas : ");
	printf("dx=%.4f, dy=%.4f, dTheta=%.4f\n", dx, dy, dtheta);
	ASSERT_FLOAT_EQ(dx, 0);
	ASSERT_FLOAT_EQ(dy, 0);
	ASSERT_FLOAT_EQ(dtheta, 0);
	printf("OK \n");

    return 0;
}
