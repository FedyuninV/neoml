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
	CEltwiseOperatorBase( const onnx::NodeProto& eltwise, int opsetVersion, TOperation operation, int argsNum = NotFound );

	// CLayerOperator methods
	void AddLayers( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const override;
	// We can guarantee the support only when second input is CDataTensor (other division is impossible)
	void UserInputMask( CUserInputMask& mask ) const override { mask |= 0; }

	// In some versions different operators supported different broadcast types
	// E.g. 'Add' operators in opset v1 supports onnx-broadcast but 'Sum' operators doesn't support broadcast at all
	// That's why each derivative should determine by itself which broadcast type is supported
	virtual CBroadcast GetBroadcast() const = 0;

private:
	// Operation performed by this operator
	TOperation operation;
	// Expected number of arguments (-1 if any number is supported)
	int argsNum;

	CPtr<const CTensorBase> prepareSecondInput( const CTensorArray& inputs ) const;
};

// Eltwise operators with 2 inputs

// Base class
class CEltwiseBinaryOperatorBase : public CEltwiseOperatorBase {
public:
	CEltwiseBinaryOperatorBase( const onnx::NodeProto& eltwise, int opsetVersion, TOperation operation ) :
		CEltwiseOperatorBase( eltwise, opsetVersion, operation, 2 ) {}

protected:
	// CEltwiseOperatorBase methods
	CBroadcast GetBroadcast() const override;
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

// Eltwise operators with any number of inputs

// Sum operator
class CSumOperator : public CEltwiseOperatorBase {
public:
	CSumOperator( const onnx::NodeProto& sum, int opsetVersion ) : CEltwiseOperatorBase( sum, opsetVersion, O_Add ) {}

protected:
	// CEltwiseOperatorBase methods
	CBroadcast GetBroadcast() const override;
};

} // namespace NeoOnnx