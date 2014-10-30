#version 150

in vec3 position;
in vec3 normal;

uniform vec3 lightAmbient;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 lightSpecular;	// vec3 or 4? 33333? why is it?

uniform vec3 cameraPosition;

void main()
{
	vec3 N = normalize( normal );				// normal
	vec3 L = normalize( -lightDirection );		// light direction

	vec3 R = reflect(-L, N);						// reflection
	vec3 E = normalize( cameraPosition - position);	// fuck

	// diffuse
	float d = max(0, dot(N, L));
	vec3 diffuse = lightColor * d;

	// ambient
	vec3 ambient = lightAmbient;

	// specular
	float s = pow (max(0, dot(E, R)), 128);
	vec3 specular = lightSpecular * s;

	gl_FragColor = vec4(ambient + diffuse + specular, 1);
}