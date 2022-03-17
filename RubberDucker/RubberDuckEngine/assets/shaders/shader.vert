#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in mat4 inModelMatrix;
//		location = 4
//		location = 5
//		location = 6

layout (binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
} ubo;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;

void main()
{
	mat4 model = inModelMatrix;
	gl_Position = ubo.projection * ubo.view * model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}