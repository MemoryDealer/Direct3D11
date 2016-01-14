//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************

#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

#include "d3dUtil.h"

class GeometryGenerator
{
public:
	struct Vertex
	{
		Vertex(){}
		Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
			: position(p), normal(n), tangentU(t), texC(uv){}
		Vertex(
			float px, float py, float pz, 
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: position(px,py,pz), normal(nx,ny,nz),
			  tangentU(tx, ty, tz), texC(u,v){}

        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT3 tangentU;
        DirectX::XMFLOAT2 texC;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};

	///<summary>
	/// Creates a box centered at the origin with the given dimensions.
	///</summary>
	void createBox(float width, float height, float depth, MeshData& meshData);

	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	void createSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	void createGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	void createCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	void createGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);

	///<summary>
	/// Creates a quad covering the screen in NDC coordinates.  This is useful for
	/// postprocessing effects.
	///</summary>
	void createFullscreenQuad(MeshData& meshData);

private:
	void subdivide(MeshData& meshData);
	void buildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void buildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
};

#endif // GEOMETRYGENERATOR_H