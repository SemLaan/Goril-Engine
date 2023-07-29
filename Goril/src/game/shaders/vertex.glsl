#version 460 core


layout (location = 0) in vec4 v_position;
  
//out vec4 vertexColor;

void main()
{
    gl_Position = v_position;
    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}