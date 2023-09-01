#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D displacementmap;
uniform float displacement_coefficient;

void main()
{
    // TODO HW8 - 0_displacement_normal | calculate displacement
    //vec3 aBitangent=cross(aNormal,aTangent);
    float height =  texture(displacementmap, aTexCoord).r;    
    height=displacement_coefficient*(-1+2*height);
    vec4 worldPos = model * vec4(aPos+aNormal*height, 1.0);
    vs_out.WorldPos = worldPos.xyz / worldPos.w;
	
    vs_out.TexCoord = aTexCoord;
	
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	
    // TODO HW8 - 0_displacement_normal | calculate TBN
    
    //vec3 N = normalize(normalMatrix * aNormal);
    //vec3 N=normalize(vec3(model*at));
    //vec3 T = vec3(0);
	//vec3 B = vec3(0);
    
   vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
   vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
   T=normalize(T-dot(T,N)*N);
   vec3 B = cross(T,N);
	vs_out.TBN = (mat3(T, B, N));

    gl_Position = projection * view * worldPos;
}
