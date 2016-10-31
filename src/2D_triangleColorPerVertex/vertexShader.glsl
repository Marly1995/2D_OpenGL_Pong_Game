#version 330
in vec2 position;
in vec4 vertexColor;
out vec4 fragmentColor;

void main()
{
	 gl_Position = vec4(position, 0.0, 1.0);
	 fragmentColor = vertexColor;
}
