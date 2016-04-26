//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dx11effect.h"
#include "d3dUtil.h"
#include "LightHelper.h"

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	//Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect( ID3D11Device* device, const std::wstring& filename );
	~BasicEffect();

	void SetWorldViewProj(DirectX::CXMMATRIX M)                  
    { 
        WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); 
    }

	void SetWorld( DirectX::CXMMATRIX M )                
    { 
        World->SetMatrix( reinterpret_cast<const float*>( &M ) ); 
    }

	void SetWorldInvTranspose( DirectX::CXMMATRIX M )              
    { 
        WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); 
    }

    void SetTexTransform( DirectX::CXMMATRIX M )
    {
        TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) );
    }

	void SetEyePosW( const DirectX::XMFLOAT3& v )                  
    { 
        EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3) ); 
    }

    void SetFogColor( const DirectX::FXMVECTOR v ) 
    { 
        FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); 
    }

    void SetFogStart( float f ) 
    { 
        FogStart->SetFloat( f ); 
    }

    void SetFogRange( float f ) 
    { 
        FogRange->SetFloat( f ); 
    }

	void SetDirLights( const DirectionalLight* lights )   
    {
        DirLights->SetRawValue( lights, 0, 3*sizeof( DirectionalLight ) );
    }

	void SetMaterial( const Material& mat )               
    { 
        Mat->SetRawValue( &mat, 0, sizeof( Material ) );
    }

    void SetDiffuseMap( ID3D11ShaderResourceView* tex )
    {
        DiffuseMap->SetResource( tex );
    }

    void SetCubeMap( ID3D11ShaderResourceView* tex )
    {
        CubeMap->SetResource( tex );
    }

    void SetShadowMap( ID3D11ShaderResourceView* tex ) { ShadowMap->SetResource( tex ); }
    void SetShadowTransform( DirectX::CXMMATRIX M ) { ShadowTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetSsaoMap( ID3D11ShaderResourceView* tex ) { SsaoMap->SetResource( tex ); }

    void SetWorldViewProjTex( DirectX::CXMMATRIX M ) { WorldViewProjTex->SetMatrix( reinterpret_cast<const float*>( &M ) ); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectTechnique* Light0TexTech;
    ID3DX11EffectTechnique* Light1TexTech;
    ID3DX11EffectTechnique* Light2TexTech;
    ID3DX11EffectTechnique* Light3TexTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipTech;

    ID3DX11EffectTechnique* Light1FogTech;
    ID3DX11EffectTechnique* Light2FogTech;
    ID3DX11EffectTechnique* Light3FogTech;

    ID3DX11EffectTechnique* Light0TexFogTech;
    ID3DX11EffectTechnique* Light1TexFogTech;
    ID3DX11EffectTechnique* Light2TexFogTech;
    ID3DX11EffectTechnique* Light3TexFogTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

    ID3DX11EffectTechnique* Light1ReflectTech;
    ID3DX11EffectTechnique* Light2ReflectTech;
    ID3DX11EffectTechnique* Light3ReflectTech;

    ID3DX11EffectTechnique* Light0TexReflectTech;
    ID3DX11EffectTechnique* Light1TexReflectTech;
    ID3DX11EffectTechnique* Light2TexReflectTech;
    ID3DX11EffectTechnique* Light3TexReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipReflectTech;

    ID3DX11EffectTechnique* Light1FogReflectTech;
    ID3DX11EffectTechnique* Light2FogReflectTech;
    ID3DX11EffectTechnique* Light3FogReflectTech;

    ID3DX11EffectTechnique* Light0TexFogReflectTech;
    ID3DX11EffectTechnique* Light1TexFogReflectTech;
    ID3DX11EffectTechnique* Light2TexFogReflectTech;
    ID3DX11EffectTechnique* Light3TexFogReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogReflectTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldViewProjTex;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
    ID3DX11EffectShaderResourceVariable* CubeMap;
    ID3DX11EffectShaderResourceVariable* ShadowMap;
    ID3DX11EffectShaderResourceVariable* SsaoMap;
};
#pragma endregion

#pragma region TreeSpriteEffect
class TreeSpriteEffect : public Effect {
public:
    TreeSpriteEffect( ID3D11Device* device, const std::wstring& filename );
    ~TreeSpriteEffect();

    void SetViewProj( DirectX::CXMMATRIX M ) { ViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }
    void SetTreeTextureMapArray( ID3D11ShaderResourceView* tex ) { TreeTextureMapArray->SetResource( tex ); }

    ID3DX11EffectTechnique* Light3Tech;
    ID3DX11EffectTechnique* Light3TexAlphaClipTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

    ID3DX11EffectMatrixVariable* ViewProj;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* TreeTextureMapArray;
};
#pragma endregion

#pragma region BlurEffect
class BlurEffect : public Effect {
public:
    BlurEffect( ID3D11Device* device, const std::wstring& filename );
    ~BlurEffect();

    void SetWeights( const float weights[9] ) { Weights->SetFloatArray( weights, 0, 9 ); }
    void SetInputMap( ID3D11ShaderResourceView* tex ) { InputMap->SetResource( tex ); }
    void SetOutputMap( ID3D11UnorderedAccessView* tex ) { OutputMap->SetUnorderedAccessView( tex ); }

    ID3DX11EffectTechnique* HorzBlurTech;
    ID3DX11EffectTechnique* VertBlurTech;

    ID3DX11EffectScalarVariable* Weights;
    ID3DX11EffectShaderResourceVariable* InputMap;
    ID3DX11EffectUnorderedAccessViewVariable* OutputMap;
};
#pragma endregion

#pragma region BezierTessellationEffect
class BezierTessellationEffect : public Effect {
public:
    BezierTessellationEffect( ID3D11Device* device, const std::wstring& filename );
    ~BezierTessellationEffect();

    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorld( DirectX::CXMMATRIX M ) { World->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTranspose( DirectX::CXMMATRIX M ) { WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }
    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }

    ID3DX11EffectTechnique* TessTech;

    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region InstancedBasicEffect
class InstancedBasicEffect : public Effect {
public:
    InstancedBasicEffect( ID3D11Device* device, const std::wstring& filename );
    ~InstancedBasicEffect();

    void SetViewProj( DirectX::CXMMATRIX M ) { ViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorld( DirectX::CXMMATRIX M ) { World->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTranspose( DirectX::CXMMATRIX M ) { WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }
    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }

    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectTechnique* Light0TexTech;
    ID3DX11EffectTechnique* Light1TexTech;
    ID3DX11EffectTechnique* Light2TexTech;
    ID3DX11EffectTechnique* Light3TexTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipTech;

    ID3DX11EffectTechnique* Light1FogTech;
    ID3DX11EffectTechnique* Light2FogTech;
    ID3DX11EffectTechnique* Light3FogTech;

    ID3DX11EffectTechnique* Light0TexFogTech;
    ID3DX11EffectTechnique* Light1TexFogTech;
    ID3DX11EffectTechnique* Light2TexFogTech;
    ID3DX11EffectTechnique* Light3TexFogTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

    ID3DX11EffectMatrixVariable* ViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region SkyEffect
class SkyEffect : public Effect {
public:
    SkyEffect( ID3D11Device* device, const std::wstring& filename );
    ~SkyEffect();

    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetCubeMap( ID3D11ShaderResourceView* cubemap ) { CubeMap->SetResource( cubemap ); }

    ID3DX11EffectTechnique* SkyTech;

    ID3DX11EffectMatrixVariable* WorldViewProj;

    ID3DX11EffectShaderResourceVariable* CubeMap;
};
#pragma endregion

#pragma region NormalMapEffect
class NormalMapEffect : public Effect {
public:
    NormalMapEffect( ID3D11Device* device, const std::wstring& filename );
    ~NormalMapEffect();

    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorld( DirectX::CXMMATRIX M ) { World->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTranspose( DirectX::CXMMATRIX M ) { WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetShadowTransform( DirectX::CXMMATRIX M ) { ShadowTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }
    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }
    void SetCubeMap( ID3D11ShaderResourceView* tex ) { CubeMap->SetResource( tex ); }
    void SetNormalMap( ID3D11ShaderResourceView* tex ) { NormalMap->SetResource( tex ); }
    void SetShadowMap( ID3D11ShaderResourceView* tex ) { ShadowMap->SetResource( tex ); }
    void SetSsaoMap( ID3D11ShaderResourceView* tex ) { SsaoMap->SetResource( tex ); }

    void SetWorldViewProjTex( DirectX::CXMMATRIX M ) { WorldViewProjTex->SetMatrix( reinterpret_cast<const float*>( &M ) ); }

    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectTechnique* Light0TexTech;
    ID3DX11EffectTechnique* Light1TexTech;
    ID3DX11EffectTechnique* Light2TexTech;
    ID3DX11EffectTechnique* Light3TexTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipTech;

    ID3DX11EffectTechnique* Light1FogTech;
    ID3DX11EffectTechnique* Light2FogTech;
    ID3DX11EffectTechnique* Light3FogTech;

    ID3DX11EffectTechnique* Light0TexFogTech;
    ID3DX11EffectTechnique* Light1TexFogTech;
    ID3DX11EffectTechnique* Light2TexFogTech;
    ID3DX11EffectTechnique* Light3TexFogTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

    ID3DX11EffectTechnique* Light1ReflectTech;
    ID3DX11EffectTechnique* Light2ReflectTech;
    ID3DX11EffectTechnique* Light3ReflectTech;

    ID3DX11EffectTechnique* Light0TexReflectTech;
    ID3DX11EffectTechnique* Light1TexReflectTech;
    ID3DX11EffectTechnique* Light2TexReflectTech;
    ID3DX11EffectTechnique* Light3TexReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipReflectTech;

    ID3DX11EffectTechnique* Light1FogReflectTech;
    ID3DX11EffectTechnique* Light2FogReflectTech;
    ID3DX11EffectTechnique* Light3FogReflectTech;

    ID3DX11EffectTechnique* Light0TexFogReflectTech;
    ID3DX11EffectTechnique* Light1TexFogReflectTech;
    ID3DX11EffectTechnique* Light2TexFogReflectTech;
    ID3DX11EffectTechnique* Light3TexFogReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogReflectTech;

    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectMatrixVariable* WorldViewProjTex;
    ID3DX11EffectMatrixVariable* ShadowTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
    ID3DX11EffectShaderResourceVariable* CubeMap;
    ID3DX11EffectShaderResourceVariable* NormalMap;
    ID3DX11EffectShaderResourceVariable* ShadowMap;
    ID3DX11EffectShaderResourceVariable* SsaoMap;
};
#pragma endregion

#pragma region DisplacementMapEffect
class DisplacementMapEffect : public Effect {
public:
    DisplacementMapEffect( ID3D11Device* device, const std::wstring& filename );
    ~DisplacementMapEffect();

    void SetViewProj( DirectX::CXMMATRIX M ) { ViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorld( DirectX::CXMMATRIX M ) { World->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTranspose( DirectX::CXMMATRIX M ) { WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetShadowTransform( DirectX::CXMMATRIX M ) { ShadowTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }
    void SetHeightScale( float f ) { HeightScale->SetFloat( f ); }
    void SetMaxTessDistance( float f ) { MaxTessDistance->SetFloat( f ); }
    void SetMinTessDistance( float f ) { MinTessDistance->SetFloat( f ); }
    void SetMinTessFactor( float f ) { MinTessFactor->SetFloat( f ); }
    void SetMaxTessFactor( float f ) { MaxTessFactor->SetFloat( f ); }

    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }
    void SetCubeMap( ID3D11ShaderResourceView* tex ) { CubeMap->SetResource( tex ); }
    void SetNormalMap( ID3D11ShaderResourceView* tex ) { NormalMap->SetResource( tex ); }
    void SetShadowMap( ID3D11ShaderResourceView* tex ) { ShadowMap->SetResource( tex ); }

    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectTechnique* Light0TexTech;
    ID3DX11EffectTechnique* Light1TexTech;
    ID3DX11EffectTechnique* Light2TexTech;
    ID3DX11EffectTechnique* Light3TexTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipTech;

    ID3DX11EffectTechnique* Light1FogTech;
    ID3DX11EffectTechnique* Light2FogTech;
    ID3DX11EffectTechnique* Light3FogTech;

    ID3DX11EffectTechnique* Light0TexFogTech;
    ID3DX11EffectTechnique* Light1TexFogTech;
    ID3DX11EffectTechnique* Light2TexFogTech;
    ID3DX11EffectTechnique* Light3TexFogTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

    ID3DX11EffectTechnique* Light1ReflectTech;
    ID3DX11EffectTechnique* Light2ReflectTech;
    ID3DX11EffectTechnique* Light3ReflectTech;

    ID3DX11EffectTechnique* Light0TexReflectTech;
    ID3DX11EffectTechnique* Light1TexReflectTech;
    ID3DX11EffectTechnique* Light2TexReflectTech;
    ID3DX11EffectTechnique* Light3TexReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipReflectTech;

    ID3DX11EffectTechnique* Light1FogReflectTech;
    ID3DX11EffectTechnique* Light2FogReflectTech;
    ID3DX11EffectTechnique* Light3FogReflectTech;

    ID3DX11EffectTechnique* Light0TexFogReflectTech;
    ID3DX11EffectTechnique* Light1TexFogReflectTech;
    ID3DX11EffectTechnique* Light2TexFogReflectTech;
    ID3DX11EffectTechnique* Light3TexFogReflectTech;

    ID3DX11EffectTechnique* Light0TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light1TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light2TexAlphaClipFogReflectTech;
    ID3DX11EffectTechnique* Light3TexAlphaClipFogReflectTech;

    ID3DX11EffectMatrixVariable* ViewProj;
    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectMatrixVariable* ShadowTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;
    ID3DX11EffectScalarVariable* HeightScale;
    ID3DX11EffectScalarVariable* MaxTessDistance;
    ID3DX11EffectScalarVariable* MinTessDistance;
    ID3DX11EffectScalarVariable* MinTessFactor;
    ID3DX11EffectScalarVariable* MaxTessFactor;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
    ID3DX11EffectShaderResourceVariable* CubeMap;
    ID3DX11EffectShaderResourceVariable* NormalMap;
    ID3DX11EffectShaderResourceVariable* ShadowMap;
};
#pragma endregion

#pragma region TerrainEffect
class TerrainEffect : public Effect {
public:
    TerrainEffect( ID3D11Device* device, const std::wstring& filename );
    ~TerrainEffect();

    void SetViewProj( DirectX::CXMMATRIX M ) { ViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }
    void SetFogColor( const DirectX::FXMVECTOR v ) { FogColor->SetFloatVector( reinterpret_cast<const float*>( &v ) ); }
    void SetFogStart( float f ) { FogStart->SetFloat( f ); }
    void SetFogRange( float f ) { FogRange->SetFloat( f ); }
    void SetDirLights( const DirectionalLight* lights ) { DirLights->SetRawValue( lights, 0, 3 * sizeof( DirectionalLight ) ); }
    void SetMaterial( const Material& mat ) { Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }

    void SetMinDist( float f ) { MinDist->SetFloat( f ); }
    void SetMaxDist( float f ) { MaxDist->SetFloat( f ); }
    void SetMinTess( float f ) { MinTess->SetFloat( f ); }
    void SetMaxTess( float f ) { MaxTess->SetFloat( f ); }
    void SetTexelCellSpaceU( float f ) { TexelCellSpaceU->SetFloat( f ); }
    void SetTexelCellSpaceV( float f ) { TexelCellSpaceV->SetFloat( f ); }
    void SetWorldCellSpace( float f ) { WorldCellSpace->SetFloat( f ); }
    void SetWorldFrustumPlanes( DirectX::XMFLOAT4 planes[6] ) { WorldFrustumPlanes->SetFloatVectorArray( reinterpret_cast<float*>( planes ), 0, 6 ); }

    void SetLayerMapArray( ID3D11ShaderResourceView* tex ) { LayerMapArray->SetResource( tex ); }
    void SetBlendMap( ID3D11ShaderResourceView* tex ) { BlendMap->SetResource( tex ); }
    void SetHeightMap( ID3D11ShaderResourceView* tex ) { HeightMap->SetResource( tex ); }


    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;
    ID3DX11EffectTechnique* Light1FogTech;
    ID3DX11EffectTechnique* Light2FogTech;
    ID3DX11EffectTechnique* Light3FogTech;

    ID3DX11EffectMatrixVariable* ViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVectorVariable* FogColor;
    ID3DX11EffectScalarVariable* FogStart;
    ID3DX11EffectScalarVariable* FogRange;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;
    ID3DX11EffectScalarVariable* MinDist;
    ID3DX11EffectScalarVariable* MaxDist;
    ID3DX11EffectScalarVariable* MinTess;
    ID3DX11EffectScalarVariable* MaxTess;
    ID3DX11EffectScalarVariable* TexelCellSpaceU;
    ID3DX11EffectScalarVariable* TexelCellSpaceV;
    ID3DX11EffectScalarVariable* WorldCellSpace;
    ID3DX11EffectVectorVariable* WorldFrustumPlanes;

    ID3DX11EffectShaderResourceVariable* LayerMapArray;
    ID3DX11EffectShaderResourceVariable* BlendMap;
    ID3DX11EffectShaderResourceVariable* HeightMap;
};
#pragma endregion

#pragma region BuildShadowMapEffect
class BuildShadowMapEffect : public Effect {
public:
    BuildShadowMapEffect( ID3D11Device* device, const std::wstring& filename );
    ~BuildShadowMapEffect();

    void SetViewProj( DirectX::CXMMATRIX M ) { ViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorld( DirectX::CXMMATRIX M ) { World->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTranspose( DirectX::CXMMATRIX M ) { WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetEyePosW( const DirectX::XMFLOAT3& v ) { EyePosW->SetRawValue( &v, 0, sizeof( DirectX::XMFLOAT3 ) ); }

    void SetHeightScale( float f ) { HeightScale->SetFloat( f ); }
    void SetMaxTessDistance( float f ) { MaxTessDistance->SetFloat( f ); }
    void SetMinTessDistance( float f ) { MinTessDistance->SetFloat( f ); }
    void SetMinTessFactor( float f ) { MinTessFactor->SetFloat( f ); }
    void SetMaxTessFactor( float f ) { MaxTessFactor->SetFloat( f ); }

    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }
    void SetNormalMap( ID3D11ShaderResourceView* tex ) { NormalMap->SetResource( tex ); }

    ID3DX11EffectTechnique* BuildShadowMapTech;
    ID3DX11EffectTechnique* BuildShadowMapAlphaClipTech;
    ID3DX11EffectTechnique* TessBuildShadowMapTech;
    ID3DX11EffectTechnique* TessBuildShadowMapAlphaClipTech;

    ID3DX11EffectMatrixVariable* ViewProj;
    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectScalarVariable* HeightScale;
    ID3DX11EffectScalarVariable* MaxTessDistance;
    ID3DX11EffectScalarVariable* MinTessDistance;
    ID3DX11EffectScalarVariable* MinTessFactor;
    ID3DX11EffectScalarVariable* MaxTessFactor;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
    ID3DX11EffectShaderResourceVariable* NormalMap;
};
#pragma endregion

#pragma region DebugTexEffect
class DebugTexEffect : public Effect {
public:
    DebugTexEffect( ID3D11Device* device, const std::wstring& filename );
    ~DebugTexEffect();

    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexture( ID3D11ShaderResourceView* tex ) { Texture->SetResource( tex ); }

    ID3DX11EffectTechnique* ViewArgbTech;
    ID3DX11EffectTechnique* ViewRedTech;
    ID3DX11EffectTechnique* ViewGreenTech;
    ID3DX11EffectTechnique* ViewBlueTech;
    ID3DX11EffectTechnique* ViewAlphaTech;

    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectShaderResourceVariable* Texture;
};
#pragma endregion

#pragma region SsaoNormalDepthEffect
class SsaoNormalDepthEffect : public Effect {
public:
    SsaoNormalDepthEffect( ID3D11Device* device, const std::wstring& filename );
    ~SsaoNormalDepthEffect();

    void SetWorldView( DirectX::CXMMATRIX M ) { WorldView->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldInvTransposeView( DirectX::CXMMATRIX M ) { WorldInvTransposeView->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetWorldViewProj( DirectX::CXMMATRIX M ) { WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetTexTransform( DirectX::CXMMATRIX M ) { TexTransform->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetDiffuseMap( ID3D11ShaderResourceView* tex ) { DiffuseMap->SetResource( tex ); }

    ID3DX11EffectTechnique* NormalDepthTech;
    ID3DX11EffectTechnique* NormalDepthAlphaClipTech;

    ID3DX11EffectMatrixVariable* WorldView;
    ID3DX11EffectMatrixVariable* WorldInvTransposeView;
    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* TexTransform;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region SsaoEffect
class SsaoEffect : public Effect {
public:
    SsaoEffect( ID3D11Device* device, const std::wstring& filename );
    ~SsaoEffect();

    void SetViewToTexSpace( DirectX::CXMMATRIX M ) { ViewToTexSpace->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
    void SetOffsetVectors( const DirectX::XMFLOAT4 v[14] ) { OffsetVectors->SetFloatVectorArray( reinterpret_cast<const float*>( v ), 0, 14 ); }
    void SetFrustumCorners( const DirectX::XMFLOAT4 v[4] ) { FrustumCorners->SetFloatVectorArray( reinterpret_cast<const float*>( v ), 0, 4 ); }
    void SetOcclusionRadius( float f ) { OcclusionRadius->SetFloat( f ); }
    void SetOcclusionFadeStart( float f ) { OcclusionFadeStart->SetFloat( f ); }
    void SetOcclusionFadeEnd( float f ) { OcclusionFadeEnd->SetFloat( f ); }
    void SetSurfaceEpsilon( float f ) { SurfaceEpsilon->SetFloat( f ); }

    void SetNormalDepthMap( ID3D11ShaderResourceView* srv ) { NormalDepthMap->SetResource( srv ); }
    void SetRandomVecMap( ID3D11ShaderResourceView* srv ) { RandomVecMap->SetResource( srv ); }

    ID3DX11EffectTechnique* SsaoTech;

    ID3DX11EffectMatrixVariable* ViewToTexSpace;
    ID3DX11EffectVectorVariable* OffsetVectors;
    ID3DX11EffectVectorVariable* FrustumCorners;
    ID3DX11EffectScalarVariable* OcclusionRadius;
    ID3DX11EffectScalarVariable* OcclusionFadeStart;
    ID3DX11EffectScalarVariable* OcclusionFadeEnd;
    ID3DX11EffectScalarVariable* SurfaceEpsilon;

    ID3DX11EffectShaderResourceVariable* NormalDepthMap;
    ID3DX11EffectShaderResourceVariable* RandomVecMap;
};
#pragma endregion

#pragma region SsaoBlurEffect
class SsaoBlurEffect : public Effect {
public:
    SsaoBlurEffect( ID3D11Device* device, const std::wstring& filename );
    ~SsaoBlurEffect();

    void SetTexelWidth( float f ) { TexelWidth->SetFloat( f ); }
    void SetTexelHeight( float f ) { TexelHeight->SetFloat( f ); }

    void SetNormalDepthMap( ID3D11ShaderResourceView* srv ) { NormalDepthMap->SetResource( srv ); }
    void SetInputImage( ID3D11ShaderResourceView* srv ) { InputImage->SetResource( srv ); }

    ID3DX11EffectTechnique* HorzBlurTech;
    ID3DX11EffectTechnique* VertBlurTech;

    ID3DX11EffectScalarVariable* TexelWidth;
    ID3DX11EffectScalarVariable* TexelHeight;

    ID3DX11EffectShaderResourceVariable* NormalDepthMap;
    ID3DX11EffectShaderResourceVariable* InputImage;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
    static TreeSpriteEffect* TreeSpriteFX;
    static BlurEffect* BlurFX;
    static BezierTessellationEffect* BezierTessellationFX;
    static InstancedBasicEffect* InstancedBasicFX;
    static SkyEffect* SkyFX;
    static NormalMapEffect* NormalMapFX;
    static DisplacementMapEffect* DisplacementMapFX;
    static BuildShadowMapEffect* BuildShadowMapFX;
    static DebugTexEffect* DebugTexFX;
    static TerrainEffect* TerrainFX;
    static SsaoNormalDepthEffect* SsaoNormalDepthFX;
    static SsaoEffect* SsaoFX;
    static SsaoBlurEffect* SsaoBlurFX;
};
#pragma endregion

#endif // EFFECTS_H