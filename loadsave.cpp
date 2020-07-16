/*
 * loadsave.cpp
 *
 *  Created on: 23 мая 2020 г.
 *      Author: Serkin
 */

#include <iostream>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/rapidjson.h"
#include "error.h"
#include "graph.h"
#include "settings.h"
#include "informed.h"

void loadGraph(Graph& graph, const Settings& settings) {
	using namespace std;

	FILE * fd = fopen(settings.FilePath.c_str(), "rb");

	if (!fd){
		cerr << "Cannot load graph from file: " << settings.FilePath;
		exit(FATAL_ERROR_FILE_OPEN_FAILURE);
	}

	fseek(fd, 0, SEEK_END);                          // устанавливаем позицию в конец файла
	long lSize = ftell(fd);                            // получаем размер в байтах
	rewind (fd);                                       // устанавливаем указатель в конец файла

	char * buffer = (char*) malloc(sizeof(char) * lSize + 1); // выделить память для хранения содержимого файла
	if (buffer == NULL)
	{
	  fputs("Memory allocation error.", stderr);
	  exit(FATAL_ERROR_NO_MEMORY);
	}

	char *begin = buffer;
	int chunksize = 256;
	size_t result;
	do {
		result = fread(begin, 1, chunksize, fd);       // считываем файл в буфер
		begin += result;
	} while (result > 0);

	if (begin - buffer != lSize)
	{
	  fputs("Failed to read file", stderr);
		exit(FATAL_ERROR_FILE_READ_FAILURE);
	}

	buffer[lSize] = 0;

	//содержимое файла теперь находится в буфере
	if (settings.Verbose) puts(buffer);

	// завершение работы
	fclose (fd);

	rapidjson::Document doc;
	doc.ParseInsitu(buffer);
	if (!doc.IsArray()) {
		fputs("Invalid file format. The root element is expected to be array.", stderr);
		exit(FATAL_ERROR_FILE_READ_FAILURE);
	}

	//creating all vertices
	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) {
	    auto vertex_obj1 = itr->GetObject();
		if (!vertex_obj1.HasMember("name")) {
			fputs("Invalid file format. \"name\" element is expected in object, but not found", stderr);
			exit(FATAL_ERROR_NAME_ELEMENT_NOT_FOUND);
		}
		string *str1 = new string (vertex_obj1["name"].GetString());

		Vertex *vertex = addVertex(*str1, graph, settings);
		if (vertex == nullptr) {
			cerr << "Failed to add vertex.";
			exit(FATAL_ERROR_FAILED_TO_ADD_VERTEX);
		}
	}

	//creating edges
	for (rapidjson::Value::ConstValueIterator itv = doc.Begin(); itv != doc.End(); ++itv) {
	    auto vertex_obj2 = itv->GetObject();
	    string *str_from = new string (vertex_obj2["name"].GetString());
	    Vertex* from_vertex = findVertex(*str_from, graph);

	    if (!from_vertex) {
	    	cerr << "Vertex not found: " << *str_from;
			exit(FATAL_ERROR_FROM_VERTEX_NOT_FOUND);
	    }

	    if (vertex_obj2.HasMember("edges")) {
			auto edges = vertex_obj2["edges"].GetArray();
			for (rapidjson::Value::ConstValueIterator ite = edges.Begin(); ite != edges.End(); ++ite) {
				auto edge_obj = ite->GetObject();
				string *str_to = new string(edge_obj["to_vertex"].GetString());
				Vertex* to_vertex = findVertex(*str_to, graph);
			    if (!to_vertex) {
			    	cerr << "Vertex not found: " << *str_to;
					exit(FATAL_ERROR_TO_VERTEX_NOT_FOUND);
			    }
				double weight = edge_obj["weight"].GetDouble();
				if (!addEdge(from_vertex, to_vertex, weight, graph, settings)) {
					cerr << "Cannot add edge: " << *str_from << "->" << *str_to << endl;
					exit(FATAL_ERROR_FAILED_TO_ADD_EDGE);
				}
			}
		}
	}
	free (buffer);
}

int load2dGraph(Graph& graph, const Settings& settings) {
	Vertex *v2d[9][9];
	for (unsigned char y1 = 0; y1 < 9; y1++) {
		for (unsigned char x1 = 0; x1 < 9; x1++) {
			string name {"0:0"};
			name[0] = 0x30 + x1;
			name[2] = 0x30 + y1;
			Vertex *vertex = new Vertex2d(name, x1, y1);
			if (!addVertex(vertex, graph, settings)) return FATAL_ERROR_FAILED_TO_ADD_VERTEX;
			v2d[y1][x1] = vertex;
		}
	}
	for (int y2 = 0; y2 < 9; y2++) {
		for (int x2 = 0; x2 < 9; x2++) {
			if (y2 > 0) {
				if (!addEdge(v2d[y2][x2], v2d[y2-1][x2], 1, graph, settings)){
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (y2 < 8) {
				if (!addEdge(v2d[y2][x2], v2d[y2+1][x2], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 > 0) {
				if(!addEdge(v2d[y2][x2], v2d[y2][x2-1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
			if (x2 < 8) {
				if (!addEdge(v2d[y2][x2], v2d[y2][x2+1], 1, graph, settings)) {
					return FATAL_ERROR_FAILED_TO_ADD_EDGE;
				}
			}
		}
	}

	ifstream file(settings.FilePath.c_str());
	if (file.fail()) {
		cout << "Error: failed to open file.\n";
		return FATAL_ERROR_FILE_OPEN_FAILURE;
	}
	string line;
	int y3 = 0;
	while(y3 < 9 && std::getline(file, line)){
		if (line.length() < 9) {
			cout << "Error: line is shorter than 9 charecters.\n";
			exit(FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE);
		}
		for (int x3 = 0; x3 < 9; x3++) {
			switch (line[x3]) {
			case '+':
				removeVertex(&v2d[y3][x3], graph);
				break;
			case '-':
				break;
			case 0x13:
			case 0x10:
				cout << "Error: line " << y3 << " is shorter than 9 characters\n";
				exit(FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE);
			default:
				cout << "Error: line "<< y3 << " contains illegal character " << line[x3] << "(" << int(line[x3]) << ")\n";
				exit(FATAL_ERROR_2D_FILE_CONTAINS_SHORT_LINE);
			}
		}
		y3++;
	}
	if (y3 < 9) {
		cout << "Error: file is shorter than needed.\n";
		return FATAL_ERROR_2D_FILE_IS_SHORT;
	}
	return 0;
}

void saveGraph(Graph& graph, const Settings& settings) {
	rapidjson::Document doc;
	doc.SetArray();
	for (auto &pair: graph) {
		rapidjson::Value vert(rapidjson::kObjectType);
		rapidjson::Value name(rapidjson::kStringType);
		rapidjson::Value edges(rapidjson::kArrayType);
		for (auto &edge: *pair.second->OutcomingEdges)
		{
			rapidjson::Value edge_value(rapidjson::kObjectType);
			rapidjson::Value to_vertex_value(rapidjson::kStringType);
			to_vertex_value.SetString(edge->ToVertex->Name.c_str(), edge->ToVertex->Name.length(), doc.GetAllocator());
			rapidjson::Value weight_value(edge->Weight);
			edge_value.AddMember ("to_vertex_value", to_vertex_value, doc.GetAllocator());
			edge_value.AddMember ("weight_value", weight_value, doc.GetAllocator());
			edges.PushBack(edge_value, doc.GetAllocator());
		}
		name.SetString(pair.second->Name.c_str(), pair.second->Name.length(), doc.GetAllocator());
		vert.AddMember("name", name, doc.GetAllocator());
		vert.AddMember("edges", edges, doc.GetAllocator());
		doc.PushBack(vert, doc.GetAllocator());
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<> > writer(buffer);
	doc.Accept(writer);

	FILE *fd = fopen(settings.FilePath.c_str(), "w");
	fputs(buffer.GetString(), fd);
	fclose(fd);
}


