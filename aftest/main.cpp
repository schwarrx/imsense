#include <arrayfire.h>
#include <af/cuda.h>
#include <af/util.h>
#include <iostream>
 
using namespace af;
 
static size_t dimension =  8*1024;
static const int maxIter = 10;
static const int sparsityFactor = 7;
 
static array A;
static array spA;  // Sparse A
static array x0;
static array b;
 

using namespace af;

void print_gpu_memory() {
  // show memory usage of GPU
  size_t free_byte ;
  size_t total_byte ;

  cudaError_t cuda_status = cudaMemGetInfo( &free_byte, &total_byte ) ;

  if ( cudaSuccess != cuda_status ){
    printf("Error: cudaMemGetInfo fails, %s \n", cudaGetErrorString(cuda_status) );
    exit(1);
  }

  double free_db = (double)free_byte ;
  double total_db = (double)total_byte ;
  double used_db = total_db - free_db ;
  af::printMemInfo("af::printMemInfo ");
  printf("GPU memory usage: used = %f MB, free = %f MB, total = %f MB\n",
         used_db/1024.0/1024.0, free_db/1024.0/1024.0, total_db/1024.0/1024.0);
}

void print_gpu_memory(const char* s) {
  printf("%s\t", s);
  print_gpu_memory();
}

void setupInputs()
{
    // Generate a random input: A
    array T = randu(dimension, dimension, f32);
    // Create 0s in input.
    // Anything that is no divisible by sparsityFactor will become 0. 
    A = floor(T * 1000);
    A = A * ((A % sparsityFactor) == 0) / 1000;
    print_gpu_memory("Allocated A matrix");
 
    // Make it positive definite
    A = transpose(A) + A + A.dims(0)*identity(A.dims(0), A.dims(0), f32);
 
    // Make A sparse as spA
    spA = sparse(A);
    print_gpu_memory("Computed spA matrix");
 
    // Generate x0: Random guess
    x0 = randu(A.dims(0), f32);
 
    //Generate b
    b = matmul(A, x0);

 
    std::cout << "Sparsity of A = "
              << 100.f * (float)sparseGetNNZ(spA) / (float)spA.elements()
              << std::endl;
    std::cout << "Memory Usage of A = " <<  A.bytes() << " bytes" << std::endl;
    std::cout << "Memory Usage of spA = "
              << sparseGetValues(spA).bytes()
               + sparseGetRowIdx(spA).bytes()
               + sparseGetColIdx(spA).bytes()
              << " bytes" << std::endl;
}


 void sparseConjugateGradient(void)
{
    array x = constant(0, b.dims(), f32);
    array r = b - matmul(spA, x);
    array p = r;
    for (int i = 0; i < maxIter; ++i) {
        array Ap = matmul(spA, p);
        array alpha_num = dot(r, r);
        array alpha_den = dot(p, Ap);
        array alpha = alpha_num/alpha_den;
        r -= tile(alpha, Ap.dims())*Ap;
        x += tile(alpha, Ap.dims())*p;
        array beta_num = dot(r, r);
        array beta = beta_num/alpha_num;
        p = r + tile(beta, p.dims()) * p;
    }
}


void denseConjugateGradient(void)
{
    array x = constant(0, b.dims(), f32);
    array r = b - matmul(A, x);
    array p = r;
    for (int i = 0; i < maxIter; ++i) {
        array Ap = matmul(A, p);
        array alpha_num = dot(r, r);
        array alpha_den = dot(p, Ap);
        array alpha = alpha_num/alpha_den;
        r -= tile(alpha, Ap.dims())*Ap;
        x += tile(alpha, Ap.dims())*p;
        array beta_num = dot(r, r);
        array beta = beta_num/alpha_num;
        p = r + tile(beta, p.dims()) * p;
    }
}


void checkConjugateGradient(const af::array in)
{
    array x = constant(0, b.dims(), f32);
    array r = b - matmul(in, x);
    array p = r;
    for (int i = 0; i < maxIter; ++i) {
        array Ap = matmul(in, p);
        array alpha_num = dot(r, r);
        array alpha_den = dot(p, Ap);
        array alpha = alpha_num/alpha_den;
        r -= tile(alpha, Ap.dims())*Ap;
        x += tile(alpha, Ap.dims())*p;
        array beta_num = dot(r, r);
        array beta = beta_num/alpha_num;
        p = r + tile(beta, p.dims()) * p;
    }
    array res = x0 - x;
    std::cout<<"Final difference in solutions:\n";
    af_print(dot(res, res));
}


int main(int argc, char *argv[])
{
    af::info();
    setupInputs();
    std::cout << "Verifying Dense Conjugate Gradient:" << std::endl;
    checkConjugateGradient(A);
    std::cout << "Verifying Sparse Conjugate Gradient:" << std::endl;
    checkConjugateGradient(spA);
    af::sync();
    std::cout << "Dense Conjugate Gradient Time: "
              << timeit(denseConjugateGradient) * 1000
              << "ms" << std::endl;
    std::cout << "Sparse Conjugate Gradient Time: "
              << timeit(sparseConjugateGradient) * 1000
              << "ms" << std::endl;
    return 0;
}
