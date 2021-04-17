/*
FX file
Transformations and device settings
*/

//world/view/projection matrix
//global shader variable
//effect parameter
uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldViewProjection;
uniform extern float4x4 gWorldInverseTranspose;

uniform extern texture gBoxTex;
uniform extern texture gGroundTex;
uniform extern texture gGrassTex;
uniform extern texture gStoneTex;
uniform extern texture gBlendTex;
uniform extern texture gMeshTex;

uniform extern bool gIsFloor;
uniform extern bool gIsMesh;
uniform extern bool gIsBoundingBox;

uniform extern float2 gOffsetXY;
uniform extern float4 gMeshColor;
uniform extern float3 gLightVecW;
uniform extern float3 gEyeVecW;

uniform float3 directionalLightColor = float3(0.5f, 0.5f, 0.5f); // directional light color
uniform float3 ambientLightColor = float3(0.1f, 0.1f, 0.1f); // ambient light color
uniform float3 specularLightColor = float3(1.0f, 1.0f, 1.0f); // specular light color

uniform float specPower = 256; // specular power factor

//sampler for mesh
sampler MeshSampler = sampler_state
{
	Texture = <gMeshTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//sampler for box
sampler BoxSampler = sampler_state
{
	Texture = <gBoxTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//sampler for ground
sampler GroundSampler = sampler_state
{
	Texture = <gGroundTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//sampler for grass
sampler GrassSampler = sampler_state
{
	Texture = <gGrassTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//sampler for stone
sampler StoneSampler = sampler_state
{
	Texture = <gStoneTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//sampler for blend map
sampler BlendSampler = sampler_state
{
	Texture = <gBlendTex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

//output struct
struct OutputVertexStruct
{
	float4 posH : POSITION0; //transformed homogeneous clip space output vector
	float2 tex0 : TEXCOORD0; //texture coords to pixel shader
	float2 tex1 : TEXCOORD1; //texture coords of blend map
	float3 normal : TEXCOORD2; // the normal in world space
    float3 toEye : TEXCOORD3; // the vector from the vertex to the eye in world space
};


//Vertex Shader
//position coords - posL local space, normalL local space, texture coords - tex0
OutputVertexStruct transformVertexShader(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0 : TEXCOORD0)
{
	//zero struct
	OutputVertexStruct outVS = (OutputVertexStruct)0;

	// store the world space position of the vertex
    float3 posW = mul(float4(posL, 1.0f), gWorld);
    outVS.toEye = normalize(gEyeVecW - posW);
	
	// transform normal vector to world space (inverse transpose because normals can become un-orthogonal without)
	float3 normalW = mul(float4(normalL, 1.0f), gWorldInverseTranspose).xyz;
	// normalize the normal vector
	outVS.normal = normalize(normalW);

	//transform to homogeneous clip space
	outVS.posH = mul(float4(posL, 1.0f), gWorldViewProjection);

	//pass color as is
	if (gIsFloor)
	{
		outVS.tex0 = tex0 * 10; //scale floor texture by 16

		outVS.tex0 += gOffsetXY;//animate the texture

		outVS.tex1 = tex0; //leave blend map as [0,1]
	}
	else
	{
		outVS.tex0 = tex0;
		outVS.tex1 = tex0;
	}
	
	//return output
	return outVS;
}

//Pixel Shader
float4 transformPixelShader(float2 tex0 : TEXCOORD0, float2 tex1 : TEXCOORD1, float3 normal : TEXCOORD2, float3 toEye : TEXCOORD3) : COLOR
{
	float3 texColor;
	float3 groundColor;
	float3 stoneColor;
	float3 grassColor;
	float3 blendColor;

	float scaleR;
	float scaleG;
	float scaleB;

	if (gIsMesh)
	{
		texColor = tex2D(MeshSampler, tex1);
		texColor += gMeshColor;
	}
	else if (gIsFloor)
	{
		groundColor = tex2D(GroundSampler, tex0);
		stoneColor = tex2D(StoneSampler, tex0);
		grassColor = tex2D(GrassSampler, tex0);
		blendColor = tex2D(BlendSampler, tex1);

		scaleR = blendColor.r / (blendColor.r + blendColor.g + blendColor.b);
		scaleG = blendColor.g / (blendColor.r + blendColor.g + blendColor.b);
		scaleB = blendColor.b / (blendColor.r + blendColor.g + blendColor.b);

		texColor = groundColor * scaleR + stoneColor * scaleG + grassColor * scaleB;
	}
	else if (gIsBoundingBox)
	{
		return float4(1, 0, 0, 0.3);
	}
	else
	{
		texColor = tex2D(BoxSampler, tex0);
	}
	
    normal = normalize(normal); // interpolated normals can become un-normalized
	
	// specular
    float3 specularReflVec = reflect(-gLightVecW, normal); // specular reflection vector
    float intensitySRL = pow(max(dot(toEye, specularReflVec), 0.0f), specPower); // intensity of the reflected specular light scalar
	
	// diffuse
    float intensityDRL = max(dot(normalize(gLightVecW), normal), 0.0f); // lambert, intensity of the reflected diffuse light scalar
	
    float3 ambient = (texColor.rgb * ambientLightColor); // ambient light with material texture color
    float3 diffuse = (intensityDRL * directionalLightColor) * (texColor).rgb; // the diffuse reflection light with material texture color
    float3 specular = (intensitySRL * specularLightColor) * (texColor).rgb; // specular reflected light
    //texColor = specular + diffuse + ambient; // add ambient, diffuse, and specular light together
    texColor = diffuse + ambient; // add ambient, diffuse, and specular light together
	
	return float4(texColor, 1.0f);
}

technique transformTech
{
	pass P0
	{
		//specify vertex and pixel shader associated with this pass
		vertexShader = compile vs_2_0 transformVertexShader();
		pixelShader = compile ps_2_0 transformPixelShader();

		//set render and device states associated with this pass
	}
}
