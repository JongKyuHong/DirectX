#include "spriteclass.h"

SpriteClass::SpriteClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Textures = 0;
}

SpriteClass::SpriteClass(const SpriteClass&)
{
}

SpriteClass::~SpriteClass()
{
}

bool SpriteClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, char* spriteFilename, int renderX, int renderY)
{
    bool result;
    
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    
    m_renderX = renderX;
    m_renderY = renderY;
    
    m_frameTime = 0;
    
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }
    
    result = LoadTextures(device, deviceContext, spriteFilename);
    if (!result)
    {
        return false;
    }
    return true;
}

void SpriteClass::Shutdown()
{
    ReleaseTextures();
    ShutdownBuffers();
    return;
}

bool SpriteClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;
    
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }
    RenderBuffers(deviceContext);
    
    return true;
}

void SpriteClass::Update(float frameTime)
{
    // 매 프레임 마다 프레임 시간을 누적
    m_frameTime += frameTime;
    // 프레임 시간이 순환 주기에 도달했는지 확인
    if (m_frameTime >= m_cycleTime)
    {
        // 도달했다면 프레임 시간을 초기화하고 텍스처 배열의 다음 스프라이트로 넘어간다.
        m_frameTime -= m_cycleTime;
        m_currentTexture++;
        
        // 마지막 스프라이트 텍스처라면 다시 텍스처 배열의 처음으로
        if (m_currentTexture == m_textureCount)
        {
            m_currentTexture = 0;
        }
    }
    
    return;
}

int SpriteClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* SpriteClass::GetTexture()
{
    return m_Textures[m_currentTexture].GetTexture();
}

void SpriteClass::SetRenderLocation(int x, int y)
{
    m_renderX = x;
    m_renderY = y;
    return;
}

bool SpriteClass::InitializeBuffers(ID3D11Device* device)
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
    
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));
    
    for (i = 0; i < m_vertexCount; i++)
    {
        indices[i] = i;
    }
    
    // 동적 정점 버퍼의 description을 설정한다.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 정점 데이터의 포인터를 지정한다.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 이제 마지막으로 정점 버퍼를 생성한다.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if(FAILED(result))
    {
        return false;
    }
    
    // 인덱스 버퍼의 설명 구조체를 설정한다.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 인덱스 데이터의 포인터를 지정한다.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼를 생성한다.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if(FAILED(result))
    {
        return false;
    }
    
    // 정점·인덱스 버퍼를 생성하고 채웠으니 배열을 해제한다.
    delete [] vertices;
    vertices = 0;

    delete [] indices;
    indices = 0;

    return true;
}

void SpriteClass::ShutdownBuffers()
{
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }
    
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }
    return;
}

bool SpriteClass::UpdateBuffers(ID3D11DeviceContext* deviceContent)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;
    
    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
    {
        return true;
    }
    
    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;
    
    vertices = new VertexType[m_vertexCount];
    
    // 비트맵 왼쪽 변의 화면 좌표를 계산한다.
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;

    // 비트맵 오른쪽 변의 화면 좌표를 계산한다.
    right = left + (float)m_bitmapWidth;

    // 비트맵 위쪽 변의 화면 좌표를 계산한다.
    top = (float)(m_screenHeight / 2) - (float)m_renderY;

    // 비트맵 아래쪽 변의 화면 좌표를 계산한다.
    bottom = top - (float)m_bitmapHeight;
    
    // 정점 배열에 데이터를 채운다.
    // 첫 번째 삼각형.
    vertices[0].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // 좌하단.
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // 두 번째 삼각형.
    vertices[3].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(right, top, 0.0f);  // 우상단.
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);
    
    // 정점 버퍼를 잠근다.
    result = deviceContent->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내부 데이터의 포인터를 얻는다.
    dataPtr = (VertexType*)mappedResource.pData;

    // 데이터를 정점 버퍼에 복사한다.
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // 정점 버퍼의 잠금을 해제한다.
    deviceContent->Unmap(m_vertexBuffer, 0);

    // 포인터 참조를 해제한다.
    dataPtr = 0;

    // 더 이상 필요 없으므로 정점 배열을 해제한다.
    delete [] vertices;
    vertices = 0;

    return true;
}

void SpriteClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;


    // 정점 버퍼의 스트라이드와 오프셋을 설정한다.
    stride = sizeof(VertexType);
    offset = 0;

    // 렌더링할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성화한다.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // 렌더링할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성화한다.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 이 정점 버퍼로 그릴 프리미티브 종류를 설정한다(여기서는 삼각형).
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

bool SpriteClass::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    char textureFilename[128];
    ifstream fin;
    int i, j;
    char input;
    bool result;


    // 스프라이트 정보 데이터 파일을 연다.
    fin.open(filename);
    if(fin.fail())
    {
        return false;
    }

    // 텍스처 개수를 읽어 들인다.
    fin >> m_textureCount;

    // 파일에서 읽은 텍스처 개수로 텍스처 배열을 생성하고 초기화한다.
    m_Textures = new TextureClass[m_textureCount];

    // 다음 줄 시작까지 읽는다.
    fin.get(input);

    // 각 텍스처 파일 이름을 읽어 들인다.
    for(i=0; i<m_textureCount; i++)
    {
        j=0;
        fin.get(input);
        while(input != '\n')
        {
            textureFilename[j] = input;
            j++;
            fin.get(input);
        }
        textureFilename[j] = '\0';

        // 파일 이름을 얻었으면 그 텍스처를 텍스처 배열에 불러온다.
        result = m_Textures[i].Initialize(device, deviceContext, textureFilename);
        if(!result)
        {
            return false;
        }
    }

    // 사이클 시간을 읽어 들인다.
    fin >> m_cycleTime;

    // 정수 밀리초를 실수 표현으로 변환한다.
    m_cycleTime = m_cycleTime * 0.001f;

    // 파일을 닫는다.
    fin.close();

    // 첫 번째 텍스처의 크기를 얻어 2D 스프라이트 이미지의 크기로 사용한다.
    m_bitmapWidth = m_Textures[0].GetWidth();
    m_bitmapHeight = m_Textures[0].GetHeight();

    // 사이클의 시작 텍스처를 목록의 첫 번째로 설정한다.
    m_currentTexture = 0;

    return true;
}

void SpriteClass::ReleaseTextures()
{
    int i;


    // 텍스처 객체들을 해제한다.
    if(m_Textures)
    {
        for(i=0; i<m_textureCount; i++)
        {
            m_Textures[i].Shutdown();
        }

        delete [] m_Textures;
        m_Textures = 0;
    }

    return;
}
