//
// Created by ieperen3039 on 28-11-19.
//

#include "Mesh.h"
#include "Shader.h"
#include "../Tools.h"
#include <stdlib.h>

#define VERTEX_LOCATION 0
#define NORMAL_LOCATION 1
#define MATERIAL_LOCATION 2

struct _Mesh {
    GLuint VAO_ID;
    GLuint num_elements;
    GLuint VBOIndices[3]; // [position, normal]
};

typedef struct {
    int vert[3];
    int norm[3];
} Face;

static inline float getNextFloat(const char* space, char** line_tok) {
    return strtof(strtok_r(NULL, space, line_tok), NULL);
}

static inline void readVector(int vectorNumber, List* sourceList, float* targetArray, int index) {
    Vector3f* vertex = list_get_checked(sourceList, index);
    int offset = vectorNumber * 3;
    targetArray[offset] = vertex->x;
    targetArray[offset + 1] = vertex->y;
    targetArray[offset + 2] = vertex->z;
}

static inline void readFaceVertex(Face* face, List* posList, int faceNumber, float* posArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, posList, posArr, face->vert[i]);
    }
}

static inline void readFaceNormals(Face* face, List* normList, int faceNumber, float* normArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, normList, normArr, face->norm[i]);
    }
}

void createVBO(GLuint vboId, void* data, int data_size, int att_index, int dimensions, int num_vertices) {
    GLsizeiptr i_size = num_vertices * dimensions * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, i_size, data, GL_STATIC_DRAW);
    glVertexAttribPointer(att_index, dimensions, data_size, false, 0, 0);
}

Mesh* mesh_from_arrays(float* normals, float* positions, int* materials, int num_vertices) {
    Mesh* mesh = malloc(sizeof(Mesh));
    glGenVertexArrays(1, &mesh->VAO_ID);
    glBindVertexArray(mesh->VAO_ID);
    glGenBuffers(3, mesh->VBOIndices);

    // Position VBO
    createVBO(mesh->VBOIndices[0], positions, GL_FLOAT, VERTEX_LOCATION, 3, num_vertices);

    // Vertex normals VBO
    createVBO(mesh->VBOIndices[1], normals, GL_FLOAT, NORMAL_LOCATION, 3, num_vertices);

    // Material index VBO
    createVBO(mesh->VBOIndices[2], materials, GL_INT, MATERIAL_LOCATION, 1, num_vertices);

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
    List* colors = list_new(sizeof(Vector3f), 16);

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
    int matArr[num_vertices];
    memset(matArr, 0, sizeof(matArr));

    // write to arrays
    for (int i = 0; i < list_get_size(faces); ++i) {
        Face* face = list_get(faces, i);
        readFaceVertex(face, vertices, i, posArr);
        readFaceNormals(face, normals, i, normArr);
    }

    return mesh_from_arrays(normArr, posArr, NULL, num_vertices);
}

void mesh_render(Mesh* mesh) {
    glBindVertexArray(mesh->VAO_ID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glDrawArrays(GL_TRIANGLES, 0, mesh->num_elements);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0);
}

void mesh_free(Mesh* mesh) {
    // Delete the VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteBuffers(3, mesh->VBOIndices);

    // Delete the VAO
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mesh->VAO_ID);
    free(mesh);
}
