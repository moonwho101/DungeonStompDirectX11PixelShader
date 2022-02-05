#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

#include "LoadWorld.hpp"
#include "world.hpp"
#include "GlobalSettings.hpp"
#include "GameLogic.hpp"
#include <time.h>
#include <vector>
#include <DirectXMath.h>
#include "MathHelper.h"
#include "Missle.hpp"
#include "d3dutil.h"


typedef struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
};

extern int number_of_tex_aliases;
extern int totalcount;
extern CDXUTTextHelper* g_pTxtHelper;
extern D3DVERTEX bubble[600];
extern int countdisplay;
extern ID3D11Buffer* g_pcbCaptionBuffer;
extern ID3D11ShaderResourceView* save_out_srv;
extern Light mMainPassCB[MaxLights];



typedef struct Vertex {
    D3DXVECTOR3 position; // The position
    //D3DCOLOR color;    // The color
    FLOAT nx, ny, nz;   // Normals
    FLOAT tu, tv;   // The texture coordinates

    //FLOAT x, y, z; D3DXCOLOR Color;

};

Vertex mCaption[5000];

extern ID3D11ShaderResourceView* textures[400];


void display_font(float x, float y, char text[1000], int r, int g, int b);

void display_message(float x, float y, char text[2048], int r, int g, int b, float fontx, float fonty, int fonttype) {

    wchar_t wtext[120];
    mbstowcs(wtext, text, strlen(text) + 1);//Plus null

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos(x, y);
    g_pTxtHelper->SetForegroundColor(D3DXCOLOR(r, g, b, 1.0f));
    g_pTxtHelper->DrawTextLine(wtext);
    g_pTxtHelper->End();

}

HRESULT load_texture(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** out_srv)

{

    ID3D11Resource* new_texture;

    D3DX11_IMAGE_LOAD_INFO image_info;

    image_info.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;

    HRESULT hr = D3DX11CreateTextureFromFile(device, filename, &image_info, NULL, &new_texture, NULL);



    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;

    ZeroMemory(&srv_desc, sizeof(srv_desc));

    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    srv_desc.Texture2D.MipLevels = 1;

    srv_desc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(new_texture, &srv_desc, out_srv);

    return hr;

}




BOOL LoadRRTextures11(char* filename, ID3D11Device* pd3dDevice)
{
    FILE* fp;
    char s[256];
    char p[256];

    int y_count = 30;
    int done = 0;
    int object_count = 0;
    int vert_count = 0;
    int pv_count = 0;
    int poly_count = 0;
    int tex_alias_counter = 0;
    int tex_counter = 0;
    int i;
    BOOL start_flag = TRUE;
    BOOL found;
    //LPDIRECTDRAWSURFACE7 lpDDsurface;

    if (fopen_s(&fp, filename, "r") != 0)
    {
        //PrintMessage(hwnd, "ERROR can't open ", filename, SCN_AND_FILE);
        //MessageBox(hwnd, filename, "Error can't open", MB_OK);
        //return FALSE;
    }

    //D3DTextr_InvalidateAllTextures();

    //for (i = 0; i < MAX_NUM_TEXTURES; i++)
        //TexMap[i].is_alpha_texture = FALSE;

    //for (i = 0; i < MAX_NUM_TEXTURES; i++)
        //lpddsImagePtr[i] = NULL;

    //NumTextures = 0;

    for (int i = 0;i < 400;i++) {
        textures[i] = NULL;
    }

    while (done == 0)
    {
        found = FALSE;
        fscanf_s(fp, "%s", &s, 256);

        if (strcmp(s, "AddTexture") == 0)
        {
            fscanf_s(fp, "%s", &p, 256);
            //PrintMessage(hwnd, "Loading ", p, LOGFILE_ONLY);

            D3DX11_IMAGE_LOAD_INFO loadInfo;
            ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
            loadInfo.Usage = D3D11_USAGE_STAGING;
            loadInfo.Format = DXGI_FORMAT_FROM_FILE;
            loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ;

            ID3D11ShaderResourceView* out_srv = NULL;
            ID3D11Resource* new_texture;
            ID3D11Texture2D* pTexture2D = NULL;

            D3DX11_IMAGE_LOAD_INFO image_info;
            //image_info.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
            HRESULT hr = D3DX11CreateTextureFromFile(pd3dDevice, charToWChar(p), &image_info, NULL, (ID3D11Resource**)(&pTexture2D), NULL);

            D3D11_TEXTURE2D_DESC desc;
            pTexture2D->GetDesc(&desc);

            D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
            // Fill in the D3D11_SHADER_RESOURCE_VIEW_DESC structure.
            srv_desc.Format = desc.Format;
            srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.MipLevels = -1;

            // Create the shader resource view.
            hr = pd3dDevice->CreateShaderResourceView(pTexture2D, &srv_desc, &save_out_srv);

            if (hr == S_OK)
            {
                textures[tex_counter] = save_out_srv;
                tex_counter++;
                found = TRUE;

            }
            SAFE_RELEASE(pTexture2D);
        }

        if (strcmp(s, "Alias") == 0)
        {
            fscanf_s(fp, "%s", &p, 256);

            fscanf_s(fp, "%s", &p, 256);

            strcpy_s((char*)TexMap[tex_alias_counter].tex_alias_name, 100, (char*)&p);

            TexMap[tex_alias_counter].texture = tex_counter - 1;

            fscanf_s(fp, "%s", &p, 256);
            if (strcmp(p, "AlphaTransparent") == 0)
                TexMap[tex_alias_counter].is_alpha_texture = TRUE;

            i = tex_alias_counter;
            fscanf_s(fp, "%s", &p, 256);

            if (strcmp(p, "WHOLE") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.0;
            }

            if (strcmp(p, "TL_QUAD") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)0.5;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)0.5;
                TexMap[i].tv[2] = (float)0.5;
                TexMap[i].tu[3] = (float)0.5;
                TexMap[i].tv[3] = (float)0.0;
            }

            if (strcmp(p, "TR_QUAD") == 0)
            {
                TexMap[i].tu[0] = (float)0.5;
                TexMap[i].tv[0] = (float)0.5;
                TexMap[i].tu[1] = (float)0.5;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)0.5;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.0;
            }
            if (strcmp(p, "LL_QUAD") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.5;
                TexMap[i].tu[2] = (float)0.5;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)0.5;
                TexMap[i].tv[3] = (float)0.5;
            }
            if (strcmp(p, "LR_QUAD") == 0)
            {
                TexMap[i].tu[0] = (float)0.5;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.5;
                TexMap[i].tv[1] = (float)0.5;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.5;
            }
            if (strcmp(p, "TOP_HALF") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)0.5;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)0.5;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.0;
            }
            if (strcmp(p, "BOT_HALF") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.5;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.5;
            }
            if (strcmp(p, "LEFT_HALF") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.0;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)0.5;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)0.5;
                TexMap[i].tv[3] = (float)0.0;
            }
            if (strcmp(p, "RIGHT_HALF") == 0)
            {
                TexMap[i].tu[0] = (float)0.5;
                TexMap[i].tv[0] = (float)1.0;
                TexMap[i].tu[1] = (float)0.5;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)1.0;
                TexMap[i].tv[2] = (float)1.0;
                TexMap[i].tu[3] = (float)1.0;
                TexMap[i].tv[3] = (float)0.0;
            }
            if (strcmp(p, "TL_TRI") == 0)
            {
                TexMap[i].tu[0] = (float)0.0;
                TexMap[i].tv[0] = (float)0.0;
                TexMap[i].tu[1] = (float)1.0;
                TexMap[i].tv[1] = (float)0.0;
                TexMap[i].tu[2] = (float)0.0;
                TexMap[i].tv[2] = (float)1.0;
            }
            if (strcmp(p, "BR_TRI") == 0)
            {
            }

            tex_alias_counter++;
            found = TRUE;
        }

        if (strcmp(s, "END_FILE") == 0)
        {
            //PrintMessage(hwnd, "\n", NULL, LOGFILE_ONLY);
            number_of_tex_aliases = tex_alias_counter;
            //NumTextures = tex_counter;
            found = TRUE;
            done = 1;
        }

        if (found == FALSE)
        {
            //PrintMessage(hwnd, "File Error: Syntax problem :", p, SCN_AND_FILE);
            //MessageBox(hwnd, "p", "File Error: Syntax problem ", MB_OK);
            //return FALSE;
        }
    }
    fclose(fp);

    //D3DTextr_RestoreAllTextures(GetDevice());
    //DDCOLORKEY ckey;
    // set color key to black, for crosshair texture.
    // so any pixels in crosshair texture with color RGB 0,0,0 will be transparent
    //ckey.dwColorSpaceLowValue = RGB_MAKE(0, 0, 0);
    //ckey.dwColorSpaceHighValue = 0L;

    //for (i = 0; i < NumTextures; i++)
    //{
    //	lpDDsurface = D3DTextr_GetSurface(ImageFile[i]);
    //	lpddsImagePtr[i] = lpDDsurface;

    //	if (strstr(ImageFile[i], "@") != NULL || strstr(ImageFile[i], "fontA") != NULL || strstr(ImageFile[i], "die") != NULL || strstr(ImageFile[i], "dungeont") != NULL || strstr(ImageFile[i], "button") != NULL || strstr(ImageFile[i], "lightmap") != NULL || strstr(ImageFile[i], "flare") != NULL || strstr(ImageFile[i], "pb8") != NULL || strstr(ImageFile[i], "pb0") != NULL || strstr(ImageFile[i], "crosshair") != NULL || strstr(ImageFile[i], "pbm") != NULL || strstr(ImageFile[i], "box1") != NULL)
    //	{

    //		if (lpddsImagePtr[i])
    //			lpddsImagePtr[i]->SetColorKey(DDCKEY_SRCBLT, &ckey);
    //	}
    //	else
    //	{

    //		DDCOLORKEY ckeyfix;
    //		ckeyfix.dwColorSpaceLowValue = RGB_MAKE(9, 99, 99);
    //		ckeyfix.dwColorSpaceHighValue = 0L;

    //		if (lpddsImagePtr[i])
    //			lpddsImagePtr[i]->SetColorKey(DDCKEY_SRCBLT, &ckeyfix);
    //	}
    //}
    //PrintMessage(hwnd, "CMyD3DApplication::LoadRRTextures - suceeded", NULL, LOGFILE_ONLY);

    return TRUE;
}


static wchar_t* charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    wchar_t* wText = new wchar_t[size];
    mbstowcs(wText, text, size);
    return wText;
}

void ProcessLights11()
{
    int sort[200];
    float dist[200];
    int obj[200];
    int temp;

    for (int i = 0; i < MaxLights; i++)
    {
        mMainPassCB[i].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCB[i].FalloffStart = 100.0f;
        mMainPassCB[i].Direction = { 0.0f, -1.0f, 0.0f };
        mMainPassCB[i].FalloffEnd = 175.0f;
        mMainPassCB[i].Position = DirectX::XMFLOAT3{ 0.0f,9000.0f,0.0f };
        mMainPassCB[i].SpotPower = 90.0f;
    }


    int dcount = 0;
    //Find lights
    for (int q = 0; q < oblist_length; q++)
    {
        int ob_type = oblist[q].type;
        float	qdist = FastDistance(m_vEyePt.x - oblist[q].x,
            m_vEyePt.y - oblist[q].y,
            m_vEyePt.z - oblist[q].z);
        //if (ob_type == 57)
        if (ob_type == 6 && qdist < 2500 && oblist[q].light_source->command == 900)
        {
            dist[dcount] = qdist;
            sort[dcount] = dcount;
            obj[dcount] = q;
            dcount++;
        }
    }
    //sorting - ASCENDING ORDER
    for (int i = 0;i < dcount;i++)
    {
        for (int j = i + 1;j < dcount;j++)
        {
            if (dist[sort[i]] > dist[sort[j]])
            {
                temp = sort[i];
                sort[i] = sort[j];
                sort[j] = temp;
            }
        }
    }

    if (dcount > 16) {
        dcount = 16;
    }

    for (int i = 0;i < dcount;i++)
    {
        int q = obj[sort[i]];
        float dist2 = dist[sort[i]];

        int angle = (int)oblist[q].rot_angle;
        int ob_type = oblist[q].type;

        mMainPassCB[i + 1].Position = DirectX::XMFLOAT3{ oblist[q].x,oblist[q].y + 10.0f, oblist[q].z };
    }

    //01234567890123456789
    //PPPPPPPPPPPMMMMCSSSS

    int count = 0;

    for (int misslecount = 0; misslecount < MAX_MISSLE; misslecount++)
    {
        if (your_missle[misslecount].active == 1)
        {
            if (count < 4) {
                mMainPassCB[11 + count + 1].Position = DirectX::XMFLOAT3{ your_missle[misslecount].x, your_missle[misslecount].y, your_missle[misslecount].z };
                mMainPassCB[11 + count + 1].Strength = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
                mMainPassCB[11 + count + 1].FalloffStart = 200.0f;
                mMainPassCB[11 + count + 1].Direction = { 0.0f, -1.0f, 0.0f };
                mMainPassCB[11 + count + 1].FalloffEnd = 300.0f;
                mMainPassCB[11 + count + 1].SpotPower = 10.0f;


                if (your_missle[misslecount].model_id == 103) {
                    mMainPassCB[11 + count + 1].Strength = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
                }
                else if (your_missle[misslecount].model_id == 104) {
                    mMainPassCB[11 + count + 1].Strength = DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f };
                }
                else if (your_missle[misslecount].model_id == 105) {
                    mMainPassCB[11 + count + 1].Strength = DirectX::XMFLOAT3{ 1.0f, 0.0f, 1.0f };

                }
                count++;
            }
        }
    }

    bool flamesword = false;

    if (strstr(your_gun[current_gun].gunname, "FLAME") != NULL ||
        strstr(your_gun[current_gun].gunname, "ICE") != NULL ||
        strstr(your_gun[current_gun].gunname, "LIGHTNINGSWORD") != NULL)
    {
        flamesword = true;
    }

    if (flamesword) {

        int spot = 16;

        mMainPassCB[spot].Position = DirectX::XMFLOAT3{ m_vEyePt.x, m_vEyePt.y, m_vEyePt.z };
        mMainPassCB[spot].Strength = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
        mMainPassCB[spot].FalloffStart = 200.0f;
        mMainPassCB[spot].Direction = { 0.0f, -1.0f, 0.0f };
        mMainPassCB[spot].FalloffEnd = 300.0f;
        mMainPassCB[spot].SpotPower = 10.0f;

        if (strstr(your_gun[current_gun].gunname, "FLAME") != NULL) {
            mMainPassCB[spot].Strength = DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f };
        }
        else if (strstr(your_gun[current_gun].gunname, "ICE") != NULL) {

            mMainPassCB[spot].Strength = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };
        }
        else if (strstr(your_gun[current_gun].gunname, "LIGHTNINGSWORD") != NULL) {
            mMainPassCB[spot].Strength = DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };
        }
    }

    count = 0;
    dcount = 0;

    //Find lights SPOT
    for (int q = 0; q < oblist_length; q++)
    {
        int ob_type = oblist[q].type;
        float	qdist = FastDistance(m_vEyePt.x - oblist[q].x,
            m_vEyePt.y - oblist[q].y,
            m_vEyePt.z - oblist[q].z);
        //if (ob_type == 6)
        if (ob_type == 6 && qdist < 2500 && oblist[q].light_source->command == 1)
        {
            dist[dcount] = qdist;
            sort[dcount] = dcount;
            obj[dcount] = q;
            dcount++;
        }

    }

    //sorting - ASCENDING ORDER
    for (int i = 0;i < dcount;i++)
    {
        for (int j = i + 1;j < dcount;j++)
        {
            if (dist[sort[i]] > dist[sort[j]])
            {
                temp = sort[i];
                sort[i] = sort[j];
                sort[j] = temp;
            }
        }
    }

    if (dcount > 4) {
        dcount = 4;
    }

    for (int i = 0;i < dcount;i++)
    {
        int q = obj[sort[i]];
        float dist2 = dist[sort[i]];
        int angle = (int)oblist[q].rot_angle;
        int ob_type = oblist[q].type;

        mMainPassCB[i + 16 + 1].Position = DirectX::XMFLOAT3{ oblist[q].x,oblist[q].y + 20.0f, oblist[q].z };
        mMainPassCB[i + 16 + 1].Strength = DirectX::XMFLOAT3{ (float)oblist[q].light_source->rcolour, (float)oblist[q].light_source->gcolour, (float)oblist[q].light_source->bcolour };
        mMainPassCB[i + 16 + 1].FalloffStart = 600.0f;
        mMainPassCB[i + 16 + 1].Direction = { oblist[q].light_source->direction_x, oblist[q].light_source->direction_y, oblist[q].light_source->direction_z };
        mMainPassCB[i + 16 + 1].FalloffEnd = 800.0f;
        mMainPassCB[i + 16 + 1].SpotPower = 10.0f;
    }
}


void DisplayPlayerCaption2(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext) {

    int i;
    //LPDIRECTDRAWSURFACE7 lpDDsurface;
    float x, y, z;

    float pangle = 0;

    int countit = 0;
    int cullloop = 0;
    int cullflag = 0;
    int num = 0;
    int len = 0;
    int count = 0;
    char junk2[2000];
    int flag = 1;
    float yadjust = 0;

    D3DXMATRIX matWorld, matProj;

    //if (showtexture == 0)
        //return;

    D3DXMATRIX matRotate;
    int j = 0;

    //if (menuflares == 1)
    //{
    //	if (lastmaterial == 0)
    //	{
    //		D3DMATERIAL7 mtrl;
    //		D3DUtil_InitMaterial(mtrl, 1.0f, 1.0f, 1.0f, 1.0f);
    //		mtrl.emissive.r = 1.0f;
    //		mtrl.emissive.g = 1.0f;
    //		mtrl.emissive.b = 1.0f;
    //		lastmaterial = 1;
    //		m_pd3dDevice->SetMaterial(&mtrl);
    //	}
    //}

    //pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    //pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);
    //pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

    //pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    int bground = FindTextureAlias("fontA");
    //int bground = FindTextureAlias("die10s2");
    int texture_number = TexMap[bground].texture;

    //pd3dDevice->SetTexture(0, g_pTextureList[texture_number]); //set texture

    //lpDDsurface = lpddsImagePtr[texture_number];

    //if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
    //	PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

    //D3DVIEWPORT7 vp;
    //m_pd3dDevice->GetViewport(&vp);


    totalcount = 0;

    for (j = 0; j < num_monsters; j++)
    {

        cullflag = 0;
        for (cullloop = 0; cullloop < monstercount; cullloop++)
        {
            if (monstercull[cullloop] == monster_list[j].monsterid)
            {
                cullflag = 1;
                break;
            }
        }

        flag = 1;
        num = 0;
        count = 0;
        yadjust = 0.0f;



        if (monster_list[j].bIsPlayerValid && cullflag == 1 && monster_list[j].bStopAnimating == FALSE)
        {


            len = strlen(monster_list[j].chatstr);


            //TODO: why?
            //if (len > 0)
              //  len--;

            while (flag)
            {
                count = 0;

                while (monster_list[j].chatstr[num] != '!')
                {

                    junk2[count] = monster_list[j].chatstr[num];
                    count++;
                    num++;
                    if (num >= len)
                        break;
                }
                if (monster_list[j].chatstr[num] == '!')
                    num++;

                junk2[count] = '\0';

                if (num >= len || len == 0)
                    flag = 0;

                float x = monster_list[j].x;
                float y = monster_list[j].y + 28.0f - yadjust;
                float z = monster_list[j].z;

                yadjust += 6.0f;

                countdisplay = 0;
                display_font(0.0f, 0.0f, junk2, 255, 255, 0);

                D3DUtil_SetTranslateMatrix(matWorld, D3DXVECTOR3(x, y, z));
                D3DXMatrixRotationY(&matRotate, (angy * k + (int)3.14));
                D3DXMatrixMultiply(&matWorld, &matRotate, &matWorld);

                //pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
                //countdisplay = countdisplay;
                //g_pVB->Lock(0, sizeof(&pMonsterCaption), (void**)&pMonsterCaption, 0);



                D3DXVECTOR3 collidenow;
                D3DXVECTOR3 normroadold;
                D3DXVECTOR3 work1, work2, vDiff, vw1, vw2;
                normroadold.x = 50;
                normroadold.y = 0;
                normroadold.z = 0;

                work1.x = m_vEyePt.x;
                work1.y = m_vEyePt.y;
                work1.z = m_vEyePt.z;

                work2.x = x;
                work2.y = y;
                work2.z = z;

                vDiff = work1 - work2;

                D3DXVec3Normalize(&vDiff, &vDiff);

                vw1.x = work1.x;
                vw1.y = work1.y;
                vw1.z = work1.z;

                vw2.x = work2.x;
                vw2.y = work2.y;
                vw2.z = work2.z;

                vDiff = vw1 - vw2;

                D3DXVECTOR3 final, final2;
                D3DXVECTOR3 m, n;


                D3DXVec3Normalize(&final, &vDiff);
                D3DXVec3Normalize(&final2, &normroadold);
                float fDot = D3DXVec3Dot(&final, &final2);


                //float fDot = dot(final, final2);
                float convangle;
                convangle = (float)acos(fDot) / k;

                fDot = convangle;


                if (vw2.z < vw1.z)
                {
                    fDot = -1.0f * (180.0f - fDot) + 90.0f;
                }
                else
                {
                    fDot = 90.0f + (180.0f - fDot);
                }

                if ((vw2.x < vw1.x) && (vw2.z < vw1.z))
                {
                    fDot = fixangle(fDot, 360.0f);
                }


                float cosine = cos_table[(int)fDot];
                float sine = sin_table[(int)fDot];


                for (i = 0; i < ((countdisplay)); i += 1)
                {
                    D3DXVECTOR3 a = D3DXVECTOR3(bubble[i].x, bubble[i].y, bubble[i].z);
                    mCaption[i].position = a;
                    //mCaption[i].color = D3DCOLOR_RGBA(105, 105, 105, 0); //0xffffffff;
                    mCaption[i].tu = bubble[i].tu;
                    mCaption[i].tv = bubble[i].tv;

                    totalcount++;
                }


                for (i = 0; i < ((countdisplay)); i += 1)
                {
                    float x2 = mCaption[i].position.x;
                    float y2 = mCaption[i].position.y;
                    float z2 = mCaption[i].position.z;

                    //tx[vert_cnt] = obdata[ob_type].t[ob_vert_count].x;
                    //ty[vert_cnt] = obdata[ob_type].t[ob_vert_count].y;

                    float wx = x, wy = y, wz = z;

                    mCaption[i].position.x = wx + (x2 * cosine - z2 * sine);
                    mCaption[i].position.y = wy + y2;
                    mCaption[i].position.z = wz + (x2 * sine + z2 * cosine);
                }

                //flag = 0;
                //g_pVB->Unlock();
                //totalcount = totalcount - 1;
                //for (int i = 0; i < countdisplay; i = i + 4)
                //{
                    //pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i, 2);
                //}
                //if (yadjust > 6) {
                D3D11_MAPPED_SUBRESOURCE resource;
                pd3dImmediateContext->Map(g_pcbCaptionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
                int s = sizeof(Vertex) * countdisplay;
                memcpy(resource.pData, mCaption, s);
                pd3dImmediateContext->Unmap(g_pcbCaptionBuffer, 0);

                UINT stride = sizeof(Vertex);
                UINT offset = 0;
                pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_pcbCaptionBuffer, &stride, &offset);
                pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                pd3dImmediateContext->Draw(countdisplay, 0);
                //}

            }
        }

    }

    //pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}



void DrawScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext) {




    int currentObject = 0;
    for (currentObject = 0; currentObject < number_of_polys_per_frame; currentObject++)
    {
        int i = ObjectsToDraw[currentObject].vert_index;
        int vert_index = ObjectsToDraw[currentObject].srcstart;
        int fperpoly = (float)ObjectsToDraw[currentObject].srcfstart;
        int face_index = ObjectsToDraw[currentObject].srcfstart;

        int texture_alias_number = texture_list_buffer[i];
        int texture_number = TexMap[texture_alias_number].texture;

        //pd3dImmediateContext->SetTexture(0, g_pTextureList11[texture_number]); //set texture

        pd3dImmediateContext->PSSetShaderResources(0, 1, &textures[texture_number]);
        //pd3dImmediateContext->PSSetShaderResources(0, 1, &save_out_srv);

        if (dp_command_index_mode[i] == 1 && TexMap[texture_alias_number].is_alpha_texture == FALSE) {  //USE_NON_INDEXED_DP
            int primitive = 0;

            if (dp_commands[currentObject] == D3DPT_TRIANGLEFAN)
            {
                //no longer needed
                int v = verts_per_poly[currentObject];

                primitive = (verts_per_poly[currentObject] - 2);
                pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            }
            else if (dp_commands[currentObject] == D3DPT_TRIANGLESTRIP)
            {

                int v = verts_per_poly[currentObject];

                if (v > 4)
                    int gg = 1;

                primitive = (verts_per_poly[currentObject] - 2);
                pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                pd3dImmediateContext->Draw(v, vert_index);
            }
            else if (dp_commands[currentObject] == D3DPT_TRIANGLELIST)
            {
                primitive = verts_per_poly[currentObject] / 3;
                pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                //This is good
                int  v = verts_per_poly[currentObject];
                pd3dImmediateContext->Draw(v, vert_index);
            }

        }

    } // end for i




    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
    //blendStateDesc.AlphaToCoverageEnable = FALSE;
    //blendStateDesc.IndependentBlendEnable = FALSE;
    //blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    //blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
    //blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
    //blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    //blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    //blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    //blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    //blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    //printf("Failed To Create Blend State\n");
    float blendFactor[] = { 1.00f, 1.00f, 1.00f, 1.0f };


    blendStateDesc.AlphaToCoverageEnable = FALSE;
    blendStateDesc.IndependentBlendEnable = FALSE;
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pd3dDevice->CreateBlendState(&blendStateDesc, &blendState);

    pd3dImmediateContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

    bool draw = true;
    currentObject = 0;
    for (currentObject = 0; currentObject < number_of_polys_per_frame; currentObject++)
    {
        int i = ObjectsToDraw[currentObject].vert_index;
        int vert_index = ObjectsToDraw[currentObject].srcstart;
        int fperpoly = (float)ObjectsToDraw[currentObject].srcfstart;
        int face_index = ObjectsToDraw[currentObject].srcfstart;

        int texture_alias_number = texture_list_buffer[i];
        int texture_number = TexMap[texture_alias_number].texture;

        if (texture_number >= 94 && texture_number <= 101 ||
            texture_number >= 289 - 1 && texture_number <= 296 - 1 ||
            texture_number >= 279 - 1 && texture_number <= 288 - 1 ||
            texture_number >= 206 - 1 && texture_number <= 210 - 1) {
            draw = false;
        }
        else {
            draw = true;
        }

        if (draw) {

            //pd3dImmediateContext->SetTexture(0, g_pTextureList11[texture_number]); //set texture

            pd3dImmediateContext->PSSetShaderResources(0, 1, &textures[texture_number]);
            //pd3dImmediateContext->PSSetShaderResources(0, 1, &save_out_srv);

            if (dp_command_index_mode[i] == 1 && TexMap[texture_alias_number].is_alpha_texture == TRUE) {  //USE_NON_INDEXED_DP
                int primitive = 0;

                if (dp_commands[currentObject] == D3DPT_TRIANGLEFAN)
                {
                    //no longer needed
                    int v = verts_per_poly[currentObject];

                    primitive = (verts_per_poly[currentObject] - 2);
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

                }
                else if (dp_commands[currentObject] == D3DPT_TRIANGLESTRIP)
                {

                    int v = verts_per_poly[currentObject];

                    if (v > 4)
                        int gg = 1;

                    primitive = (verts_per_poly[currentObject] - 2);
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                    pd3dImmediateContext->Draw(v, vert_index);
                }
                else if (dp_commands[currentObject] == D3DPT_TRIANGLELIST)
                {
                    primitive = verts_per_poly[currentObject] / 3;
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                    //This is good
                    int  v = verts_per_poly[currentObject];
                    pd3dImmediateContext->Draw(v, vert_index);
                }
            }
        }


    } // end for i



    blendStateDesc.AlphaToCoverageEnable = FALSE;
    blendStateDesc.IndependentBlendEnable = FALSE;
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;






    pd3dImmediateContext->VSSetShader(g_pVertexShaderTorch11, NULL, 0);
    pd3dImmediateContext->PSSetShader(g_pPixelShaderTorch11, NULL, 0);

    SAFE_RELEASE(blendState);

    pd3dDevice->CreateBlendState(&blendStateDesc, &blendState);



    pd3dImmediateContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

    draw = true;

    currentObject = 0;
    for (currentObject = 0; currentObject < number_of_polys_per_frame; currentObject++)
    {
        int i = ObjectsToDraw[currentObject].vert_index;
        int vert_index = ObjectsToDraw[currentObject].srcstart;
        int fperpoly = (float)ObjectsToDraw[currentObject].srcfstart;
        int face_index = ObjectsToDraw[currentObject].srcfstart;

        int texture_alias_number = texture_list_buffer[i];
        int texture_number = TexMap[texture_alias_number].texture;

        if (texture_number >= 94 && texture_number <= 101 ||
            texture_number >= 289 - 1 && texture_number <= 296 - 1 ||
            texture_number >= 279 - 1 && texture_number <= 288 - 1 ||
            texture_number >= 206 - 1 && texture_number <= 210 - 1) {
            draw = true;
        }
        else {
            draw = false;
        }

        if (draw) {

            //pd3dImmediateContext->SetTexture(0, g_pTextureList11[texture_number]); //set texture

            pd3dImmediateContext->PSSetShaderResources(0, 1, &textures[texture_number]);
            //pd3dImmediateContext->PSSetShaderResources(0, 1, &save_out_srv);

            if (dp_command_index_mode[i] == 1 && TexMap[texture_alias_number].is_alpha_texture == TRUE) {  //USE_NON_INDEXED_DP
                int primitive = 0;

                if (dp_commands[currentObject] == D3DPT_TRIANGLEFAN)
                {
                    //no longer needed
                    int v = verts_per_poly[currentObject];

                    primitive = (verts_per_poly[currentObject] - 2);
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

                }
                else if (dp_commands[currentObject] == D3DPT_TRIANGLESTRIP)
                {

                    int v = verts_per_poly[currentObject];

                    if (v > 4)
                        int gg = 1;

                    primitive = (verts_per_poly[currentObject] - 2);
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                    pd3dImmediateContext->Draw(v, vert_index);
                }
                else if (dp_commands[currentObject] == D3DPT_TRIANGLELIST)
                {
                    primitive = verts_per_poly[currentObject] / 3;
                    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                    //This is good
                    int  v = verts_per_poly[currentObject];
                    pd3dImmediateContext->Draw(v, vert_index);
                }
            }
        }


    } // end for i



            //printf("Failed To Create Blend State\n");

    int t = FindTextureAlias("fontA");
    pd3dImmediateContext->PSSetShaderResources(0, 1, &textures[t]);

    DisplayPlayerCaption2(pd3dDevice, pd3dImmediateContext);


    pd3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
    //pd3dImmediateContext->OMSetBlendState(blendState, NULL, 0xffffffff);



    SAFE_RELEASE(blendState);

}
