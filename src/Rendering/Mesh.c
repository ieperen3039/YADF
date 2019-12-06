//
// Created by ieperen3039 on 28-11-19.
//

#include "Mesh.h"
#include "Shader.h"

#define VERTEX_LOCATION 0
#define NORMAL_LOCATION 1

struct _Mesh {
    GLuint VAO_ID;
    GLuint num_elements;
    GLuint VBOIndices[2]; // [position, normal]
};

typedef struct {
    int vert[3];
    int norm[3];
} Face;

float getNextFloat(const char* space, char** line_tok) {
    return strtof(strtok_r(NULL, space, line_tok), NULL);
}

void readVector(int vectorNumber, List* sourceList, float* targetArray, int index) {
    Vector3f* vertex = list_get(sourceList, index);
    int offset = vectorNumber * 3;
    targetArray[offset] = vertex->x;
    targetArray[offset + 1] = vertex->y;
    targetArray[offset + 2] = vertex->z;
}

void readFaceVertex(Face* face, List* posList, int faceNumber, float* posArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, posList, posArr, face->vert[i]);
    }
}

void readFaceNormals(Face* face, List* normList, int faceNumber, float* normArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, normList, normArr, face->norm[i]);
    }
}

void createVBO(GLuint vboId, float* data, int att_index, int dimensions, int num_vertices) {
    GLsizeiptr i_size = num_vertices * dimensions * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, i_size, data, GL_STATIC_DRAW);
    glVertexAttribPointer(att_index, dimensions, GL_FLOAT, false, 0, 0);
}

Mesh* mesh_from_arrays(float* normals, float* positions, int num_vertices) {
    Mesh* mesh = malloc(sizeof(Mesh));
    glGenVertexArrays(1, &mesh->VAO_ID);
    glBindVertexArray(mesh->VAO_ID);
    glGenBuffers(2, mesh->VBOIndices);

    // Position VBO
    createVBO(mesh->VBOIndices[0], positions, VERTEX_LOCATION, 3, num_vertices);

    // Vertex normals VBO
    createVBO(mesh->VBOIndices[1], normals, NORMAL_LOCATION, 3, num_vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mesh->num_elements = num_vertices;

    return mesh;
}

Mesh* mesh_from_file(const char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (!file) {
        LOG_ERROR_F("File not found: %s", file_name);
        return NULL;
    }

    List* vertices = list_new(sizeof(Vector3f), 16);
    List* normals = list_new(sizeof(Vector3f), 16);
    List* faces = list_new(sizeof(Face), 16);
//    List* colors = list_new(sizeof(Vector3f), 16);

    // write to lists
    char line[128];
    const char* space = " ";
    const char* slashes = "//";
    char* line_tok;

    while (fgets(line, 128, file) != NULL) {
        char* type = strtok_r(line, space, &line_tok);
        if (strcmp(type, "v") == 0) {
            Vector3f v = {
                    getNextFloat(space, &line_tok),
                    getNextFloat(space, &line_tok),
                    getNextFloat(space, &line_tok)
            };
            list_add(vertices, &v);

        } else if (strcmp(type, "vn") == 0) {
            Vector3f v = {
                    getNextFloat(space, &line_tok),
                    getNextFloat(space, &line_tok),
                    getNextFloat(space, &line_tok)
            };
            list_add(normals, &v);

        } else if (strcmp(type, "f") == 0) {
            char* triangle_tok;
            char* vec_str = strtok_r(NULL, space, &line_tok);
            // each read index -1, as they are 1-indexed
            // read first vertex
            int base_vec = (int) strtol(strtok_r(vec_str, slashes, &triangle_tok), NULL, 10) - 1;
            int base_norm = (int) strtol(strtok_r(NULL, slashes, &triangle_tok), NULL, 10) - 1;

            // read second vertex
            vec_str = strtok_r(NULL, space, &line_tok);
            int last_vec = (int) strtol(strtok_r(vec_str, slashes, &triangle_tok), NULL, 10) - 1;
            int last_norm = (int) strtol(strtok_r(NULL, slashes, &triangle_tok), NULL, 10) - 1;

            // read any next vertex; use triangle-fan creation to allow any polygon
            while ((vec_str = strtok_r(NULL, space, &line_tok)) != NULL) {
                int new_vec = (int) strtol(strtok_r(vec_str, slashes, &triangle_tok), NULL, 10) - 1;
                int new_norm = (int) strtol(strtok_r(NULL, slashes, &triangle_tok), NULL, 10) - 1;

                Face f = {
                        {base_vec,  last_vec,  new_vec},
                        {base_norm, last_norm, new_norm}
                };
                list_add(faces, &f);

                last_vec = new_vec;
                last_norm = new_norm;
            }
        }
    }

    // Create position array in the order it has been declared. faces have 3 vertices.
    int num_vertices = list_get_size(faces) * 3;

    float posArr[num_vertices * 3];
    float normArr[num_vertices * 3];

    // write to GPU
    for (int i = 0; i < list_get_size(faces); ++i) {
        Face* face = list_get_unsafe(faces, i);
        readFaceVertex(face, vertices, i, posArr);
        readFaceNormals(face, normals, i, normArr);
    }

    Mesh* mesh = mesh_from_arrays(normArr, posArr, num_vertices);

    return mesh;
}

void mesh_render(Mesh* mesh, Matrix4f transform, ShaderID shader) {
    // write transform to gl
    glGetUniformLocation(shader, uniformName)
    glUniformMatrix4fv(unif(uniformName), false, fb);

    glBindVertexArray(mesh->VAO_ID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, mesh->num_elements);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);
}

void mesh_free(Mesh* mesh) {
    glDisableVertexAttribArray(0);
    // Delete the VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteBuffers(2, mesh->VBOIndices);

    // Delete the VAO
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mesh->VAO_ID);
    free(mesh);
}
