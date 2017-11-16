#version 330
  
layout(location = 0)  out vec4 out0; // color 

in vec2 TexCoords;
in vec3 NormalW;
in vec3 LightW;
in vec3 PosW;
in vec3 cameraPos;
in float depth;

uniform sampler2D texture_diffuse;

const float shinyness = 32.0;
const vec3 ambientCol = vec3(0.0, 0.0, 0.0); // grey
const vec3 specularCol = vec3(0.4, 0.4,0.4); // white
 
 
 
 //BRDF
const vec3 baseColor = vec3(1.0, 1.0, 1.0);
uniform float uMetallic;
uniform float uSubsurface;
uniform float uSpecular;
uniform float uRoughness;
uniform float uSpecularTint;
uniform float uAnisotropic;
uniform float uSheen;
uniform float uSheenTint;
uniform float uClearcoat;
uniform float uClearcoatGloss;
uniform float uLightIntensity;

uniform sampler2D uTexFlakes;
 
uniform float uTime;
 
 
const float PI = 3.14159265358979323846;

float sqr(float x) { return x*x; }

float SchlickFresnel(float u)
{
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR1(float NdotH, float a)
{
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}

float GTR2(float NdotH, float a)
{
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

vec3 mon2lin(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}


/*
vec3 BRDF_aniso( vec3 L, vec3 V, vec3 N, vec3 X, vec3 Y )
{
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    vec3 Cdlin = mon2lin(baseColor);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.

    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(uSpecular*.08*mix(vec3(1), Ctint, uSpecularTint), Cdlin, uMetallic);
    vec3 Csheen = mix(vec3(1), Ctint, uSheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on uRoughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * uRoughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on uRoughness
    float Fss90 = LdotH*LdotH*uRoughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

    // uSpecular
    float aspect = sqrt(1-uAnisotropic*.9);
    float ax = max(.001, sqr(uRoughness)/aspect);
    float ay = max(.001, sqr(uRoughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    // uSheen
    vec3 Fsheen = FH * uSheen * Csheen;

    // uClearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1,.001,uClearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

    return ((1/PI) * mix(Fd, ss, uSubsurface)*Cdlin + Fsheen)
        * (1-uMetallic)
        + Gs*Fs*Ds + .25*uClearcoat*Gr*Fr*Dr;
}
*/

vec3 BRDF( vec3 L, vec3 V, vec3 N)
{
	vec3 color = texture(texture_diffuse, TexCoords).rgb * baseColor;


    float NdotL = dot(N,L);	
    float NdotV = dot(N,V);
    //if (NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    vec3 Cdlin = mon2lin(color);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.

    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(uSpecular*.08*mix(vec3(1), Ctint, uSpecularTint), Cdlin, uMetallic);
    vec3 Csheen = mix(vec3(1), Ctint, uSheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on uRoughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * uRoughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on uRoughness
    float Fss90 = LdotH*LdotH*uRoughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

    // uSpecular
    float aspect = sqrt(1-uAnisotropic*.9);
    float Ds = GTR2(NdotH, uRoughness);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX(NdotL, uRoughness);
    Gs *= smithG_GGX(NdotV, uRoughness);

    // uSheen
    vec3 Fsheen = FH * uSheen * Csheen;

    // uClearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1,.001,uClearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

    vec3 brdf = ((1/PI) * mix(Fd, ss, uSubsurface)*Cdlin + Fsheen)
        * (1-uMetallic)
        + Gs*Fs*Ds*NdotL + .25*uClearcoat*Gr*Fr*Dr;
	return clamp(brdf * uLightIntensity,vec3(0.0),vec3(1.0));
}



vec3 blinn_phong_reflectance(vec3 N, vec3 L, vec3 V) {
	vec3 color = texture(texture_diffuse, TexCoords).rgb;

	vec3 H = normalize(L+V); // half way vector

	vec3 amb = ambientCol;

	vec3 diff = max(0.0, dot(L, N)) * color;
	vec3 spec = pow(max(0.0, dot(H, N)), shinyness) * specularCol;

	return amb + diff + spec;
}

void main() 
{ 
	vec3 L = normalize(-LightW);
	vec3 N = normalize(NormalW);
	vec3 V = normalize(PosW-cameraPos);
	
	vec3 phong = blinn_phong_reflectance(N, L, V);
	vec3 brdf = BRDF(-L,-V,N);
	
	
	float timer_sawtooth = mod(uTime*0.35,1.0);
	
	float sonar = 1.0-clamp(abs(timer_sawtooth-length(PosW-cameraPos)*0.01)*60.0,0.0,1.0);
	//sonar += 1.0-min(1.0,(timer_sawtooth-length(PosW-cameraPos)*0.01));
	sonar += 0.1;
	out0 = vec4(brdf*max(0.0,sonar), depth);
	out0 = vec4(brdf, depth);
}
