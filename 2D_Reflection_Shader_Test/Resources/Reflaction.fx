texture gLightTexture;
sampler2D LightMap = sampler_state
{
	Texture = (gLightTexture);
};

texture gDarkTexture;
sampler2D DarkMap = sampler_state
{
	Texture = (gDarkTexture);
};

texture gNoiseTexture;
sampler2D NoiseMap = sampler_state
{
	Texture = (gNoiseTexture);
};

float time;

float4 ps_main(float2 uv : TEXCOORD) : COLOR0
{
	float2 reverseuv = uv;
	float4 noiseTexture = tex2D(NoiseMap, float2(uv.x - time, uv.y));
	float4 lightTexture = tex2D(LightMap, uv + noiseTexture.r);
	float4 darkTexture = tex2D(DarkMap, uv);

	lightTexture.a = darkTexture.a;

	return lightTexture;
}

technique
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		PixelShader = compile ps_2_0 ps_main();
	}
};