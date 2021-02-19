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

#include "UnsqueezeNode.h"
#include "NeoOnnxCheck.h"

#include "onnx.pb.h"

namespace NeoOnnx {

CUnsqueezeNode::CUnsqueezeNode( const onnx::NodeProto& unsqueeze, int opsetVersion ) :
	COpNode( unsqueeze, opsetVersion )
{
	// v1 - original
	// v11 - supported negative axes values
	CheckNeoOnnxSupport( OpsetVersion >= 1 && OpsetVersion <= MaxOpsetVersion, "opset version", unsqueeze);

	CheckOnnxProtocol( InputCount() == 1, "node must have 1 input", unsqueeze );
	CheckOnnxProtocol( OutputCount() == 1, "node must have 1 output", unsqueeze );
}

void CUnsqueezeNode::AddLayers( const CObjectArray<const CTensorBase>& inputs,
	CObjectArray<const CTensorBase>& outputs, CDnn& dnn )
{
	CheckNeoOnnxInternal( inputs[0] != nullptr && !inputs[0]->IsCalculated(), "user-provided input is expected", OnnxNode );

	CFastArray<int, 8> axes;
	getAxes( inputs[0]->Shape(), axes );

	CTensorShape outputShape;
	calcOutputShape( inputs[0]->Shape(), axes, outputShape );

	CDimOrder outputDimOrder;
	calcOutputDimOrder( inputs[0]->Layout().OnnxOrder, axes, outputDimOrder );

	outputs[0] = new CUserTensor( outputShape, CTensorLayout( outputDimOrder ),
		dynamic_cast<const CUserTensor*>( inputs[0].Ptr() )->LayerOutput() );
}

// Fills array with axes indices to be squeezed
// Returns array of positive indices in sorted order
void CUnsqueezeNode::getAxes( const CTensorShape& inputShape, CFastArray<int, 8>& axes ) const
{
	axes.Empty();
	Attributes.GetOptionalIntArray( "axes", axes );
	for( int i = 0; i < axes.Size(); ++i ) {
		if( axes[i] < 0 ) {
			CheckOnnxProtocol( OpsetVersion >= 11, "negative axes indices are supported since v11", OnnxNode );
			axes[i] += inputShape.Size();
		}
	}
	axes.QuickSort<Ascending<int>>();
}

// Calculates output tensor's shape
void CUnsqueezeNode::calcOutputShape( const CTensorShape& inputShape, const CFastArray<int, 8>& axes, CTensorShape& outputShape ) const
{
	outputShape.Empty();
	outputShape.SetBufferSize( inputShape.Size() + axes.Size() );

	int axeIndex = 0;
	int inputDimIndex = 0;
	outputShape.SetBufferSize( axes.Size() + inputShape.Size() );

	for( int i = 0; i < axes.Size() + inputShape.Size(); ++i ) {
		if( axeIndex < axes.Size() && i == axes[axeIndex] ) {
			outputShape.Add( 1 );
		} else {
			CheckNeoOnnxInternal( inputDimIndex < inputShape.Size(), "Wrong dimensions number", OnnxNode );
			outputShape.Add( inputShape[inputDimIndex] );
			++inputDimIndex;
		}
	}
}

// Calculates output tensor's dim order
void CUnsqueezeNode::calcOutputDimOrder( const CDimOrder& inputDimOrder, const CFastArray<int, 8>& axes, CDimOrder& outputDimOrder ) const
{
	if( inputDimOrder.IsEmpty() ) {
		// Original layout is Onnx
		// No need in conversion
		outputDimOrder.Empty();
		return;
	}

	// NeoML layout
	TBlobDim currDim = BD_BatchLength;
	int axeIndex = 0;
	int inputDimIndex = 0;
	outputDimOrder.SetBufferSize( axes.Size() + inputDimOrder.Size() );

	// Distribute unused blob dimensions among new axes
	for( int i = 0; i < axes.Size() + inputDimOrder.Size(); ++i ) {
		if( axeIndex < axes.Size() && i == axes[axeIndex] ) {
			// Looking for unused blob dim
			while( currDim < BD_Count && inputDimOrder.Find( currDim ) != NotFound ) {
				++currDim;
			}
			CheckNeoOnnxInternal( currDim != BD_Count, "Wrong dimensions number", OnnxNode );
			outputDimOrder.Add( currDim );
			++axeIndex;
		} else {
			CheckNeoOnnxInternal( inputDimIndex < inputDimOrder.Size(), "Wrong dimensions number", OnnxNode );
			outputDimOrder.Add( inputDimOrder[inputDimIndex] );
			++inputDimIndex;
		}
	}
}

} // namespace NeoOnnx
