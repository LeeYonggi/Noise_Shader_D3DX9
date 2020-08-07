//--------------------------------------------------------------------------------------
// File: 2D_Reflection_Shader_Test.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "resource.h"

#define DEVICE DXUTGetD3D9Device()

LPD3DXSPRITE sprite = nullptr;
void CreateTexSurface(LPDIRECT3DTEXTURE9& tex, LPDIRECT3DSURFACE9& surf)
{
    HRESULT result = DEVICE->CreateTexture(
        1280, 720,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &tex,
        nullptr
    );

    tex->GetSurfaceLevel(0, &surf);
}

void CreateShader(std::string path, LPD3DXEFFECT& shader)
{
    LPD3DXBUFFER error = nullptr;

    D3DXCreateEffectFromFileA(
        DEVICE,
        path.c_str(),
        nullptr, nullptr,
        D3DXSHADER_DEBUG, 0,
        &shader,
        &error
    );

    if (error)
        MessageBoxA(DXUTGetHWND(), (LPCSTR)error->GetBufferPointer(), 0, 0);
}

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
    // Typically want to skip back buffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}

Texture playerTex = nullptr;
Texture backgroundTex = nullptr;
Texture backSurfTex = nullptr;
Texture reflectionSurfTex = nullptr;
Texture waterHoleTex = nullptr;
Texture noiseTex = nullptr;

D3DXIMAGE_INFO playerInfo;
D3DXIMAGE_INFO backgroundInfo;
D3DXIMAGE_INFO waterHoleInfo;
D3DXIMAGE_INFO noiseInfo;

LPDIRECT3DSURFACE9 backSurf = nullptr;
LPDIRECT3DSURFACE9 reflectionSurf = nullptr;

LPD3DXEFFECT reflectionShader = nullptr;

Vector2 playerPos = Vector2(640, 360);


Texture AddTexture(std::string fileName, D3DXIMAGE_INFO &outInfo)
{
    Texture tempTexture = nullptr;

    D3DXCreateTextureFromFileExA(DEVICE,
        fileName.c_str(),
        D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
        D3DX_DEFAULT, 0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT, D3DX_DEFAULT,
        0,
        &outInfo,
        nullptr,
        &tempTexture);

    return tempTexture;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    D3DXCreateSprite(DEVICE, &sprite);
    playerTex = AddTexture("./Resources/Player.png", playerInfo);
    backgroundTex = AddTexture("./Resources/Background.png", backgroundInfo);
    waterHoleTex = AddTexture("./Resources/Waterhole.png", waterHoleInfo);
    noiseTex = AddTexture("./Resources/noise2.png", noiseInfo);

    CreateTexSurface(backSurfTex, backSurf);
    CreateTexSurface(reflectionSurfTex, reflectionSurf);

    CreateShader("./Resources/Reflaction.fx", reflectionShader);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    if (GetAsyncKeyState('W'))
    {
        playerPos.y += -3.0f;
    }
    if (GetAsyncKeyState('A'))
    {
        playerPos.x += -3.0f;
    }
    if (GetAsyncKeyState('S'))
    {
        playerPos.y += 3.0f;
    }
    if (GetAsyncKeyState('D'))
    {
        playerPos.x += 3.0f;
    }
}

void Draw(Texture tex, D3DXIMAGE_INFO info, Vector2 pos)
{
    Matrix matW, matS, matR, matT;

    D3DXMatrixScaling(&matS, 1, -1, 1);
    D3DXMatrixRotationZ(&matR, 0);
    D3DXMatrixTranslation(&matT, pos.x, pos.y, 0);

    matW = matS * matR * matT;

    sprite->SetTransform(&matW);

    Vector3 center = { info.Width * 0.5f, info.Height * 0.5f, 0 };

    sprite->Draw(tex, nullptr, &center, nullptr, D3DCOLOR_RGBA(255, 255, 255, 255));
}

void Render()
{
    LPDIRECT3DSURFACE9 firstSurf;

    DEVICE->GetRenderTarget(0, &firstSurf);

    DEVICE->SetRenderTarget(0, backSurf);

    DEVICE->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DXCOLOR(0, 0, 0, 0), 1, 0);

    sprite->Begin(D3DXSPRITE_ALPHABLEND);

    Draw(backgroundTex, backgroundInfo, Vector2(640, 360));

    Draw(playerTex, playerInfo, playerPos);

    sprite->End();

    DEVICE->SetRenderTarget(0, reflectionSurf);
    DEVICE->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DXCOLOR(0, 0, 0, 0), 1, 0);

    sprite->Begin(D3DXSPRITE_ALPHABLEND);

    Draw(waterHoleTex, waterHoleInfo, Vector2(640, 360));

    sprite->End();

    DEVICE->SetRenderTarget(0, firstSurf);
    
    reflectionShader->SetTexture((D3DXHANDLE)"gLightTexture", backSurfTex);
    reflectionShader->SetTexture((D3DXHANDLE)"gDarkTexture", reflectionSurfTex);
    reflectionShader->SetTexture((D3DXHANDLE)"gNoiseTexture", noiseTex);
    reflectionShader->SetFloat((D3DXHANDLE)"time", DXUTGetTime() / 20);

    UINT num = 0;
    reflectionShader->Begin(&num, 0);

    for (int i = 0; i < num; i++)
    {
        Matrix tempMatW;

        D3DXMatrixTranslation(&tempMatW, 640, 360, 0);

        sprite->SetTransform(&tempMatW);

        sprite->Begin(D3DXSPRITE_ALPHABLEND);

        reflectionShader->BeginPass(i);

        Vector3 center = { 640, 360, 0 };
        sprite->Draw(backSurfTex, nullptr, &center, nullptr, D3DXCOLOR(1, 1, 1, 1));

        reflectionShader->EndPass();

        sprite->End();
    }
    reflectionShader->End();
    
    SAFE_RELEASE(firstSurf);
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        Render();
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    return 0;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    SAFE_RELEASE(sprite);

    SAFE_RELEASE(playerTex);
    playerTex = nullptr;

    SAFE_RELEASE(backgroundTex);
    backgroundTex = nullptr;

    SAFE_RELEASE(reflectionShader);
    SAFE_RELEASE(reflectionSurf);
    SAFE_RELEASE(reflectionSurfTex);
    SAFE_RELEASE(backSurf);
    SAFE_RELEASE(backSurfTex);
    SAFE_RELEASE(waterHoleTex);
    SAFE_RELEASE(noiseTex);
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // TODO: Perform any application-level initialization here

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"2D_Reflection_Shader_Test" );
    DXUTCreateDevice( true, 1280, 720 );

    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here

    return DXUTGetExitCode();
}


