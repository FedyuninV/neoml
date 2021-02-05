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

#include <common.h>
#pragma hdrstop

#include <NeoMathEngine/NeoMathEngineException.h>
#include <MathEngineDnnLrn.h>
#include <CpuMathEngine.h>

namespace NeoML {

CLrnDesc* CCpuMathEngine::InitLrn( const CBlobDesc& source, int windowSize, float bias, float alpha, float beta )
{
	return new CMathEngineLrnDesc( source, windowSize, bias, alpha, beta );
	return nullptr;
}

void CCpuMathEngine::Lrn( const CLrnDesc& lrnDesc, const CConstFloatHandle& input, const CFloatHandle& invSum,
	const CFloatHandle& invSumBeta, const CFloatHandle& outputHandle )
{
	const CMathEngineLrnDesc& desc = static_cast<const CMathEngineLrnDesc&>( lrnDesc );
}

void CCpuMathEngine::LrnBackward( const CLrnDesc& /* desc */, const CConstFloatHandle& /* input */, const CConstFloatHandle& /* output */,
		const CConstFloatHandle& /* outputDiff */, const CConstFloatHandle& /* invSum */, const CConstFloatHandle& /* invSumBeta */,
		const CFloatHandle& /* inputDiff */ )
{
	ASSERT_EXPR( false );
}

} // namespace NeoML
