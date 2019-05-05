//#pragma OPENCL EXTENSION cl_intel_printf : enable

__kernel void update_volume(__global float* renderable, __global float* vertices, __constant float* cameraPos, __constant float* cameraDir, __constant float* separationDistance,
							__global float* tetraTransforms)
{
    // Get the index of the current element
    int i = get_global_id(0);
	
	int volumeIndex = i;
	int volumeSize = 12;
	
	bool render = true;
//	printf("%f, %f, %f, %f\n", tetraTransforms[0], tetraTransforms[1], tetraTransforms[2], tetraTransforms[3]);
	for(int j = 0; j < volumeSize; j++)
	{
		float transformedCoordinates[4] = {0.0, 0.0, 0.0, 0.0};
		float vertex[4] = {vertices[j * 6], vertices[j * 6 + 1], vertices[j * 6 + 2], 1.0};
		for(int k = 0; k < 4; k++)
		{
			for(int m = 0; m < 4; ++m)
			{
				transformedCoordinates[k] += tetraTransforms[16 * volumeIndex + m * 4 + k] * vertex[m];
			}
		}
//		printf("CHINCHILLA\n");
		// calculate distance from point to plane
		float distance = 0.0f;
		for(int k = 0; k < 3; k++)
		{
			distance += cameraDir[k] * (transformedCoordinates[k] - cameraPos[k]);
		}
		
		if(distance < separationDistance[0])
		{
			render = false;
			break;
		}
	}
	
	if(render)
	{
		renderable[volumeIndex] = 1.0f;
	}
	else
	{
		renderable[volumeIndex] = 0.0f;
	}
}