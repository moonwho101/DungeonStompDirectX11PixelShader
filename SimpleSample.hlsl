//--------------------------------------------------------------------------------------
// File: SimpleSample.hlsl
//
// The HLSL file for the SimpleSample sample for the Direct3D 11 device
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// Include structures and functions for lighting.
#define MaxLights 21

struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
};

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register(b0)
{
    matrix  g_mWorldViewProjection  : packoffset(c0);
    matrix  g_mWorld                : packoffset(c4);
    float4  g_MaterialAmbientColor  : packoffset(c8);
    float4  g_MaterialDiffuseColor  : packoffset(c9);
}

cbuffer cbPerFrame : register(b1)
{
    float3              g_vLightDir             : packoffset(c0);
    float               g_fTime : packoffset(c0.w);
    float4              g_LightDiffuse          : packoffset(c1);
    
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
{
    Light gLights[MaxLights];
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;


    //return lightStrength;
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    //return lightStrength;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    // Scale by spotlight
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
    lightStrength *= spotFactor;

    //return lightStrength;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//-----------------------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------------------
Texture2D    g_txDiffuse : register(t0);
SamplerState g_samLinear : register(s0);

//--------------------------------------------------------------------------------------
// shader input/output structure
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position     : POSITION; // vertex position 
    float3 Normal       : NORMAL;   // this normal comes in per-vertex
    float2 TextureUV    : TEXCOORD0;// vertex texture coords 
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
    float3 PosW    : POSITION;
    float3 Normal       : NORMAL;   // this normal comes in per-vertex
    float4 Diffuse      : COLOR0;      // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV    : TEXCOORD0;   // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS(VS_INPUT input)
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    float3 result = 0.0f;
    float3 gEyePosW = float3(g_vLightDir.x, g_vLightDir.y, g_vLightDir.z);
    int i = 0;

    float4 posW = mul(input.Position, g_mWorld);
    Output.PosW = posW.xyz;

    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(posW, g_mWorldViewProjection);

    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(mul(input.Normal, (float3x3)g_mWorld)); // normal (world space)

    // Just copy the texture coordinate through
    Output.TextureUV = input.TextureUV;
    Output.Normal = mul(input.Normal, (float3x3)g_mWorld);

    // Calc diffuse color    
    //Output.Diffuse.rgb = g_MaterialDiffuseColor * g_LightDiffuse * max(0, dot(vNormalWorldSpace, g_vLightDir)) + g_MaterialAmbientColor;
    //Output.Diffuse.a = 1.0f;

    /*

    Material mat;
    mat.DiffuseAlbedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    mat.FresnelR0 = float3(0.01f, 0.01f, 0.01f);
    mat.Shininess = float(0.0f);

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    //vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
    

    //Output.Normal = normalize(input.Normal);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - input.Position);

    //result += 1.0f * ComputeDirectionalLight(l, mat, input.Normal, toEyeW);
    for (i = 1; i < 16; ++i)
    {

        if (i < 12) {
            result += ComputePointLight(gLights[i], mat, input.Position, input.Normal, toEyeW);
        }
        else {
            result += ComputeSpotLight(gLights[i], mat, input.Position, input.Normal, toEyeW);
        }

        //result += ComputeSpotLight(gLights[i], mat, input.Position, input.Normal, toEyeW);

        //result += ComputeDirectionalLight(gLights[i], mat, input.Normal, toEyeW);
    }


    Output.Diffuse.rgb = result.rgb;
    //Output.Diffuse.a = 1.0f;
    //Output.Color = result.Color;
    */

    return Output;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
float4 RenderScenePS(VS_OUTPUT Input) : SV_TARGET
{
    float3 result = 0.0f;
    int i = 0;
    
    Material mat;
    mat.DiffuseAlbedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    mat.FresnelR0 = float3(0.01f, 0.01f, 0.01f);
    mat.Shininess = float(0.0f);

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    //vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
    //Output.Normal = normalize(input.Normal);

    // Vector from point being lit to eye. 
    float3 toEyeW = 0.0f;

    //01234567890123456789
    //PPPPPPPPPPPMMMMCSSSS
    //First light is broken
    //result += ComputeDirectionalLight(gLights[i], mat, Input.Normal, toEyeW);
    
    for (i = 1; i < MaxLights; ++i)
    {
        if (i < 17) {
            result += ComputePointLight(gLights[i], mat, Input.PosW, Input.Normal, toEyeW);
        }
        else {
            result += ComputeSpotLight(gLights[i], mat, Input.PosW, Input.Normal, toEyeW);
        }
    }

    return float4(result,0.0f) * g_txDiffuse.Sample(g_samLinear, Input.TextureUV);

    // Lookup mesh texture and modulate it with diffuse
    return g_txDiffuse.Sample(g_samLinear, Input.TextureUV) * Input.Diffuse;
}
