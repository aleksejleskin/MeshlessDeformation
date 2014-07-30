
struct input
{
	float3 currentPosition;
};

struct output
{
	float3 relativePosition;
};
#define GROUPSIZE 128
struct values
{
	float3 COM;
	float mass;
	float total;
};
//position of new vertecies.
//original com

StructuredBuffer<input> Buffer0 : register(t0);

RWStructuredBuffer<output> BufferOut : register(u0);
RWStructuredBuffer<values> BufferOut2 : register(u1);

groupshared float3 accumulator[1024];

[numthreads(1024, 1, 1)]
void CSMain(uint3 inp : SV_DispatchThreadID, uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID, uint gtidx : SV_GroupIndex)
{

	accumulator[gtidx] = Buffer0[inp.x].currentPosition;
	//Wait for all
	GroupMemoryBarrierWithGroupSync();

	[unroll]
	for (uint ix = GROUPSIZE >> 1; ix > 0; ix = ix >> 1) {
		//Check if we are in the bottom half of the part under consideration. 
		//If we are, add the corresponding top half value to it. If not, this
		//thread's job is finished. Unfortunately it cannot return yet because
		//of the call to synchronize
		if (gtidx < ix) {
			accumulator[gtidx] = (accumulator[gtidx] + accumulator[gtidx + ix]);
		}
		GroupMemoryBarrierWithGroupSync();
	}
	if (gtidx != 0) { return; }
	//Here we store the accumulated value of the group to the global texture
	BufferOut[gid.x].relativePosition = accumulator[0] ;



	//accumulation[dispatchThreadID.x] = Buffer0[threadIDInGroup.x].currentPosition.x;
	//for (unsigned int j = 0; j < 40; j++)
	//{
	//	if (BufferOut[0].relativePosition.x != 0 && BufferOut[0].relativePosition.y != 0 && BufferOut[0].relativePosition.z != 0)
	//	{
	//		unsigned int result;
	//		GroupMemoryBarrierWithGroupSync();
	//		BufferOut[0].relativePosition.x += accumulation[j];
	//		GroupMemoryBarrierWithGroupSync();
	//	}
	//}

}
