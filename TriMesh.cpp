#include "include/TriMesh.h"
#include <fstream>
#include <sstream>
#include <algorithm> // 为了使用 std::min 和 std::max

// 一些基础颜色
const glm::vec3 basic_colors[8] = {
	glm::vec3(1.0, 1.0, 1.0),	// White
	glm::vec3(1.0, 1.0, 0.0),	// Yellow
	glm::vec3(0.0, 1.0, 0.0),	// Green
	glm::vec3(0.0, 1.0, 1.0),	// Cyan
	glm::vec3(1.0, 0.0, 1.0),	// Magenta
	glm::vec3(1.0, 0.0, 0.0),	// Red
	glm::vec3(0.0, 0.0, 0.0),	// Black
	glm::vec3(0.0, 0.0, 1.0)	// Blue
};

// 立方体的各个点
const glm::vec3 cube_vertices[8] = {
	glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, -0.5, -0.5),
	glm::vec3(-0.5,  0.5, -0.5), glm::vec3(0.5,  0.5, -0.5),
	glm::vec3(-0.5, -0.5,  0.5), glm::vec3(0.5, -0.5,  0.5),
	glm::vec3(-0.5,  0.5,  0.5), glm::vec3(0.5,  0.5,  0.5)
};


TriMesh::TriMesh() {}
TriMesh::~TriMesh() {}

std::vector<glm::vec3> TriMesh::getVertexPositions() { return vertex_positions; }
std::vector<glm::vec3> TriMesh::getVertexColors() { return vertex_colors; }
std::vector<vec3i> TriMesh::getFaces() { return faces; }
std::vector<glm::vec3> TriMesh::getPoints() { return points; }
std::vector<glm::vec3> TriMesh::getColors() { return colors; }

void TriMesh::cleanData() {
	vertex_positions.clear();
	vertex_colors.clear();	
	faces.clear();
	points.clear();
	colors.clear();
}

void TriMesh::storeFacesPoints() {
    points.clear();
    colors.clear();
    for (auto& f : faces) {
        points.push_back(vertex_positions[f.x]);
        points.push_back(vertex_positions[f.y]);
        points.push_back(vertex_positions[f.z]);
        if (vertex_colors.size() > 0) {
            colors.push_back(vertex_colors[f.x]);
            colors.push_back(vertex_colors[f.y]);
            colors.push_back(vertex_colors[f.z]);
        }
    }
}

void TriMesh::generateCube() {
	cleanData();
    for (int i = 0; i < 8; i++) {
        vertex_positions.push_back(cube_vertices[i]);
        vertex_colors.push_back(basic_colors[i]);
    }
    int cube_faces[12][3] = {
        {0, 1, 2}, {1, 3, 2}, {4, 6, 5}, {5, 6, 7}, {0, 2, 4}, {4, 2, 6},
        {1, 5, 3}, {5, 7, 3}, {0, 4, 1}, {1, 4, 5}, {2, 3, 6}, {3, 7, 6}
    };
    for (int i = 0; i < 12; i++) {
        faces.push_back(vec3i(cube_faces[i][0], cube_faces[i][1], cube_faces[i][2]));
    }
	storeFacesPoints();
}

void TriMesh::setColor(const glm::vec3& color) {
    vertex_colors.clear();
    for (size_t i = 0; i < vertex_positions.size(); ++i) {
        vertex_colors.push_back(color);
    }
}

void TriMesh::setGradientColor(const glm::vec3& bottom_color, const glm::vec3& top_color) {
    if (vertex_positions.empty()) { return; }
    float minY = vertex_positions[0].y;
    float maxY = vertex_positions[0].y;
    for (const auto& vertex : vertex_positions) {
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.y > maxY) maxY = vertex.y;
    }
    vertex_colors.clear();
    float height = maxY - minY;
    for (const auto& vertex : vertex_positions) {
        float t = (height > 0.0001f) ? (vertex.y - minY) / height : 0.5f;
        vertex_colors.push_back(glm::mix(bottom_color, top_color, t));
    }
}

void TriMesh::setPositionalColor() {
    if (vertex_positions.empty()) {
        return;
    }

    glm::vec3 minPos = vertex_positions[0];
    glm::vec3 maxPos = vertex_positions[0];
    for (const auto& vertex : vertex_positions) {
        minPos.x = std::min(minPos.x, vertex.x);
        minPos.y = std::min(minPos.y, vertex.y);
        minPos.z = std::min(minPos.z, vertex.z);
        maxPos.x = std::max(maxPos.x, vertex.x);
        maxPos.y = std::max(maxPos.y, vertex.y);
        maxPos.z = std::max(maxPos.z, vertex.z);
    }

    glm::vec3 size = maxPos - minPos;
    vertex_colors.clear();

    for (const auto& vertex : vertex_positions) {
        float red   = (size.x > 0.0001f) ? (vertex.x - minPos.x) / size.x : 0.5f;
        float green = (size.y > 0.0001f) ? (vertex.y - minPos.y) / size.y : 0.5f;
        float blue  = (size.z > 0.0001f) ? (vertex.z - minPos.z) / size.z : 0.5f;
        
        vertex_colors.push_back(glm::vec3(red, green, blue));
    }
}

bool TriMesh::readOff(const std::string& filename)
{
    if (filename.empty()) { return false; }
    std::ifstream fin(filename);
    if (!fin) {
        printf("File on error\n");
        return false;
    }
    
    printf("File open success\n");
    cleanData();
    int nVertices, nFaces, nEdges;
    std::string str;
    fin >> str;
    if (str != "OFF") {
        printf("Not a valid OFF file\n");
        return false;
    }
    fin >> nVertices >> nFaces >> nEdges;
    for (int i = 0; i < nVertices; i++) {
        glm::vec3 tmp_node;
        fin >> tmp_node.x >> tmp_node.y >> tmp_node.z;
        vertex_positions.push_back(tmp_node);
    }
    for (int i = 0; i < nFaces; i++) {
        int num, a, b, c;
        fin >> num >> a >> b >> c;
        faces.push_back(vec3i(a, b, c));
    }
    fin.close();
    return true;
};