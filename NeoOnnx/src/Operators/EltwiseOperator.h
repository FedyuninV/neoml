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

#pragma once

#include "../LayerOperator.h"

#include "TensorUtils.h"

namespace NeoOnnx {

// Base class for operators which perform eltwise operations
class CEltwiseOperatorBase : public CLayerOperator {
public:
	// Supported eltwise operations
	enum TOperation {
		// Addition
		O_Add,
		// Subtraction
		O_Sub,
		// Multiplication
		O_Mul,
		// Division
		O_Div,

		O_Count
	};

protected:
	CEltwiseOperatorBase( const onnx::NodeProto& eltwise, int opsetVersion, TOperation operation, int argsNum = -1 );

	// AddLayers implementation for the given broadcast
	// The derivatives should call this method from their AddLayers
	void AddLayers( const CBroadcast& broadcast, const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const;

private:
	// Operation performed by this operator
	const TOperation operation;
	// Expected number of arguments (-1 if any number is supported)
	const int argsNum;

	CPtr<const CTensorBase> prepareSecondInput( const CTensorArray& inputs ) const;
};

//---------------------------------------------------------------------------------------------------------------------

// Eltwise operators with 2 inputs

// Base class
class CEltwiseBinaryOperatorBase : public CEltwiseOperatorBase {
public:
	CEltwiseBinaryOperatorBase( const onnx::NodeProto& eltwise, int opsetVersion, TOperation operation ) :
		CEltwiseOperatorBase( eltwise, opsetVersion, operation, 2 ) {}

	// CLayerOperator methods
	void AddLayers( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const override;
};

// Add operator
class CAddOperator : public CEltwiseBinaryOperatorBase {
public:
	CAddOperator( const onnx::NodeProto& add, int opsetVersion ) : CEltwiseBinaryOperatorBase( add, opsetVersion, O_Add ) {}
};

// Sub operator
class CSubOperator : public CEltwiseBinaryOperatorBase {
public:
	CSubOperator( const onnx::NodeProto& sub, int opsetVersion ) : CEltwiseBinaryOperatorBase( sub, opsetVersion, O_Sub ) {}
};

// Mul operator
class CMulOperator : public CEltwiseBinaryOperatorBase {
public:
	CMulOperator( const onnx::NodeProto& mul, int opsetVersion ) : CEltwiseBinaryOperatorBase( mul, opsetVersion, O_Mul ) {}
};

// Div operator
class CDivOperator : public CEltwiseBinaryOperatorBase {
public:
	CDivOperator( const onnx::NodeProto& div, int opsetVersion ) : CEltwiseBinaryOperatorBase( div, opsetVersion, O_Div ) {}
};

//---------------------------------------------------------------------------------------------------------------------

// Eltwise operators with any number of inputs

// Sum operator
class CSumOperator : public CEltwiseOperatorBase {
public:
	CSumOperator( const onnx::NodeProto& sum, int opsetVersion ) : CEltwiseOperatorBase( sum, opsetVersion, O_Add ) {}

protected:
	// CLayerOperator methods
	void AddLayers( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const override;
};

} // namespace NeoOnnx

