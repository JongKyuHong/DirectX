#include "bitmapclass.h"


BitmapClass::BitmapClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
}


BitmapClass::BitmapClass(const BitmapClass& other)
{
}


BitmapClass::~BitmapClass()
{
}


bool BitmapClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight,
							 char* textureFilename, int renderX, int renderY)
{
	bool result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_renderX = renderX;
	m_renderY = renderY;

	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	result = LoadTexture(device, deviceContext, textureFilename);
	if(!result)
	{
		ShutdownBuffers();
		return false;
	}

	return true;
}


void BitmapClass::Shutdown()
{
	ReleaseTexture();
	ShutdownBuffers();
	return;
}


bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
	bool result;

	result = UpdateBuffers(deviceContext);
	if(!result)
	{
		return false;
	}

	RenderBuffers(deviceContext);
	return true;
}


int BitmapClass::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
	return m_Texture->GetTexture();
}


bool BitmapClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	m_prevPosX = -1;
	m_prevPosY = -1;
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	vertices = new VertexType[m_vertexCount];
	indices = new unsigned long[m_indexCount];

	memset(vertices, 0, sizeof(VertexType) * m_vertexCount);

	for(i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		delete [] vertices;
		delete [] indices;
		return false;
	}

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
		delete [] vertices;
		delete [] indices;
		return false;
	}

	delete [] vertices;
	delete [] indices;
	return true;
}


void BitmapClass::ShutdownBuffers()
{
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	float left, right, top, bottom;
	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* dataPtr;
	HRESULT result;

	// if((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
	// {
	// 	return true;
	// }
	if (!m_verticesDirty && m_prevPosX == m_renderX && m_prevPosY == m_renderY)
	{
		return true;
	}

	m_prevPosX = m_renderX;
	m_prevPosY = m_renderY;
	vertices = new VertexType[m_vertexCount];

	left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;
	right = left + (float)m_bitmapWidth;
	top = (float)(m_screenHeight / 2) - (float)m_renderY;
	bottom = top - (float)m_bitmapHeight;

	vertices[0].position = XMFLOAT3(left, top, 0.0f);
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);
	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);
	vertices[3].position = XMFLOAT3(left, top, 0.0f);
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[4].position = XMFLOAT3(right, top, 0.0f);
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);
	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		delete [] vertices;
		return false;
	}

	dataPtr = (VertexType*)mappedResource.pData;
	memcpy(dataPtr, vertices, sizeof(VertexType) * m_vertexCount);
	deviceContext->Unmap(m_vertexBuffer, 0);
	m_verticesDirty = false;

	delete [] vertices;
	return true;
}


void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return;
}


bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;

	m_Texture = new TextureClass;

	result = m_Texture->Initialize(device, deviceContext, filename);
	if(!result)
	{
		delete m_Texture;
		m_Texture = 0;
		return false;
	}

	m_bitmapWidth = m_Texture->GetWidth();
	m_bitmapHeight = m_Texture->GetHeight();
	return true;
}


void BitmapClass::ReleaseTexture()
{
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}

void BitmapClass::SetRenderSize(int width, int height)
{
	m_bitmapWidth = width;
	m_bitmapHeight = height;
	m_verticesDirty = true;
}


void BitmapClass::SetRenderLocation(int x, int y)
{
	m_renderX = x;
	m_renderY = y;
	return;
}
