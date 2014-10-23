#version 330   

//-------------------------------------
// values sent from the vertex shader

in vec2 TexCoord;
in vec4 vColor;

//-------------------------------------

// output color
out vec4 outColor;

// textures
uniform sampler2D DiffuseTexture;

void main() 
{ 
	outColor = texture2D( DiffuseTexture, TexCoord );
	outColor.a = 1;
}