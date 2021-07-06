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

#include "../common.h"
#pragma hdrstop

#include "onnx.pb.h"

#include "EltwiseOperator.h"
#include "NeoOnnxCheck.h"
#include "TensorUtils.h"

namespace NeoOnnx {

CEltwiseOperatorBase::CEltwiseOperatorBase( const onnx::NodeProto& eltwise, int opsetVersion,
		TOperation _operation, int _argsNum ) :
	CLayerOperator( eltwise, opsetVersion ),
	operation( _operation ),
	argsNum( _argsNum )
{
	if( argsNum > 0 ) {
		CheckOnnxProtocol( InputCount() == argsNum, "expected " + Str( argsNum ) + " arguments", *this );
	}
	CheckOnnxProtocol( OutputCount() == 1, "operator must have 1 output", *this );
}

void CEltwiseOperatorBase::AddLayers( const CBroadcast& broadcast, const CTensorArray& inputs,
	CDnn& dnn, CTensorArray& outputs ) const
{
	// Corner case which doesn't violate Onnx protocol: opeartors with variable input count may have 1 input
	if( inputs.Size() == 1 && argsNum < 0 ) {
		outputs[0] = inputs[0];
		return;
	}

	// Calculate outputShape
	CTensorShape outputShape;
	inputs[0]->Shape().CopyTo( outputShape );
	for( int i = 1; i < inputs.Size(); ++i ) {
		CTensorShape buff;
		CheckNeoOnnxSupport( BroadcastTensorShape( outputShape, inputs[i]->Shape(), broadcast, buff ),
			"Can't broadcast tensors shape", *this );
		buff.CopyTo( outputShape );
	}

	CTensorArray currInputs;
	inputs.CopyTo( currInputs );
	if( argsNum == 2 ) {
		// Preparing values in case of subtraction or division
		currInputs[1] = prepareSecondInput( currInputs );
	}
	// Broadcast input to the final shape and set proper layout
	for( int i = 0; i < inputs.Size(); ++i ) {
		currInputs[i] = BroadcastTensor( *currInputs[i], broadcast, outputShape );
		currInputs[i] = ConvertTensor( *currInputs[i], currInputs[0]->Layout() );
	}

	// Put pre-calculated blobs to the source in the net
	for( int i = 0; i < currInputs.Size(); ++i ) {
		if( currInputs[i]->IsCalculated() ) {
			CPtr<CSourceLayer> source = new CSourceLayer( dnn.GetMathEngine() );
			source->SetName( Name() + "_input_" + Str( i ) );
			source->SetBlob( dynamic_cast<const CDataTensor*>( currInputs[i].Ptr() )->Data()->GetCopy() );
			// Save this pre-calculated data during serialization
			// Otherwise dnn won't be able to work correctly after serialization
			source->StoreBlob( true );
			dnn.AddLayer( *source );
			currInputs[i] = new CUserTensor( currInputs[i]->Shape(), currInputs[i]->Layout(), CLayerOutput( source, 0 ) );
		}
	}

	static_assert( O_Count == 4, "O_Count != 4" );
	CPtr<CBaseLayer> eltwise = nullptr;
	if( operation == O_Add || operation == O_Sub ) {
		eltwise = new CEltwiseSumLayer( dnn.GetMathEngine() );
	} else {
		eltwise = new CEltwiseMulLayer( dnn.GetMathEngine() );
	}
	eltwise->SetName( Name() );
	for( int i = 0; i < currInputs.Size(); ++i ) {
		NeoAssert( !currInputs[i]->IsCalculated() );
		const CUserTensor* userInput = dynamic_cast<const CUserTensor*>( currInputs[i].Ptr() );
		eltwise->Connect( i, *userInput->Layer(), userInput->OutputIndex() );
	}
	dnn.AddLayer( *eltwise );
	outputs.Add( new CUserTensor( outputShape, currInputs[0]->Layout(), CLayerOutput( eltwise, 0 ) ) );
}

// Modifies second input for binary division or subtraction
CPtr<const CTensorBase> CEltwiseOperatorBase::prepareSecondInput( const CTensorArray& inputs ) const
{
	static_assert( O_Count == 4, "O_Count != 4" );
	NeoAssert( inputs.Size() == 2 );
	if( operation == O_Mul || operation == O_Add ) {
		// Add and Mul already have corresponding layers 
		return inputs[1];
	}

	if( operation == O_Sub ) {
		// a - b = a + (-1 * b)
		if( inputs[1]->IsCalculated() ) {
			// Imitate by multiplying values in blob
			CPtr<const CDataTensor> secondInput = dynamic_cast<const CDataTensor*>( inputs[1].Ptr() );
			CPtr<CDnnBlob> newBlob = secondInput->Data()->GetClone();
			CFloatHandleStackVar minusOne( newBlob->GetMathEngine() );
			minusOne.SetValue( -1 );
			newBlob->GetMathEngine().VectorMultiply( secondInput->Data()->GetData(), newBlob->GetData(),
				newBlob->GetDataSize(), minusOne );
			return new CDataTensor( secondInput->Shape(), secondInput->Layout(), *newBlob );
		} else {
			// Imitate by CLinearLayer with multiplier set to -1
			CPtr<const CUserTensor> secondInput = dynamic_cast<const CUserTensor*>( inputs[1].Ptr() );
			CDnn& dnn = *secondInput->Layer()->GetDnn();
			CPtr<CLinearLayer> linear = new CLinearLayer( dnn.GetMathEngine() );
			linear->SetName( Name() + "_neg" );
			linear->SetMultiplier( -1 );
			linear->Connect( 0, *secondInput->Layer(), secondInput->OutputIndex() );
			return new CUserTensor( secondInput->Shape(), secondInput->Layout(), CLayerOutput( linear, 0 ) );
		}
	}

	// Operation is O_Div
	// a / b = a * (1 / b)
	// In that case it's impossible to imitate (1 / x) operation via layer
	// That's why only CDataTensor is supported
	CheckNeoOnnxSupport( inputs[1]->IsCalculated(), "Div supports only data tensor as a second input", *this );
	CPtr<const CDataTensor> secondInput = dynamic_cast<const CDataTensor*>( inputs[1].Ptr() );
	CPtr<CDnnBlob> newBlob = secondInput->Data()->GetClone();
	newBlob->GetMathEngine().VectorInv( secondInput->Data()->GetData(), newBlob->GetData(), newBlob->GetDataSize() );
	return new CDataTensor( secondInput->Shape(), secondInput->Layout(), *newBlob );
}

// --------------------------------------------------------------------------------------------------------------------

void CEltwiseBinaryOperatorBase::AddLayers( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const
{
	CBroadcast broadcast( BT_Numpy, NotFound );
	if( OpsetVersion < 7 ) {
		int broadcastAttr = 0;
		GetAttribute( "broadcast", broadcastAttr );
		if( broadcastAttr != 0 ) {
			broadcast.Type = BT_Onnx;
			GetAttribute( "axis", broadcast.Axis );
		} else {
			broadcast.Type = BT_None;
		}
	}

	CEltwiseOperatorBase::AddLayers( broadcast, inputs, dnn, outputs );
}

// --------------------------------------------------------------------------------------------------------------------

void CSumOperator::AddLayers( const CTensorArray& inputs, CDnn& dnn, CTensorArray& outputs ) const
{
	CBroadcast broadcast( BT_Numpy );
	if( OpsetVersion < 8 ) {
		broadcast.Type = BT_None;
	}

	CEltwiseOperatorBase::AddLayers( broadcast, inputs, dnn, outputs );
}

} // namespace NeoOnnx

