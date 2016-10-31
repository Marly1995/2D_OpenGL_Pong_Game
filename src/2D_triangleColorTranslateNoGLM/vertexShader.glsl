#version 330
in vec2 position;
in vec4 vertexColor;
out vec4 fragmentColor;
uniform vec2 translationVector = vec2(-0.5, -0.5);

void main()
{
	 gl_Position = vec4(position + translationVector, 0.0, 1.0);
	 fragmentColor = vertexColor;
}
