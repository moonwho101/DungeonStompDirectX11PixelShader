//The shader handles the mixing effect of unknown number of light sources and unknown types of light sources
//A total of 256 float4 are supported in HLSL, an int is finally converted to float4, float4x3 and float3x3 occupy the same memory, and eventually become float4
//Because the limitations of HLSL are very scattered and very many, so it is still very difficult to use, without understanding,
//The final instruction cannot exceed 500 seemingly, run the program in the graphics card, so it can be saved, and it can be streamlined!

//
//Structure Note: In order to optimize the structure, some related parameters are integrated together
//
struct PointLight//Point light source structure
{
	float4 Diffuse;//diffuse color r, g, b, a
	float4 Specular;//The color of specular highlights r, g, b, a
	float4 Position;//Light source position x, y, z, w are not used	
	float4 RangeAttenuation;//Range, constant attenuation, light intensity, secondary attenuation
};

struct DirectLight//Directional light structure
{
	float4 Diffuse;//diffuse color r, g, b, a
	float4 Specular;//The color of specular highlights r, g, b, a
	float4 Direction;//direction x, y, z, highlight
};

struct SpotLight//Spotlight structure
{
	float4 Diffuse;//diffuse color r, g, b, a
	float4 Specular;//The color of specular highlights r, g, b, a
	float4 Position;//Light source position x, y, z, w are not used	
	float4 Direction;//The directions x, y, z, w are not used	
	float4 RangeAttenuation;//Range, constant attenuation, light intensity, secondary attenuation
	float4 FalloffThetaPhi;//Intensity attenuation between inner cone and outer cone
};

//Number of receiving light sources
int g_NumPLs;//How many point light sources are defined in the program?
int g_NumDLs;//How many directional light sources are defined in the program?
int g_NumSLs;//How many directional light sources are defined in the program?

PointLight g_PLs[10];//Define to support up to 10 point light sources
DirectLight g_DLs[10];//Define to support up to 10 directional light sources
SpotLight g_SLs[10];//Define to support up to 10 spotlight light sources

//Ambient light (for multi-light situations, perform these calculations for each light cycle (except for ambient light))
//Finally add ambient light (assuming there is only one ambient light)

matrix g_WorldMatrix;//World matrix
float4 g_ViewPos;//Observation point (camera)
matrix WVPMatrix;//World-observation-projection matrix

float AmbAmount;//The intensity of the ambient light, between 0-1
float4 AmbCol = { 1.0f, 1.0f, 1.0f, 1.0f };//The color of the ambient light, the default is white


texture colorMapTexture;

sampler2D colorMap = sampler_state
{
	Texture = <colorMapTexture>;
	MagFilter = Linear;
	MinFilter = Anisotropic;
	MipFilter = Linear;
	MaxAnisotropy = 16;
};

//------------------------------------------------ --------------------------
//Vertex shader (note below, I use the output vertex position and normal of the vertex shader as the input of the pixel shader !!! 
//TEXCOORD and other registers are useless, it is most suitable for storing vertex and normal data, at most it seems to be 15)
//------------------------------------------------ --------------------------
struct VS_INPUT//Input structure
{
	float4 position : POSITION;//Input: coordinates
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;//Input: normal
};
struct VS_OUTPUT//Output structure
{
	float4 position : POSITION;
	float4 vtpos : TEXCOORD2;//For incoming pixel shader, TEXCOORD represents the register name, (actually the saved data is used as the input coordinates of the pixel shader, use TEXCOORD, on the one hand there are many, the second is useless)
	float3 normal : TEXCOORD1;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.vtpos = input.position;//Model local space coordinates
	output.position = mul(input.position, WVPMatrix);//Output: world-observation-projection changing coordinates
	output.normal = input.normal;
	output.texCoord = input.texCoord;

	return output;
}

//------------------------------------------------ --------------------------
//Pixel shader (COLOR0 must be output, others can be used for any purpose)
//------------------------------------------------ --------------------------
struct PS_INPUT//input
{
	float4 vtpos : TEXCOORD2;//Vertex position (accept the output of the vertex shader, corresponding)
	float3 normal : TEXCOORD1;
	float2 texCoord :TEXCOORD0;
};
struct PS_OUTPUT//Output pixel color
{
	float4 Color : COLOR0;//The pixel shader finally outputs the calculated color
};
PS_OUTPUT PS_Main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	float4 worldpos = mul(input.vtpos, g_WorldMatrix);//The coordinates of the vertex in world space, that is, multiplied by the world matrix
	float4 colRes;//used to save the final color = E ambient light + self light + E (point light + direction light + spot light); where the point, direction, spot light contains diffuse reflection, specular reflection
	float4 ambient = { 0.0f, 0.0f, 0.0f, 1.0f };//Total ambient light
	float4 self = { 0.0f, 0.0f, 0.0f, 1.0f };//Self-illumination
	float4 diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };//total diffuse reflected light
	float4 specular = { 0.0f, 0.0f, 0.0f, 1.0f };//Total specular reflection

	ambient = AmbCol * AmbAmount;//Find the ambient light


	for (int i = 0; i < g_NumPLs; i++)//point light
	{
		float3 dirSize = input.vtpos.xyz - g_PLs[i].Position.xyz;//The direction and size of the light source to the vertex. Light source → vertex pos
		float distance = length(dirSize);//Distance
		float3 dir = normalize(dirSize);//Unitization direction
		if (distance < g_PLs[i].RangeAttenuation.x)//The vertex is within the light range
		{
			//Find the diffuse reflection
			float difusefactor = dot(dir.xyz, input.normal.xyz);
			if (difusefactor < 0)
			{
				difusefactor = 0;
			}
			float distanceDec = 1.0f - 1.0f / g_PLs[i].RangeAttenuation.x * distance;
			//diffuse reflection = diffuse * diffuse reflection factor * distance attenuation
			diffuse.xyz += g_PLs[i].Diffuse.xyz * difusefactor * distanceDec;//Finally multiply by the absorption coefficient of the material (not multiplied here)

			//Let's find the specular reflection below (the algorithm of specular reflection should go to Baidu to find it ...)
			float3 DirectionToView = normalize(worldpos.xyz - g_ViewPos.xyz);//(Also in world space!)
			float3 VectorToLight = normalize(input.vtpos.xyz - g_PLs[i].Position.xyz);
			//Calculate reflected light
			float3 reflectanceRay = 2 * dot(input.normal.xyz, VectorToLight.xyz) * input.normal.xyz - VectorToLight.xyz;
			float specfactor = pow(abs(dot(reflectanceRay, DirectionToView)), g_PLs[i].RangeAttenuation.z);
			//Mirror light accumulation
			specular.xyz += g_PLs[i].Specular.xyz * specfactor;//Finally, it is multiplied by the absorption coefficient of the material (not multiplied here)
			//float a = 1.0f / (1.0f + 0.08f * distance + 0.0f * distance * distance);
			//float a = clamp(20.0 / distance, 0.2, 1.0);
			//specular *= a;
			//diffuse *= a;
			//ambient *= a;
		}
		//else is outside the light, there is no influence of this light
	}
	for (int j = 0; j < g_NumDLs; j++)//Directional light source
	{
		//diffuse reflection
		float3 dir = normalize(g_DLs[j].Direction.xyz);//Unitization direction (direction of light)
		float difusefactor = dot(dir.xyz, input.normal.xyz);
		if (difusefactor < 0)
		{
			difusefactor = 0;
		}
		diffuse.xyz += g_DLs[j].Diffuse.xyz * difusefactor;//Finally, multiply by the absorption coefficient of the material (not multiplied here)

		//reflection of mirror
		float3 DirectionToView = normalize(worldpos.xyz - g_ViewPos.xyz);//observation point → vertex (same in world space!)
		//dir already
			//Calculate reflected light
		float3 reflectanceRay = 2 * dot(input.normal.xyz, dir.xyz) * input.normal.xyz - dir.xyz;
		float specfactor = pow(abs(dot(reflectanceRay, DirectionToView)), g_DLs[j].Direction.w);
		//Mirror light accumulation
		specular.xyz += g_DLs[j].Specular.xyz * specfactor;//Finally, it must be multiplied by the absorption coefficient of the material (not multiplied here)
	}


	for (int k = 0; k < g_NumSLs; k++)//Spotlight
	{
		float disdec = 0.0f;//distance decay
		float raddec = 0.0f;//Angle attenuation

		float distance = length(g_SLs[k].Position.xyz - worldpos.xyz);//The distance from the light source to the vertex
		float3 xconeDirection = normalize(g_SLs[k].Direction.xyz);//Spotlight direction
		float3 ltvdir = normalize(g_SLs[k].Position.xyz - worldpos.xyz);//Light to vertex direction
		float cosx = abs(dot(xconeDirection, ltvdir));//Cosine of the angle

		float cosPhi = cos(g_SLs[k].FalloffThetaPhi.w / 2.0f);
		float cosTheta = cos(g_SLs[k].FalloffThetaPhi.z / 2.0f);

		//Distance attenuation factor
		float sss = 1.0f / g_SLs[k].RangeAttenuation.x;

		if (cosx > cosTheta)//Umbra (angle attenuation: no attenuation)
		{
			raddec = 1.0f;

			if (distance * cosx < g_SLs[k].RangeAttenuation.x)//Within the range
			{
				disdec = 1.0f - 1.0f / g_SLs[k].RangeAttenuation.x * distance * cosx;//Distance attenuation (linear attenuation)
			}
		}

		if (cosx < cosTheta)//Umbrella outside (angle attenuation: sharp attenuation)
		{
			float v1 = cosx - cosPhi;
			float v2 = cosTheta - cosPhi;
			float v3 = v1 / v2;
			raddec = pow(abs(v3), g_SLs[k].FalloffThetaPhi.x);

			if (distance * cosx < g_SLs[k].RangeAttenuation.x)//Within the range (no light)
			{
				disdec = 1.0f - 1.0f / g_SLs[k].RangeAttenuation.x * distance * cosx;//distance attenuation (linear attenuation)
			}
		}

		if (cosx < cosPhi)//Outside the light cone
		{
			raddec = 0.0f;
		}

		//Calculate diffuse reflection
		float difusefactor = dot(ltvdir.xyz, input.normal.xyz);
		if (difusefactor < 0.0f)
		{
			difusefactor = 0.0f;
		}
		diffuse.xyz += g_SLs[k].Diffuse.xyz * raddec * disdec * difusefactor;//Finally, multiply by the absorption coefficient of the material (not multiplied here)

		//if (difusefactor > 0.0f)
		//{

			//Calculate specular reflection
			float3 DirectionToView = normalize(worldpos.xyz - g_ViewPos.xyz);//observation point → vertex (same in world space!)
			float3 VectorToLight = normalize(input.vtpos.xyz - g_SLs[k].Position.xyz);
			//Calculate reflected light
			float3 reflectanceRay = 2 * dot(input.normal.xyz, VectorToLight.xyz) * input.normal.xyz - VectorToLight.xyz;
			float specfactor = pow(abs(dot(reflectanceRay, DirectionToView)), g_SLs[k].RangeAttenuation.z);
			//Mirror light accumulation
			specular.xyz += g_SLs[k].Specular.xyz * raddec * disdec * specfactor;//Finally, it is multiplied by the absorption coefficient of the material (not multiplied here)
		//}
	}

	output.Color.w = 1.0f;//Because output is initialized to (PS_OUTPUT) 0, w is also initialized to 0.
	output.Color.xyz = ambient.xyz + self.xyz + diffuse.xyz + specular.xyz;//Add up all kinds of light


	//color * tex2D(colorMap, IN.texCoord);

	//output.Color *= float4(0.9f, 0.8f, 0.0f, 1);   //do a simple effect

    float4 result = tex2D(colorMap, input.texCoord);
	output.Color *= result;

	return output;
}

//------------------------------------------------ --------------------------
//Frame effect
//------------------------------------------------ --------------------------
technique MulLights
{
	pass P0
	{
		vertexShader = compile vs_3_0 VS_Main();//Vertex shader
		pixelshader = compile ps_3_0 PS_Main();//Pixel shader
	}
}