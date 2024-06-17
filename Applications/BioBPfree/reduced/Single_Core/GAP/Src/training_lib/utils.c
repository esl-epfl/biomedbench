#include "utils.h"
#include "defines.h"

#include <math.h>
#include <stdbool.h>


void scalar_by_vector_mul(my_type alpha, my_type *vec, my_type *output, int size)    {

    for (int i = 0; i < size; i++)  {

        output[i] = alpha * vec[i];

    }

}


void vector_by_vector_sum_4(my_type *vec1, my_type *vec2, my_type *vec3, my_type *vec4, my_type *vec_sum, int size)    {

    for (int i = 0; i < size; i++)  {

        vec_sum[i] = vec1[i] + vec2[i] + vec3[i] + vec4[i];

    }

}


my_type vector_by_vector_mul(my_type *vec1, my_type *vec2, int size)    {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += vec1[i] * vec2[i];

    }

    return sum;
}


void vector_by_vector_mul_elementwise(my_type *vec1, my_type *vec2, my_type *vec_out, int size) {

    for (int i = 0; i < size; i++)  {

        vec_out[i] = vec1[i] * vec2[i];

    }

}


void vector_by_vector_div_elementwise(my_type *vec1, my_type *vec2, my_type *vec_out, int size, my_type epsilon) {

    for (int i = 0; i < size; i++)  {

        vec_out[i] = vec1[i] / (vec2[i] + epsilon);

    }

}


void vector_sqrt(my_type *vec, int size)   {

    for (int i = 0; i < size; i++)  {

        #ifdef FLOATS
        vec[i] = sqrtf(vec[i]);
        #elif defined(DOUBLES)
        vec[i] = sqrt(vec[i]);
        #else
        vec[i] = sqrtl(vec[i]);
        #endif

    }

}


my_type vector_sum_squared(const my_type *vec, int size)   {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += vec[i] * vec[i];

    }

    return sum;

}


void matrix_by_diagonal_mul(my_type *mat, my_type *diag, my_type *output, int dim1, int dim2)    {

    // Input / output index
    int index = 0;

    for (int i = 0; i < dim1; i++)  {
        for (int j = 0; j < dim2; j++)  {

            output[index] += mat[index] * diag[j];
            index++;

        }
    }

}


void matrix_by_vector_mul(my_type *mat, my_type *vec, my_type *output, int dim1, int dim2)    {

    my_type sum;

    // Matrix index
    int mat_index = 0;

    for (int i = 0; i < dim1; i++)  {

        sum = 0;

        for (int j = 0; j < dim2; j++)  {

            sum += mat[mat_index++] * vec[j];

        }
        output[i] = sum;
    }

}


void vector_by_matrix_mul(my_type *vec, my_type *mat, my_type *output, int dim1, int dim2)    {

    my_type sum;

    for (int i = 0; i < dim2; i++)  {

        sum = 0;

        for (int j = 0; j < dim1; j++)  {

            sum += vec[j] * mat[j * dim1 + i];

        }

        output[i] = sum;
    }

}


// NOTE: Implemented only for floats/doubles!
my_type Euclidean_distance(my_type *vec1, my_type *vec2, int size)  {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);

    }

    #ifdef FLOATS
    return sqrtf(sum);
    #elif defined(DOUBLES)
    return sqrt(sum);
    #else
    return sqrtl((long double)sum);
    #endif

}


my_type Euclidean_norm(my_type *vec, int size)  {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += vec[i] * vec[i];

    }

    #ifdef FLOATS
    return sqrtf(sum);
    #elif defined(DOUBLES)
    return sqrt(sum);
    #else
    return sqrtl((long double)sum);
    #endif

}


my_type Sum_Abs_Diff(my_type *vec1, my_type *vec2, int size)  {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += ABS(vec1[i] - vec2[i]);

    }

    return sum;

}


my_type Avg_Abs_Diff(my_type *vec1, my_type *vec2, int size)  {

    my_type sum = 0;

    for (int i = 0; i < size; i++)  {

        sum += ABS(vec1[i] - vec2[i]);

    }

    return sum / size;

}


bool compare_floating(my_type f1, my_type f2, my_type tolerance)   {

    if (f2 < f1 + tolerance && f2 > f1 - tolerance)
        return true;
    else
        return false;

}


