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
uniform extern float3 gLightDirection;
uniform extern float3 gEyeVecW;
uniform extern float3 sunLightVecW = normalize(float3(0.0f, 20.0f, -20.0f));

// light sources
uniform float3 directionalLightColor = float3(0.3f, 0.3f, 0.3f); // directional light
uniform float3 pointLightColor = float3(0.5f, 0.5f, 0.5f); // point light
uniform float3 spotLightColor = float3(1.0f, 1.0f, 1.0f); // spotlight
uniform float3 ambientLightColor = float3(0.1f, 0.1f, 0.1f); // ambient light
uniform float3 areaLightColor = float3(0.1f, 0.1f, 0.1f); // area light

// materials - per object, light reflections
uniform float3 ambientReflection = float3(0.5f, 0.5f, 0.5f); // ambient light color reflected
uniform float3 diffuseReflection = float3(0.5f, 0.5f, 0.5f); // diffuse light color reflected
uniform float3 specularReflection = float3(0.8f, 0.8f, 0.8f); // specular light color reflected

uniform float shininess = 128; // (shininess) specular power factor

//output struct
struct OutputVertexStruct
{
    float4 posH : POSITION0; //transformed homogeneous clip space output vector
    float2 tex0 : TEXCOORD0; //texture coords to pixel shader
    float2 tex1 : TEXCOORD1; //texture coords of blend map
    float3 normal : TEXCOORD2; // the normal in world space
    float3 toEye : TEXCOORD3; // the vector from the vertex to the eye in world space
    float3 lightW : TEXCOORD4; // the vector from the vertex to the light
    float attenuation : TEXCOORD6; // attenuation scalar
};

// function prototypes
float3 directionalLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR;
float3 pointLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR;
float3 spotLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR;

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


// vertex Shader
OutputVertexStruct transformVertexShader(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0 : TEXCOORD0)
{
	//zero struct
	OutputVertexStruct outVS = (OutputVertexStruct)0;

	// store the world space position of the vertex
    float3 posW = mul(float4(posL, 1.0f), gWorld);
    
	// vector from vertex to eye
    outVS.toEye = normalize(gEyeVecW - posW);
	
	// vector from vertex to light
    outVS.lightW = normalize(gLightVecW - posW);
	
	// attenuation
    float distanceBetween = distance(gLightVecW, posW);
    outVS.attenuation = 0 + 0.03f * distanceBetween + 0; // a0 + a1*d + a2*d*d, for triplets: 0,1,0 - linear attenuation, 0,0,1 - (real) inverse square
	
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

// pixel Shader
float4 transformPixelShader(OutputVertexStruct input) : COLOR
{
    float3 color = float3(0.0f, 0.0f, 0.0f);
	float3 texColor;
    float3 normal;
	
	if (gIsMesh)
	{
        texColor = tex2D(MeshSampler, input.tex1);
		texColor += gMeshColor; // if there was no texture then use mesh color
	}
	else if (gIsFloor)
	{
        float3 groundColor = tex2D(GroundSampler, input.tex0);
        float3 stoneColor = tex2D(StoneSampler, input.tex0);
        float3 grassColor = tex2D(GrassSampler, input.tex0);
        float3 blendColor = tex2D(BlendSampler, input.tex1);

        float blendRGBSum = blendColor.r + blendColor.g + blendColor.b;
        float scaleR = blendColor.r / blendRGBSum;
        float scaleG = blendColor.g / blendRGBSum;
        float scaleB = blendColor.b / blendRGBSum;

		texColor = groundColor * scaleR + stoneColor * scaleG + grassColor * scaleB;
    }
	else if (gIsBoundingBox)
	{
		return float4(1, 0, 0, 0.3);
	}
	else
	{
        texColor = tex2D(BoxSampler, input.tex0);
    }
	
    input.normal = normalize(input.normal); // interpolated normals can become un-normalized
    input.lightW = normalize(input.lightW); // normalize the light vector
    input.toEye = normalize(input.toEye); // normalize camera vector
	
	// directional light
    color += directionalLightCalculations(input, texColor);
	
	// point light
    color += pointLightCalculations(input, texColor);
	
	// spotlight
    color += spotLightCalculations(input, texColor);
	
	// area light
	
	// add the ambient reflection
    float3 ambient = ambientLightColor * (ambientReflection * texColor.rgb); // ambient light with material texture color
    color += ambient;
	
    return float4(color, 1.0f);
}

technique transformTech
{
	pass P0
	{
		//specify vertex and pixel shader associated with this pass
		vertexShader = compile vs_2_0 transformVertexShader();
		pixelShader = compile ps_3_0 transformPixelShader();

		//set render and device states associated with this pass
	}
}

float3 directionalLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR
{
	// specular
    float3 specularReflVec = reflect(-sunLightVecW, input.normal); // specular reflection vector
    float intensitySpec = pow(max(dot(normalize(specularReflVec), input.toEye), 0.0f), shininess); // intensity of the reflected specular light scalar
    float3 specular = (intensitySpec * directionalLightColor) * (specularReflection * texColor).rgb; // reflected specular light with material texture color
	
	// diffuse
    float intensityDiff = max(dot(sunLightVecW, input.normal), 0.0f); // lambert, intensity of the reflected diffuse light scalar
    float3 diffuse = (intensityDiff * directionalLightColor) * (diffuseReflection * texColor).rgb; // the diffuse reflection light with material texture color
    
	// compute final color
    texColor = specular + diffuse; // add diffuse, specular
	
    return texColor;
}

float3 pointLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR
{
	// specular
    float3 specularReflVec = reflect(-input.lightW, input.normal); // specular reflection vector
    float intensitySpec = pow(max(dot(normalize(specularReflVec), input.toEye), 0.0f), shininess); // intensity of the reflected specular light scalar
    float3 specular = (intensitySpec * pointLightColor) * (specularReflection * texColor).rgb; // reflected specular light with material texture color
	
	// diffuse
    float intensityDiff = max(dot(input.lightW, input.normal), 0.0f); // lambert, intensity of the reflected diffuse light scalar
    float3 diffuse = (intensityDiff * pointLightColor) * (diffuseReflection * texColor).rgb; // the diffuse reflection light with material texture color
    
	// compute final color
    texColor = (specular + diffuse) / input.attenuation; // add diffuse, specular, together with attenuation
	
    return texColor;
}

float3 spotLightCalculations(OutputVertexStruct input, float3 texColor) : COLOR
{
	// spotlight
    float3 spotlightReflVec = reflect(-input.lightW, gLightDirection); // spotlight reflection vector
    float intensitySpot = pow(max(dot(normalize(spotlightReflVec), input.toEye), 0.0f), shininess); // intensity of the reflected spotlight scalar
    float3 spotlight = (intensitySpot * spotLightColor); // reflected spotlight
	
	// specular
    float3 specularReflVec = reflect(-input.lightW, input.normal); // specular reflection vector
    float intensitySpec = pow(max(dot(normalize(specularReflVec), input.toEye), 0.0f), shininess); // intensity of the reflected specular light scalar
    float3 specular = (intensitySpec * spotLightColor) * (specularReflection * texColor).rgb; // reflected specular light with material texture color
	
	// diffuse
    float intensityDiff = max(dot(input.lightW, input.normal), 0.0f); // lambert, intensity of the reflected diffuse light scalar
    float3 diffuse = (intensityDiff * spotLightColor) * (diffuseReflection * texColor).rgb; // the diffuse reflection light with material texture color
    
	// compute final color
    texColor = (specular + diffuse) / input.attenuation; // add diffuse, specular, together with attenuation
    texColor = spotlight * (texColor);
	
    return texColor;
}
