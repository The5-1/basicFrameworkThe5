/*
https://thebookofshaders.com/10/
*/

#version 330
layout(location = 0)  out vec4 out0; // color 

uniform float time;
uniform vec2 resolution;
uniform float density;
uniform int type;

/* ************************************************************************
	0. Random Number
************************************************************************ */
float random (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))*43758.5453123);
}

/* ************************************************************************
	1. Simple noise
************************************************************************ */
vec3 simpleNoise(){
	vec2 st = (gl_FragCoord.xy/resolution.xy);
	float rnd = random( st );

	return vec3(rnd);
} 

/* ************************************************************************
	2. Rescaled Noise
************************************************************************ */

vec3 rescaleSimpleNoise(){
	vec2 st = gl_FragCoord.xy/resolution.xy;

    st *= 10.0; // Scale the coordinate system by 10
    vec2 ipos = floor(st);  // get the integer coords
    vec2 fpos = fract(st);  // get the fractional coords

    // Assign a random value based on the integer coord
    return vec3(random( ipos )); 
} 

/* ************************************************************************
	3. Truchet Noise
************************************************************************ */

vec2 truchetPattern(in vec2 _st, in float _index){
    _index = fract(((_index-0.5)*2.0));
    if (_index > 0.75) {
        _st = vec2(1.0) - _st;
    } else if (_index > 0.5) {
        _st = vec2(1.0-_st.x,_st.y);
    } else if (_index > 0.25) {
        _st = 1.0-vec2(1.0-_st.x,_st.y);
    }
    return _st;
}

vec3 truchetMaze(){
	 vec2 st = gl_FragCoord.xy/resolution.xy;
     st *= 10.0;
     st = (st-vec2(5.0))*(abs(sin(time*0.2))*5.);
     st.x += time*3.0;

    vec2 ipos = floor(st);  // integer
    vec2 fpos = fract(st);  // fraction

    vec2 tile = truchetPattern(fpos, random( ipos ));

    float color = 0.0;

    // Maze
    color = smoothstep(tile.x-0.3,tile.x,tile.y)-
            smoothstep(tile.x,tile.x+0.3,tile.y);
	
	return vec3(color);
}

vec3 truchetCircles(){
	vec2 st = gl_FragCoord.xy/resolution.xy;
    st *= 10.0;
    // st = (st-vec2(5.0))*(abs(sin(u_time*0.2))*5.);
    // st.x += u_time*3.0;

    vec2 ipos = floor(st);  // integer
    vec2 fpos = fract(st);  // fraction

    vec2 tile = truchetPattern(fpos, random( ipos ));

    float color = 0.0;

    // Circles
    color = (step(length(tile),0.6) -
              step(length(tile),0.4) ) +
             (step(length(tile-vec2(1.)),0.6) -
              step(length(tile-vec2(1.)),0.4) );
	
	return vec3(color);
}

vec3 truchetDoubleTriangles(){
	vec2 st = gl_FragCoord.xy/resolution.xy;
    st *= 10.0;
    // st = (st-vec2(5.0))*(abs(sin(u_time*0.2))*5.);
    // st.x += u_time*3.0;

    vec2 ipos = floor(st);  // integer
    vec2 fpos = fract(st);  // fraction

    vec2 tile = truchetPattern(fpos, random( ipos ));

    float color = 0.0;

    // Truchet (2 triangles)
    color = step(tile.x,tile.y);
	
	return vec3(color);
}

/* ************************************************************************
	4. Perlin Noise
************************************************************************ */
float cubicSmoothstep(float f){
	return f * f * (3.0 - 2.0 * f ); // custom cubic curve
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners porcentages
    return mix(a, b, u.x) + 
            (c - a)* u.y * (1.0 - u.x) + 
            (d - b) * u.x * u.y;
}

vec3 valueNoise(){
	 vec2 st = gl_FragCoord.xy/resolution.xy;

    // Scale the coordinate system to see
    // some noise in action
    vec2 pos = vec2(st * density);

    // Use the noise function
    float n = noise(pos);

	return vec3(n);
}

/* //////////////////////////////////////////////////////////////////////
	Main-Function
  ////////////////////////////////////////////////////////////////////// */ 

void main()
{
	
	vec3 color;
	switch (type) {
		case 0: color =  simpleNoise(); break;

		case 1: color =  rescaleSimpleNoise(); break;

		case 2: color =  truchetMaze(); break;

		case 3: color = truchetCircles(); break;

		case 4: color = truchetDoubleTriangles(); break;

		case 5: color = valueNoise(); break;
	}


	out0 = vec4(color, 1.0);
}
