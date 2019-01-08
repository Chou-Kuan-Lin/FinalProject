#version 330 core
out vec4 FragColor;
 
in vec2 TexCoords;
in vec3 FragPos;  
in vec3 Normal;  

uniform sampler2D exit_texture;

void main()
{
	FragColor = texture(exit_texture, TexCoords);
}