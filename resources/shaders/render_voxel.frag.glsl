#version 430

in vec4 g_voxel_coords;

layout(binding = 0, rgba32f) uniform image3D u_voxel;

void main()
{
	vec4 color = imageLoad(u_voxel, ivec3(g_voxel_coords.xyz));
	if (color.a <= 1)
	{
		discard;
	}
	gl_FragColor = color;
}
