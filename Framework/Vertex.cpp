#include "Vertex.h"
#include "Effects.h"

using namespace DirectX;

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Pos[1] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::TreePointSprite[2] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::InstancedBasic32[8] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Basic32 = nullptr;
ID3D11InputLayout* InputLayouts::Pos = nullptr;
ID3D11InputLayout* InputLayouts::TreePointSprite = nullptr;
ID3D11InputLayout* InputLayouts::InstancedBasic32 = nullptr;

void InputLayouts::InitAll(ID3D11Device* device)
{
	//
	// Basic32
	//

	D3DX11_PASS_DESC passDesc;
	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &Basic32));

    //
    // Pos
    //

    Effects::SkyFX->SkyTech->GetPassByIndex( 0 )->GetDesc( &passDesc );
    HR( device->CreateInputLayout( InputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
                                   passDesc.IAInputSignatureSize, &Pos ) );

   /* Effects::BezierTessellationFX->TessTech->GetPassByIndex( 0 )->GetDesc( &passDesc );
    HR( device->CreateInputLayout( InputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
                                   passDesc.IAInputSignatureSize, &Pos ) );*/

    //
    // TreePointSprite

    /*Effects::TreeSpriteFX->Light3Tech->GetPassByIndex( 0 )->GetDesc( &passDesc );
    HR( device->CreateInputLayout( InputLayoutDesc::TreePointSprite, 2, passDesc.pIAInputSignature,
                                   passDesc.IAInputSignatureSize, &TreePointSprite ) );*/

    /*Effects::InstancedBasicFX->Light1Tech->GetPassByIndex( 0 )->GetDesc( &passDesc );
    HR( device->CreateInputLayout( InputLayoutDesc::InstancedBasic32, 8, passDesc.pIAInputSignature,
                                   passDesc.IAInputSignatureSize, &InstancedBasic32 ) );*/
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM( Basic32 );
    ReleaseCOM( Pos );
    ReleaseCOM( TreePointSprite );
    ReleaseCOM( InstancedBasic32 );
}

#pragma endregion
