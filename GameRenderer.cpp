//http://users.skynet.be/fquake/
//https://github.com/microsoft/Windows-universal-samples/tree/master/Samples/Simple3DGameDX
//https://github.com/Microsoft/Windows-appsample-marble-maze

#include "pch.h"
#include "GameRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DirectX11_Game;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
GameRenderer::GameRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, float xOff, float yOff) :
	m_loadingComplete(false),
	m_degreesPerSecond(0),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources),
	m_additionalScaling(0.1),
	m_waveIncremental(0),
	//m_eye(new XMVECTORF32({ 0.0f, 5.7f, 11.5f, 0.0f })),
	m_eye(new XMVECTORF32({ 0.0f, 5.7f, 11.5f, 0.0f })),
	//the cube instances
	m_modAmount(static_cast<int>(sqrtf((float)m_dataBufferSize))),
	m_halfModAmount(m_modAmount >> 1),
	m_dataBuffers(new ModelViewProjectionConstantBuffer[m_dataBufferSize]),
	m_mandlebrotXScale(0.45f / (m_modAmount >> 2)),
	m_mandlebrotYScale(0.25f / (m_modAmount >> 2))
{
	CreateDeviceDependentResources(xOff, yOff);

	// Initialize view for objects
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void GameRenderer::CreateWindowSizeDependentResources()
{
	InitializePerspective();
}

void GameRenderer::InitializePerspective() 
{
	//fixed size of 2
	for (int i = 0; i < m_dataBufferSize; i++)
	{
		
		Size outputSize = m_deviceResources->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		float fovAngleY = 70.0f * XM_PI / 180.0f;

		// This is a simple example of change that can be made when the app is in portrait or snapped view.
		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		// Note that the OrientationTransform3D matrix is post-multiplied here in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);
		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(
			&m_dataBuffers[i].projection,
			XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		//static const XMVECTORF32 eye = { 0.0f, 5.7f, 11.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&m_dataBuffers[i].view, XMMatrixTranspose(XMMatrixLookAtRH(m_eye->v, at, up)));
	}
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void GameRenderer::Update(float radians)
{
	if (!m_tracking)
	{	
		m_radians = radians;
		
		int j = 0;
		for (int i = 0; i < m_dataBufferSize; i++)
		{
			//increment row, locks to grid
			if (i % m_modAmount == 0 && i != 0)
				j++;

			int col = (i % m_modAmount) - m_halfModAmount;
			int row = j - m_halfModAmount;

			//threadPool.push_back(std::thread(ExecutePerRow(radians, &m_dataBuffers[i], col, row, i)));
			//m_threadPool->AddTask(ExecutePerRow);
			ExecutePerRow(col, row, i);
		}

		m_waveIncremental += 1;
		if (m_waveIncremental > m_modAmount)
			m_waveIncremental = -m_halfModAmount;
	}
}

// Rotate the 3D cube model a set amount of radians.
void GameRenderer::Rotate(float radians, ModelViewProjectionConstantBuffer* modelBuffer)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&modelBuffer->model, 
		XMMatrixTranspose(XMMatrixRotationY(radians)));
}

// Rotate the 3D cube model a set amount of radians.
void GameRenderer::Scale(float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&modelBuffer->model, 
		XMMatrixTranspose(XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt)));
}

void GameRenderer::ScaleRotate(float radians, float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer)
{
	XMStoreFloat4x4(&modelBuffer->model, 
		XMMatrixTranspose(
			XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt) * 
			XMMatrixRotationY(radians)));
}

void GameRenderer::Translate(XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer)
{
	XMStoreFloat4x4(&modelBuffer->model, 
		XMMatrixTranspose(XMMatrixTranslation(axis.x, axis.y, axis.z)));
}

void GameRenderer::TranslateScaleRotate(float radians, float scaleAmt, XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer)
{
	
	XMStoreFloat4x4(&modelBuffer->model, 
		XMMatrixTranspose(
			XMMatrixTranslation(axis.x, axis.y, axis.z) * 
			XMMatrixRotationY(radians) * 
			XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt)));
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void GameRenderer::TrackingUpdate(float positionX, ModelViewProjectionConstantBuffer* modelBuffer)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians, modelBuffer);
	}
}

// Renders one frame using the vertex and pixel shaders.
void GameRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	//LinkedData<ModelViewProjectionConstantBuffer> dataBufferLink;

	//https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage
	//for each buffer stored in the data buffers draw the value stored
	for (int i = 0; i < m_dataBufferSize; i++)
	{
		DrawObject(m_dataBuffers[i]);
	}

	//Draw resources instanced
	//m_deviceResources->GetD3DDeviceContext()->DrawInstanced(m_indexCount, m_dataBufferSize, 0, 0);

}

void GameRenderer::DrawObject(ModelViewProjectionConstantBuffer& modelBuffer)
{
	//this is where multiple objects are drawn
	auto context = m_deviceResources->GetD3DDeviceContext();
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &modelBuffer, 0, 0, 0);
	
	auto result = context->GetType();

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Each index is one 16-bit unsigned integer (short).
	//Input assembly -  You need to attach each vertex and index buffer for each object in the scene
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);
}
void GameRenderer::DrawObjectInstanced(ModelViewProjectionConstantBuffer* modelBuffer)
{
	//https://gamedev.stackexchange.com/questions/170192/instancing-with-directx11
	//this is where multiple objects are drawn
	auto context = m_deviceResources->GetD3DDeviceContext();

	context->DrawInstanced(8, m_indexCount, 0, 1);
}

//Shapes
//https://github.com/microsoft/Windows-appsample-marble-maze/blob/master/C%2B%2B/Shared/BasicShapes.cpp 

//Initialize the shaders and shapes to be used later
void GameRenderer::CreateDeviceDependentResources(float xOff, float yOff)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then(
		[this]()
		{
			CreateCube();
		});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]() {
		m_loadingComplete = true;
		});
}

HRESULT GameRenderer::CreateCube()
{
	HRESULT hr = S_OK;

	// Use the Direct3D device to load resources into graphics memory.
	ID3D11Device* device = m_deviceResources->GetD3DDevice();

	// Cube Geometry.
	VertexPositionColor CubeVertices[] = {
			{DirectX::XMFLOAT3(-0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0,   0,   0),},
			{DirectX::XMFLOAT3(-0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0,   0,   1),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0,   1,   0),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0,   1,   1),},
			{DirectX::XMFLOAT3(0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(1,   0,   0),},
			{DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(1,   0,   1),},
			{DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(1,   1,   0),},
			{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(1,   1,   1),}
	};


	/*VertexPositionColor CubeVertices[] = {
			{DirectX::XMFLOAT3(-0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0,   0,   0),},
			{DirectX::XMFLOAT3(-0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0,   0,   0.5),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0,   0.5,   0),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0,   0.5,   0.5),},
			{DirectX::XMFLOAT3(0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0.5,   0,   0),},
			{DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0.5,   0,   0.5),},
			{DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0.5,   0.5,   0),},
			{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0.5,   0.5,   0.5),}
	};*/

	/*	Cube
	VertexPositionColor ver1 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f, -0.5f),	XMFLOAT3(0.0f, 0.0f, 0.0f) };
	VertexPositionColor ver2 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f,  0.5f),	XMFLOAT3(0.0f, 0.0f, 1.0f) };
	VertexPositionColor ver3 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver4 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f,  0.5f),	XMFLOAT3(0.0f, 1.0f, 1.0f) };
	VertexPositionColor ver5 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, -0.5f),	XMFLOAT3(1.0f, 0.0f, 0.0f) };
	VertexPositionColor ver6 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, 0.5f),	XMFLOAT3(1.0f, 0.0f, 1.0f) };
	VertexPositionColor ver7 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver8 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, 0.5f),		XMFLOAT3(1.0f, 1.0f, 1.0f) };
	*/

	/* Half Triangle?
	VertexPositionColor ver1 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver2 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f,  0.5f),	XMFLOAT3(0.0f, 0.0f, 1.0f) };
	VertexPositionColor ver3 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver4 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f,  0.5f),	XMFLOAT3(0.0f, 1.0f, 1.0f) };
	VertexPositionColor ver5 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver6 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, 0.5f),	XMFLOAT3(1.0f, 0.0f, 1.0f) };
	VertexPositionColor ver7 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver8 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, 0.5f),		XMFLOAT3(1.0f, 1.0f, 1.0f) };
	*/

	// Create vertex buffer:

	CD3D11_BUFFER_DESC vDesc(
		sizeof(CubeVertices),
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = CubeVertices;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(
		&vDesc,
		&vData,
		&m_vertexBuffer
	);

	// Create index buffer:
	unsigned short CubeIndices[] =
	{
		0,2,1, // -x
		1,2,3,

		4,5,6, // +x
		5,7,6,

		0,1,5, // -y
		0,5,4,

		2,6,7, // +y
		2,7,3,

		0,4,6, // -z
		0,6,2,

		1,3,7, // +z
		1,7,5,
	};

	m_indexCount = ARRAYSIZE(CubeIndices);

	CD3D11_BUFFER_DESC iDesc(
		sizeof(CubeIndices),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = CubeIndices;
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(
		&iDesc,
		&iData,
		&m_indexBuffer
	);

	return hr;
}

void GameRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void GameRenderer::ExecutePerRow(int& column, int& row, int& index)
{
	
	//once the shape gets to as large as it will be, reduce it back to original
	//scaling
	//double rotation = fmod(totalRotation, 2) / 2;
	//if (rotation > 1)
	//	rotation = 1 - rotation;

	//set the value to the gridded location, offset so it is centered on screen
	float x = static_cast<float>(column);
	float z = static_cast<float>(row);

	//	Manipulate the Grid onClick
	//translation modifiers, creates gridded formation
	XMFLOAT3 axisLocation = XMFLOAT3({ static_cast<float>(column), 0, static_cast<float>(row) });
	XMFLOAT3 axisMods = XMFLOAT3({ x + m_cameraOffset.x, 0 + m_cameraOffset.y, z + m_cameraOffset.z });
	
	axisMods = GetManipulatedValues(&axisMods, index);

	//	Create the Wave
	//10 is the upper limit to how far back the wave travels
	for (int j = 0; j < 10; j++)
	{
		//maximum wave size is 10, step of 0.2
		//waveNum
		//x
		//y
		float diff = 2 - (j * 0.2f);
		if (z + j == m_waveIncremental)
			axisMods.y += diff / 2;
		else if (z - j == m_waveIncremental)
			axisMods.y += diff / 4;

		if (x + j == m_waveIncremental)
			axisMods.y += diff;
		else if (x - j == m_waveIncremental)
			axisMods.y += diff / 2;

		//difficulty implementing, please debug
		//GetWaveValue(j, x, z, 0.2f);
	}


	//if (m_manipulationType == 6) 
	//	axisMods.x -= GetManipulatedValue(&axisLocation);
	//float xRot = (m_modAmount / axisMods.x) * 360;
	//float zRot = (m_modAmount / axisMods.z) * 360;
	TranslateScaleRotate(m_radians + GetManipulatedRotation(&axisMods), m_additionalScaling, axisMods, &m_dataBuffers[index]);
}


// waveNum	- amount of waves that can be created
// x			- index of x
// z			- index of z
// waveSize	- actual size of a wave
float GameRenderer::GetWaveValue(int waveNum, int x, int z, float waveIncrement)
{
	float smallestWaveSize = 2 - (waveNum * waveIncrement);
	float waveSize = smallestWaveSize;
	if (x - waveNum == m_waveIncremental)
		waveSize += smallestWaveSize * 2; //+ diff * 2
			
	if (z + waveNum == m_waveIncremental)
		waveSize += smallestWaveSize * 2; //+ diff * 2
	else if (z - waveNum == m_waveIncremental)
		waveSize += smallestWaveSize * 4; // diff
	return waveSize;
}

void GameRenderer::ModifyCameraPosition(float amount, int direction)
{
	m_xOffset += static_cast<int>(amount);
	switch (direction)
	{
	case 0:
		m_cameraOffset.x += amount;
		break;
	case 1:
		m_cameraOffset.y += amount;
		break;
	case 2:
		m_cameraOffset.z += amount;
		break;
	}
}

void GameRenderer::UpdatePerspective(float amount)
{
	m_additionalScaling += amount;
	//m_eye->f[1] += amount * 10;
	//m_eye->f[2] += amount * 10;

	if (m_additionalScaling <= 0) 
	{
		m_additionalScaling = 0.02;
	}
}

//Adjust the rotations by set amount, modtype(0=positive, 1=negative)
void GameRenderer::ModifyDegreesPerSecond(float amount, int modType)
{
	if (amount == 0) 
	{
		m_degreesPerSecond = amount;
		return;
	}

	//LIMIT=360
	//if the degrees per second modulus the LIMIT is less than
	//	
	if (fmod(m_degreesPerSecond, 360) < m_degreesPerSecond)
		m_degreesPerSecond = (float)fmod(m_degreesPerSecond, 360.0f);
	
	switch (modType)
	{
	default:
	case 0:
		m_degreesPerSecond += amount;
		break;
	case 1:
		m_degreesPerSecond -= amount;
		break;
	}
}


float GameRenderer::GetManipulatedValue(XMFLOAT3* axisValues) 
{
	float valX = (m_modAmount - axisValues->x);
	float valZ = (m_modAmount - axisValues->z);
	//https://stackoverflow.com/questions/969798/plotting-a-point-on-the-edge-of-a-sphere
	float decimalPercentX = static_cast<float>(m_modAmount) / valX;
	float decimalPercentZ = static_cast<float>(m_modAmount) / valZ;
	float wholePercent = decimalPercentX * 100;
	
	//float valZ = (m_halfModAmount - axisValues->z) / 2.f;
	switch (m_manipulationType) 
	{
	case 1:
		return cosf(valX);
	case 2:
		return cosf(valZ);
		//return sinf(valZ);
	case 3:
		return cosf(valZ) + cosf(valX);
	case 4:
		//return cosf(valZ * valX); //every 5th we see more stability
		//return cosf((valZ / valX) * m_modAmount); //we see it go progressively smaller the larger X is 
		return cosf((valX / valZ) * m_modAmount) + cosf((valZ / valX) * m_modAmount);
	case 5:
		return wholePercent;
	case 6:
		//return fabsf(cosf(wholePercent));
		//currentX + circleCenter * cos(angle)
		
		//return(cosf(XMConvertToRadians(decimalPercentX * 360)) * sinf(XMConvertToRadians(decimalPercentZ * 360)));
		return XMConvertToRadians(decimalPercentX * 180) + XMConvertToRadians(decimalPercentZ * 180);
	case 0:
	default:
		return 0.f;
	}
}

float GameRenderer::GetManipulatedValue(float* axis)
{
	float val = (m_modAmount - *axis);
	//https://stackoverflow.com/questions/969798/plotting-a-point-on-the-edge-of-a-sphere
	float decimalPercent = static_cast<float>(m_modAmount) / val;
	float wholePercent = decimalPercent * 100;
	float radians = XMConvertToRadians(decimalPercent * 360);
	//float valZ = (m_halfModAmount - axisValues->z) / 2.f;
	switch (m_manipulationType)
	{
	case 1:
		return cosf(val);
	case 2:
		return cosf(val);
		//return sinf(valZ);
	case 3:
		return cosf(val) + sinf(val);
	case 4:
		//return cosf(valZ * valX); //every 5th we see more stability
		//return cosf((valZ / valX) * m_modAmount); //we see it go progressively smaller the larger X is 
		return m_halfModAmount * cosf(val * m_modAmount) + sinf(val * m_modAmount);
	case 5:
		return wholePercent;
	case 6:
		//return fabsf(cosf(wholePercent));
		//currentX + circleCenter * cos(angle)

		return m_halfModAmount * (cosf(radians) * sinf(radians));
	case 0:
	default:
		return 0.f;
	}
}

XMFLOAT3 GameRenderer::GetManipulatedValues(XMFLOAT3* axisValues, int arrayIndexValue)
{
	XMFLOAT3 newAxisValues = *axisValues;
	
	float valX = m_modAmount - axisValues->x;
	float valZ = m_modAmount - axisValues->z;
	//https://stackoverflow.com/questions/969798/plotting-a-point-on-the-edge-of-a-sphere
	float decimalPercentX = static_cast<float>(m_modAmount) / valX;
	float decimalPercentZ = static_cast<float>(m_modAmount) / valZ;
	float wholePercent = decimalPercentX * 100;
	
	float xRadians = XMConvertToRadians(decimalPercentX * 360); //-50
	float zRadians = XMConvertToRadians(decimalPercentZ * 360);
	
	//float valZ = (m_halfModAmount - axisValues->z) / 2.f;
	switch (m_manipulationType)
	{
	case 1:
		newAxisValues.y -= cosf(valX);
		break;
	case 2:
		newAxisValues.y -= cosf(valZ);
		break;
		//return sinf(valZ);
	case 3:
		newAxisValues.y -= cosf(valZ) + cosf(valX);
		break;
	case 4: //mandlebrot
		
		//alternative		if (m_storedMandlebrot.find(XMFLOAT2(valX, valZ)) == m_storedMandlebrot.end())
		//if (m_storedMandlebrot.count(arrayIndexValue) == 0)
		//{
		//	//insert the mandlebrot value
		//	m_storedMandlebrot.insert(
		//		std::make_pair(
		//			arrayIndexValue, 
		//			static_cast<float>(GetMandlebrotOffset(valX, valZ))));
		//} 		
		//newAxisValues.y = m_storedMandlebrot.at(arrayIndexValue);
		newAxisValues.y = GetMandlebrotOffset(valX + (m_cameraOffset.y / 2), valZ + (m_cameraOffset.y / 2));

		break;
	case 5: //gravity well
		//distance from the center
		
		//potentialInFeet = valX - m_halfModAmount
		//radialDistanceFromCenter = valZ - m_halfModAmount
		
		newAxisValues.x = valX - m_halfModAmount;
		newAxisValues.y = valZ - m_halfModAmount;
		newAxisValues.z += (32.2 * 10) / m_halfModAmount; //velocity
		//newAxisValues.y = tanf((valX * m_halfModAmount + valZ * m_halfModAmount) / (sqrtf((valX * 2) + (valZ * 2)) * sqrtf(m_modAmount + m_modAmount))); //velocity
		break;
	case 6: //sphere
		//return fabsf(cosf(wholePercent));
		//currentX + circleCenter * cos(angle)
		//xRadians = XMConvertToRadians(decimalPercentX * 360);
		//zRadians = XMConvertToRadians(decimalPercentZ * 360);

		newAxisValues.z = m_halfModAmount * cosf(xRadians) * sinf(xRadians);
		newAxisValues.x = m_halfModAmount * cosf(xRadians) * sinf(zRadians);
		newAxisValues.y = m_halfModAmount * cosf(zRadians);
		//newAxisValues.z = m_halfModAmount	* (cosf(zRadians)); // * sinf(zRadians));
		break; 
	case 0:
	default:
		//newAxisValues.y -= cosf(XMConvertToRadians(decimalPercentX * 360));
		break;
	}
	return newAxisValues;
}
float GameRenderer::GetManipulatedRotation(XMFLOAT3* axisValues)
{
	float xPercentOfWhole = m_modAmount / axisValues->x;
	float yPercentOfWhole = m_modAmount / axisValues->y;
	float zPercentOfWhole = m_modAmount / axisValues->z;
	
	switch (m_manipulationType) 
	{
	case 4:
		//return XMConvertToRadians((xPercentOfWhole * 179) + (zPercentOfWhole * 179) + 2);
		return 0.f;
	case 5:
		return XMConvertToRadians((xPercentOfWhole * 90) + (zPercentOfWhole * 90) + 1);
	case 6:
		return XMConvertToRadians(zPercentOfWhole * 360);
	}
	return 0.f;

}

int GameRenderer::GetMandlebrotOffset(int x, int y)
{
	//https://www.geeksforgeeks.org/fractals-in-cc/
	//percent of whole
	/*  left = -1.75; top = -0.25;
		xside = 0.25; yside = 0.45; */
	float cx = x * m_mandlebrotXScale + -1.75f;// *(0.25f / 2) + -1.75f;  // c_real           //cx = x * xscale + left;
	float cy = y * m_mandlebrotYScale + -0.25f;// *(0.25f / 2) + -0.25f;  // c_imaginary      //cy = y * yscale + top;

	float zx = 0;  // z_real 
	float zy = 0;  // z_imaginary 

	int count = -2;

	// Calculate whether c(c_real + c_imaginary) belongs 
	// to the Mandelbrot set or not and draw a pixel 
	// at coordinates (x, y) accordingly 
	// If you reach the Maximum number of iterations 
	// and If the distance from the origin is 
	// greater than 2 exit the loop 
	for (int i = 0; i < m_halfModAmount; i++)
	{
		if (zx * zx + zy * zy >= 4)
			break;

		// Calculate Mandelbrot function 
		// z = z*z + c where z is a complex number 
		float tempx = zx * zx - zy * zy + cx; // tempx = z_real*_real - z_imaginary*z_imaginary + c_real

		zy = 2 * zx * zy + cy; // 2*z_real*z_imaginary + c_imaginary 

		zx = tempx;     // Updating z_real = tempx 
		count = i;
	}

	return count;
}
