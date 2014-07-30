#include "ModelManager.h"

ModelManager::ModelManager()
{
	m_errorModel.filename = "ERROR";
}

ModelManager::~ModelManager()
{
}

bool ModelManager::LoadContent(DxGraphics* dx)
{
	//Go through the whole Data/Model dir
	HANDLE hFind;
	WIN32_FIND_DATA findFileData;

	hFind = FindFirstFile("Data/Models/*.*", &findFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindNextFile(hFind, &findFileData); // gets past the folder dir
		FindNextFile(hFind, &findFileData); // gets past the subfolder dir

		do
		{
			LoadModels(dx, findFileData.cFileName);

		} while (FindNextFile(hFind, &findFileData));

		//Close the HANDLE
		FindClose(hFind);
	}
	return true;
}

void ModelManager::UnloadContent()
{
	for (unsigned int m = 0; m < (unsigned)m_models.size(); m++)
	{
		m_models[m].Release();
	}
}

Model& ModelManager::GetModel(string filename)
{
	for (unsigned int m = 0; m < (unsigned)m_models.size(); m++)
	{
		if (m_models[m].filename == filename)
		{
			return m_models[m];
		}
	}
	return m_errorModel;
}

bool ModelManager::LoadModels(DxGraphics* dx, string filename)
{
	//Create the new Model
	Model newModel;

	// Clear all the vectors ready for this load
	m_verticesPos.clear();
	m_verticesPosNor.clear();
	m_verticesPosNorTex.clear();
	m_verticesPosTex.clear();
	m_indicies.clear();

	//Load File
	ifstream file;
	string line;
	bool done = false;

	file.open("Data/Models/" + filename, ios::in);

	//assign the models filename
	newModel.filename = filename;

	if (file.is_open())
	{
		while (!done)
		{
			getline(file, line);

			// Get the vertex type
			if ((line[0] == 'V') && (line[1] == 'T'))
			{
				sscanf_s(line.c_str(), "VT %i", &newModel.vertexType);
			}
			// Get the vertex count
			else if ((line[0] == 'V') && (line[1] == 'C'))
			{
				sscanf_s(line.c_str(), "VC %i", &newModel.vertexCount);
			}
			// Get the index count
			else if ((line[0] == 'I') && (line[1] == 'C'))
			{
				sscanf_s(line.c_str(), "IC %i", &newModel.indexCount);
			}
			// Get the vertex
			else if (line[0] == 'V')
			{
				LoadVertex(newModel.vertexType, line);
			}
			// Get the index
			else if (line[0] == 'I')
			{
				int index = 0,
					index1 = 0,
					index2 = 0;

				sscanf_s(line.c_str(), "I %i %i %i", &index, &index1, &index2);
				m_indicies.push_back(index);
				m_indicies.push_back(index1);
				m_indicies.push_back(index2);
			}
			else if (line == "END")
			{
				done = true;
			}
		}

		if (!LoadBuffers(dx, newModel))
		{
			return false;
		}
	}
	else
	{
		//file does not exist
	}

	// Push back the new model
	m_models.push_back(newModel);

	return true;
}

bool ModelManager::LoadBuffers(DxGraphics* dx, Model& model)
{
	//<remarks> This is commented due to the need for sepereate buffers
	// atm the reliance on remapping buffers is causing problems
	// will come back to this after the deadline and find a better way </remarks>

	////Create the Vertex Buffer Desc
	//D3D11_BUFFER_DESC vbd;
	//vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	////Create the correct bytewidth dependant on the vertexType + set the stride bytewidth
	switch (model.vertexType)
	{
		case VERTEX_POS:
		//	vbd.ByteWidth = sizeof(VertexPos) * m_verticesPos.size();
		//	model.stride = sizeof(VertexPos);
		//	model.offset = 0;
		model.verticesPos = m_verticesPos;
		break;
	case VERTEX_POS_NOR:
		//	vbd.ByteWidth = sizeof(VertexPosNor) * m_verticesPosNor.size();
		//	model.stride = sizeof(VertexPosNor);
		//	model.offset = 0;
		model.verticesPosNor = m_verticesPosNor;
		break;
	case VERTEX_POS_TEX:
		//	vbd.ByteWidth = sizeof(VertexPosTex) * m_verticesPosTex.size();
		//	model.stride = sizeof(VertexPosTex);
		//	model.offset = 0;
		model.verticesPosTex = m_verticesPosTex;
		break;
	case VERTEX_POS_NOR_TEX:
		//	vbd.ByteWidth = sizeof(VertexPosNorTex) * m_verticesPosNorTex.size();
		//	model.stride = sizeof(VertexPos);
		//	model.offset = 0;
		model.verticesPosNorTex = m_verticesPosNorTex;
		break;
	default:
		break;
	}

	//Assign the models index list
	model.indicieList = m_indicies;

	//vbd.Usage = D3D11_USAGE_DYNAMIC;
	//vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//vbd.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA vInitData;

	//switch(model.vertexType)
	//{
	//case VERTEX_POS:
	//	vInitData.pSysMem = &m_verticesPos[0];
	//	break;
	//case VERTEX_POS_NOR:
	//	vInitData.pSysMem = &m_verticesPosNor[0];
	//	break;
	//case VERTEX_POS_TEX:
	//	vInitData.pSysMem = &m_verticesPosTex[0];
	//	break;
	//case VERTEX_POS_NOR_TEX:
	//	vInitData.pSysMem = &m_verticesPosNorTex[0];
	//	break;
	//default:
	//	break;
	//}

	//HR(dx->GetDevice()->CreateBuffer(&vbd, &vInitData, &model.vBuffer));

	//D3D11_BUFFER_DESC ibd;
	//ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//ibd.ByteWidth = sizeof(int) * model.indexCount;
	//ibd.Usage = D3D11_USAGE_DEFAULT;
	//ibd.CPUAccessFlags = 0;
	//ibd.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA iInitData;
	//iInitData.pSysMem = &m_indicies[0];

	//HR(dx->GetDevice()->CreateBuffer(&ibd, &iInitData, &model.iBuffer));

	return true;
}

void ModelManager::LoadVertex(int vertexType, string line)
{
	switch (vertexType)
	{
	case VERTEX_POS:
	{
					   VertexPos newVertex;

					   sscanf_s(line.c_str(), "V %f, %f, %f, C %f, %f, %f, %f",
						   &newVertex.position.x,
						   &newVertex.position.y,
						   &newVertex.position.z,
						   &newVertex.colour.x,
						   &newVertex.colour.y,
						   &newVertex.colour.z,
						   &newVertex.colour.w);

					   m_verticesPos.push_back(newVertex);
	}
		break;
	case VERTEX_POS_NOR:
	{
						   VertexPosNor newVertex;
						   float discard = 0.0f; //bad coding by me

						   sscanf_s(line.c_str(), "V %f, %f, %f, C %f, %f, %f, %f, N %f, %f, %f",
							   &newVertex.position.x,
							   &newVertex.position.y,
							   &newVertex.position.z,
							   &newVertex.colour.x,
							   &newVertex.colour.y,
							   &newVertex.colour.z,
							   &discard,
							   &newVertex.normal.x,
							   &newVertex.normal.y,
							   &newVertex.normal.z);

						   m_verticesPosNor.push_back(newVertex);
	}
		break;
	case VERTEX_POS_TEX:
	{
						   VertexPosTex newVertex;

						   sscanf_s(line.c_str(), "V %f, %f, %f, T %f, %f",
							   &newVertex.position.x,
							   &newVertex.position.y,
							   &newVertex.position.z,
							   &newVertex.texture.x,
							   &newVertex.texture.y);

						   m_verticesPosTex.push_back(newVertex);
	}
		break;
	case VERTEX_POS_NOR_TEX:
	{
							   VertexPosNorTex newVertex;

							   sscanf_s(line.c_str(), "V %f, %f, %f, N %f, %f, %f, T %f, %f",
								   &newVertex.position.x,
								   &newVertex.position.y,
								   &newVertex.position.z,
								   &newVertex.normal.x,
								   &newVertex.normal.y,
								   &newVertex.normal.z,
								   &newVertex.texture.x,
								   &newVertex.texture.y);

							   m_verticesPosNorTex.push_back(newVertex);
	}
		break;
	default:
		break;
	};
}