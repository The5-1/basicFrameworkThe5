#version 330

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform sampler2D sat;
uniform sampler2D color;
uniform sampler2D depth;

uniform bool manualBlur;
uniform bool debugSat;
uniform float blur;
uniform float focalPlane;
uniform ivec2 res;

uniform float time;

const bool autofocus = true;

in vec2 tc;

layout(location = 0)  out vec4 fragColor0; // color 

vec4 sat_lookup(sampler2D tex, ivec2 min, ivec2 max) {
	// TODO: implement lookup.
	// NOTE: handle border cases.
	// NOTE: the area bounds are understood to be inclusive.
	max = ivec2(clamp(max.x, 0, res.x-1), clamp(max.y, 0, res.y-1));
	min = ivec2(clamp(min.x, 0, res.x-1), clamp(min.y, 0, res.y-1));
	return (texelFetch(tex, min, 0)
            + texelFetch(tex, max + ivec2(1, 1), 0)
            - texelFetch(tex, min + ivec2(0, max.y-min.y+1), 0)
            - texelFetch(tex, min + ivec2(max.x-min.x+1, 0), 0))/((max.x-min.x+1)*(max.y-min.y+1));
}


vec3 VSPositionFromDepth(float depth)
{
    vec2 vUV = tc;
    vec3 ndc = vec3(vUV*2.0-1.0, depth*2.0-1.0);
    vec4 v0 = inverse(projMatrix)*vec4(ndc, 1);
    vec3 reconViewPos = v0.xyz/v0.w;
    vec3 reconWorldPos = (inverse(viewMatrix) * v0).xyz;
	return reconWorldPos;
}


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float fractal(vec2 uv, vec2 center, float scale, int iter)
{
	vec2 z, c;

    c.x = 1.3333 * (uv.x - 0.5) * scale - center.x;
    c.y = (uv.y - 0.5) * scale - center.y;

    int i;
    z = c;
    for(i=0; i<iter; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (z.y * z.x + z.x * z.y) + c.y;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }

	return i/100.0;
}

float fractal_julia(vec2 uv,vec2 center, int iter)
{
    vec2 z;
    z.x = 3.0 * (uv.x - 0.5);
    z.y = 2.0 * (uv.y - 0.5);

    int i;
    for(i=0; i<iter; i++) {
        float x = (z.x * z.x - z.y * z.y) + center.x;
        float y = (z.y * z.x + z.x * z.y) + center.y;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }

	return i/100.0;
}


vec4 portal_effect()
{
	vec2 coords = tc;
	vec2 circleCoords = (coords-vec2(0.5))*2.0;
	circleCoords.x *= 2.0;
	circleCoords.y *= 1.2;

	float circleBase = length(circleCoords); //basis für den kreis

	float AA = 5.0;
	float circle = (circleBase-1.0+1/AA)*AA; //antialaiased aber verkerht
	circle = 1.0-circle;
	circle = clamp(circle,0.0,1.0);

	float angle = time*10.0-circleBase*20.0;
	mat2 rot = mat2(cos(angle),-sin(angle),sin(angle),cos(angle));

	float angle2 = -time*10.0-circleBase*20.0;
	mat2 rot2 = mat2(cos(angle2),-sin(angle2),sin(angle2),cos(angle2));

	float swirl = abs(rot*circleCoords).x;
	float swirl2 = pow(abs(1.0-rot2*circleCoords).y,.5);
	float swirl3 = pow(swirl,10.0)*10.0;

	float ripples = abs(cos(time*10.0-circleBase*20.0));
	float ripples2 = pow(ripples,8.0);

	fragColor0 = vec4(ripples);
	//return;

	float glowStrength = 4.0;
	float glow = 1.0-abs(1.0-(circleBase*glowStrength - glowStrength + 1.0));
	glow = clamp(glow,0.0,1.0);
	float glow2 = pow(glow,4.0);

	vec3 color = vec3(0.0,1.0,0.7);

	float julia = fractal_julia(coords*0.5,vec2(rot*circleCoords*0.1-0.55),64);

	return vec4(vec3(color),1.0)*(circle*(swirl+swirl3*2.0-ripples2*0.5+ julia)*ripples+glow+glow2)*0.5;
}


void main() {

	int r = 0;
	float fragmentDepth = texture(color, tc).a;
	float focusDepth = texture(color, vec2(0.5,0.5)).a;
	float focusDepthNE = texture(color, vec2(0.51,0.51)).a;
	float focusDepthSE = texture(color, vec2(0.51,0.49)).a;
	float focusDepthSW = texture(color, vec2(0.49,0.49)).a;
	float focusDepthNW = texture(color, vec2(0.49,0.51)).a;
	focusDepth = (focusDepth+focusDepthNE+focusDepthSE+focusDepthSW+focusDepthNW)*0.2;
	
	if (manualBlur)
		r = int(blur);
	else {
		float coc = 2.0*abs(fragmentDepth - focusDepth)/fragmentDepth;
		r = min(int(coc), 20);
	}

	vec3 col = sat_lookup(sat, ivec2(gl_FragCoord.xy - vec2(r,r)), ivec2(gl_FragCoord.xy + vec2(r,r))).rgb;
	
	fragColor0 = vec4(col, 1.0);
	//fragColor0 = vec4(texture(color,tc).rgb, 1.0);
	//fragColor0 = vec4(VSPositionFromDepth(linearDepth), 1.0);
	
	gl_FragDepth = texture(depth, tc).r;

}
