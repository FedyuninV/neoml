/* Copyright © 2017-2020 ABBYY Production LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------------------------------------------------------*/

// This method is described in
// "Working Set Selection Using Second Order Information for Training Support Vector Machines"
// Rong-En Fan, Pai-Hsuen Chen, Chih-Jen Lin, 
// "Journal of Machine Learning Research" 6 (2005) 1889–1918
// http://www.csie.ntu.edu.tw/~cjlin/papers/quadworkset.pdf

#include <common.h>
#pragma hdrstop

#include <NeoML/TraditionalML/SvmKernel.h>
#include <NeoMathEngine/NeoMathEngineDefs.h>

namespace NeoML {

// Raise a number to a power: base**times
inline double power(double base, int times)
{
	double tmp = base, ret = 1.0;
	for(int t = times; t > 0; t /= 2)
	{
		if(t % 2 == 1) {
			ret *= tmp;
		}
		tmp = tmp * tmp;
	}
	return ret;
}

CSvmKernel::CSvmKernel(TKernelType kernelType, int degree, double gamma, double coef0) :
	kernelType(kernelType), degree(degree), gamma(gamma), coef0(coef0)
{
}

// The linear kernel
double CSvmKernel::linear(const CFloatVectorDesc& x1, const CFloatVectorDesc& x2) const
{
	return DotProduct(x1, x2);
}

// The polynomial kernel
double CSvmKernel::poly(const CFloatVectorDesc& x1, const CFloatVectorDesc& x2) const
{
	return power(gamma * DotProduct(x1, x2) + coef0, degree);
}

// The Gaussian kernel
double CSvmKernel::rbf(const CFloatVectorDesc& x1, const CFloatVectorDesc& x2) const
{
	if( x1.Indexes == nullptr ) {
		if( x2.Indexes == nullptr ) {
			return rbfDenseByDense( x1, x2 );
		} else {
			return rbfDenseBySparse( x1, x2 );
		}
	} else {
		if( x2.Indexes == nullptr ) {
			return rbfDenseBySparse( x2, x1 );
		} else {
			return rbfSparseBySparse( x1, x2 );
		}
	}
}

// The sigmoid kernel
double CSvmKernel::sigmoid(const CFloatVectorDesc& x1, const CFloatVectorDesc& x2) const
{
	return tanh(gamma * DotProduct(x1, x2) + coef0);
}

double CSvmKernel::Calculate(const CFloatVectorDesc& x1, const CFloatVectorDesc& x2) const
{
	switch( kernelType ) {
		case KT_Linear:
			return linear(x1, x2);
		case KT_Poly:
			return poly(x1, x2);
		case KT_RBF:
			return rbf(x1, x2);
		case KT_Sigmoid:
			return sigmoid(x1, x2);
		default:
			NeoAssert(false);
			return 0;
	}
}

double CSvmKernel::rbfDenseBySparse( const CFloatVectorDesc& x1, const CFloatVectorDesc& x2 ) const
{
	double square = 0;
	double diff;
	int i, j;
	for( i = 0, j = 0; i < x1.Size && j < x2.Size; ) {
		if( i == x2.Indexes[j] ) {
			diff = x1.Values[i] - x2.Values[j];
			i++;
			j++;
		} else if( i < x2.Indexes[j] ) {
			diff = x1.Values[i];
			i++;
		} else {
			diff = x2.Values[j];
			j++;
		}
		square += diff * diff;
	}
	for( ; i < x1.Size; i++ ) {
		diff = x1.Values[i];
		square += diff * diff;
	}
	for( ; j < x2.Size; j++ ) {
		diff = x2.Values[j];
		square += diff * diff;
	}
	return exp(-gamma * square);
}

double CSvmKernel::rbfDenseByDense( const CFloatVectorDesc& x1, const CFloatVectorDesc& x2 ) const
{
	float square = 0;
	const int minSize = min( x1.Size, x2.Size );
	int i = 0;
#ifdef NEOML_USE_SSE
	__m128 sseSquare = _mm_setzero_ps();
	for( ; i + 4 <= minSize; i += 4 ) {
		__m128 sseDiff = _mm_sub_ps( _mm_loadu_ps( x1.Values + i ), _mm_loadu_ps( x2.Values + i ) );
		sseSquare = _mm_add_ps( sseSquare, _mm_mul_ps( sseDiff, sseDiff ) );
	}
	__m128 tmp = _mm_shuffle_ps(sseSquare, sseSquare, _MM_SHUFFLE(0, 3, 2, 1));
	sseSquare = _mm_add_ps(sseSquare, tmp);
	tmp = _mm_shuffle_ps(sseSquare, sseSquare, _MM_SHUFFLE(1, 0, 3, 2));
	sseSquare = _mm_add_ss(sseSquare, tmp);
	square += _mm_cvtss_f32(sseSquare);
	// The cycle below will add non-sse part
#endif
	float diff;
	for( ; i < minSize; ++i ) {
		diff = x1.Values[i] - x2.Values[i];
		square += diff * diff;
	}
	if( i < x1.Size ) {
		square += DenseDotProduct( x1.Values + i, x1.Values + i, x1.Size - i );
	}
	if( i < x2.Size ) {
		square += DenseDotProduct( x2.Values + i, x2.Values + i, x2.Size - i );
	}
	return exp(-gamma * square);
}

double CSvmKernel::rbfSparseBySparse( const CFloatVectorDesc& x1, const CFloatVectorDesc& x2 ) const
{
	double square = 0;
	double diff;
	int i, j;
	for( i = 0, j = 0; i < x1.Size && j < x2.Size; ) {
		if( x1.Indexes[i] == x2.Indexes[j] ) {
			diff = x1.Values[i] - x2.Values[j];
			i++;
			j++;
		} else if( x1.Indexes[i] < x2.Indexes[j] ) {
			diff = x1.Values[i];
			i++;
		} else {
			diff = x2.Values[j];
			j++;
		}
		square += diff * diff;
	}
	for( ; i < x1.Size; i++ ) {
		diff = x1.Values[i];
		square += diff * diff;
	}
	for( ; j < x2.Size; j++ ) {
		diff = x2.Values[j];
		square += diff * diff;
	}
	return exp(-gamma * square);
}

} // namespace NeoML
