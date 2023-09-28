#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 squareVertices;
layout(location = 1) in vec4 xyzs; // Position of the center of the particule and size of the square
layout(location = 2) in vec4 color; 

// Output data ; will be interpolated for each fragment.
out vec2 UVC;
out vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec2 uvcol[120];
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)
void main()
{
	int uv_index = int(xyzs.w); // because we encoded it this way.
	uv_index=uv_index*4;
	uv_index=uv_index+gl_VertexID%4;
	vec3 particleCenter_wordspace = xyzs.xyz;
	
	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ CameraRight_worldspace * squareVertices.x*0.25f;
		+ CameraUp_worldspace * squareVertices.y*0.25f;

	// Output position of the vertex
	gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);

	// UV of the vertex. No special space for this one.
	//UVC = uvcol[uv_index];
	UVC = squareVertices.xy + vec2(0.5, 0.5);
	//particlecolor = color;
}

