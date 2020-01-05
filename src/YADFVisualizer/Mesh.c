//
// Created by ieperen3039 on 28-11-19.
//

#include "Mesh.h"
#include "../YADFEngine/Tools.h"
#include "../YADFEngine/DataStructures/List.h"
#include "../YADFEngine/DataStructures/Structs.h"
#include <stdlib.h>

#define VERTEX_LOCATION 0
#define NORMAL_LOCATION 1
#define MATERIAL_LOCATION 2

struct _Mesh {
    GLuint VAO_ID;
    GLuint VBOIndex; // [position, normal, material]
    GLuint num_elements;
};

typedef struct {
    int vert[3];
    int norm[3];
} OBJFace;

typedef struct {
    float pos[3];
    float norm[3];
    int mat;
} PLYVertex;

typedef struct {
    int verts;
    int norms;
    int mat;
} PLYFace;

static inline float getNextFloat(const char* splitter, char** line_tok) {
    return strtof(tool_split(NULL, splitter, line_tok, false), NULL);
}

static inline int getNextInt(const char* splitter, char** line_tok) {
    return strtol(tool_split(NULL, splitter, line_tok, false), NULL, 10);
}

static inline void readVector(int vectorNumber, List* sourceList, float* targetArray, int index) {
    Vector3f* vertex = list_get_checked(sourceList, index);
    int offset = vectorNumber * 3;
    targetArray[offset] = vertex->x;
    targetArray[offset + 1] = vertex->y;
    targetArray[offset + 2] = vertex->z;
}

static inline void readFaceVertex(OBJFace* face, List* posList, int faceNumber, float* posArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, posList, posArr, face->vert[i]);
    }
}

static inline void readFaceNormals(OBJFace* face, List* normList, int faceNumber, float* normArr) {
    int vectorIndex = faceNumber * 3;
    for (int i = 0; i < 3; i++) {
        readVector(vectorIndex + i, normList, normArr, face->norm[i]);
    }
}

Mesh* mesh_from_arrays(float* positions, float* normals, int* materials, int num_vertices) {
    Mesh* mesh = malloc(sizeof(Mesh));
    glGenVertexArrays(1, &mesh->VAO_ID);
    glBindVertexArray(mesh->VAO_ID);

    glGenBuffers(1, &mesh->VBOIndex);

    GLsizeiptr p_size = num_vertices * 3 * sizeof(float);
    GLsizeiptr n_size = num_vertices * 3 * sizeof(float);
    GLsizeiptr m_size = num_vertices * 1 * sizeof(int);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBOIndex);
    // allocate buffer
    glBufferData(GL_ARRAY_BUFFER, p_size + n_size + m_size, NULL, GL_STATIC_DRAW);

    // Position
    glBufferSubData(GL_ARRAY_BUFFER, 0, p_size, positions);
    glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, false, 0, 0);

    // Vertex normals
    glBufferSubData(GL_ARRAY_BUFFER, p_size, n_size, normals);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, false, 0, (void*) p_size);

    // Material index
    glBufferSubData(GL_ARRAY_BUFFER, p_size + n_size, m_size, materials);
    glVertexAttribIPointer(MATERIAL_LOCATION, 1, GL_INT, 0, (void*) p_size + n_size);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mesh->num_elements = num_vertices;

    return mesh;
}

Mesh* mesh_from_ply(const char* file_name, ElementMap* color_map) {
    List buffer[1];

    FILE* file = fopen(file_name, "r");
    if (!file) {
        LOG_ERROR_F("File not found: %s", file_name);
        return NULL;
    }

    char line[128];
    const char* space = " ";
    char* line_tok;

    int num_vertices = -1;
    int num_faces = -1;

    // header
    while (fgets(line, 128, file) != NULL && strcmp(line, "end_header") != 0) {
        if (line[0] == '#') continue;
        char* type = tool_split(line, space, &line_tok, true);

        if (strcmp(type, "element") == 0) {
            char* element = tool_split(NULL, space, &line_tok, true);
            if (strcmp(element, "vertex") == 0) {
                assert(num_vertices == -1);
                num_vertices = getNextInt(space, &line_tok);

            } else if (strcmp(element, "face") == 0) {
                assert(num_faces == -1);
                num_faces = getNextInt(space, &line_tok);

            } else {
                LOG_ERROR_F("unknown PLY element %s", element);
                return NULL;
            }
        }
        break;
    }

    PLYVertex vertices[num_vertices];

    for (int i = 0; i < num_vertices; ++i) {
        fgets(line, 128, file);

        vertices[i] = (PLYVertex) {
                .pos = {
                        getNextFloat(space, &line_tok),
                        getNextFloat(space, &line_tok),
                        getNextFloat(space, &line_tok),
                },
                .norm = {
                        getNextFloat(space, &line_tok),
                        getNextFloat(space, &line_tok),
                        getNextFloat(space, &line_tok),
                },
                .mat = 0
        };

        // determine material index
        int color[3] =  {
                getNextInt(space, &line_tok),
                getNextInt(space, &line_tok),
                getNextInt(space, &line_tok),
        };
        int color_key = color[0] + (color[1] << 8) + (color[2] << 16);
        int* mat_ind = map_get(color_map, color_key);
        if (mat_ind) vertices[i].mat = *mat_ind;
    }

    List* faces = list_init(&buffer[0], sizeof(PLYFace), num_faces);

    for (int i = 0; i < num_faces; ++i) {
        fgets(line, 128, file);
        // 0-indexed face lists
        int face_nr_of_vec = getNextInt(space, &line_tok);
        int base_vec = getNextInt(space, &line_tok);
        int last_vec = getNextInt(space, &line_tok);

        // read any next vertex; use triangle-fan creation to allow any polygon
        for (int j = 2; j < face_nr_of_vec; ++j) {
            int new_vec = getNextInt(space, &line_tok);
            PLYFace face = {base_vec, last_vec, new_vec};
            list_add(faces, &face);
            last_vec = new_vec;
        }
    }

    fclose(file);

    num_faces = list_get_size(faces);
    float posArr[num_faces * 3];
    float normArr[num_faces * 3];
    int matArr[num_faces];

    for (int i = 0; i < num_faces; ++i) {
        PLYFace* face = list_get(faces, i);
        PLYVertex v = vertices[face->verts];

        memcpy(&posArr[3 * i], &v.pos, 3 * sizeof(float));
        memcpy(&normArr[3 * i], &v.norm, 3 * sizeof(float));
        matArr[i] = v.mat;
    }

    list_free(faces);

    return mesh_from_arrays(posArr, normArr, matArr, num_faces);
}

struct VecNormPair {
    int vec;
    int norm;
};

struct VecNormPair read_vec_tex_norm_string(char* vec_str, const char* slashes) {
    char* token;
    // read first number
    char* vec_s = tool_split(vec_str, slashes, &token, false);
    int vec = (int) strtol(vec_s, NULL, 10) - 1;
    // skip texture
    char* tex_s = tool_split(NULL, slashes, &token, false);
    // read 3rd number
    char* norm_s = tool_split(NULL, slashes, &token, false);
    int norm = (int) strtol(norm_s, NULL, 10) - 1;

    return (struct VecNormPair){vec, norm};
}

Mesh* mesh_from_obj(const char* filename, int material_id) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        LOG_ERROR_F("Could not open file '%s': %s", filename, strerror(errno));
        return NULL;
    }

    List buffer[3];
    List* vertices = list_init(&buffer[0], sizeof(Vector3f), 16);
    List* normals = list_init(&buffer[1], sizeof(Vector3f), 16);
    List* faces = list_init(&buffer[2], sizeof(OBJFace), 16);

    // write to lists
    char line[128];
    const char* space = " ";
    const char* slash = "/";
    char* line_tok;

    while (fgets(line, 128, file) != NULL) {
        if (line[0] == '#') continue;

        char* type = tool_split(line, space, &line_tok, true);
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
            char* vec_str = tool_split(NULL, space, &line_tok, true);
            // each read index -1, as they are 1-indexed
            // read first vertex
            struct VecNormPair base = read_vec_tex_norm_string(vec_str, slash);

            // read second vertex
            vec_str = tool_split(NULL, space, &line_tok, true);
            struct VecNormPair last = read_vec_tex_norm_string(vec_str, slash);

            // read any next vertex; use triangle-fan creation to allow any polygon
            while ((vec_str = tool_split(NULL, space, &line_tok, true)) != NULL) {
                struct VecNormPair new = read_vec_tex_norm_string(vec_str, slash);

                OBJFace f = {
                        {base.vec,  last.vec,  new.vec},
                        {base.norm, last.norm, new.norm}
                };
                list_add(faces, &f);

                last = new;
            }
        }
    }

    fclose(file);

    // Create position array in the order it has been declared. faces have 3 vertices.
    int num_vertices = list_get_size(faces) * 3;

    float posArr[num_vertices * 3];
    float normArr[num_vertices * 3];
    int matArr[num_vertices];
    memset(matArr, material_id, sizeof(matArr));

    // write to arrays
    for (int i = 0; i < list_get_size(faces); ++i) {
        OBJFace* face = list_get(faces, i);
        readFaceVertex(face, vertices, i, posArr);
        readFaceNormals(face, normals, i, normArr);
    }

    list_free(vertices);
    list_free(normals);
    list_free(faces);

    return mesh_from_arrays(posArr, normArr, matArr, num_vertices);
}

void mesh_render(Mesh* mesh) {
    glBindVertexArray(mesh->VAO_ID);
    glEnableVertexAttribArray(VERTEX_LOCATION);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glEnableVertexAttribArray(MATERIAL_LOCATION);

    glDrawArrays(GL_TRIANGLES, 0, mesh->num_elements);

    glDisableVertexAttribArray(VERTEX_LOCATION);
    glDisableVertexAttribArray(NORMAL_LOCATION);
    glDisableVertexAttribArray(MATERIAL_LOCATION);
    glBindVertexArray(0);
}

void mesh_free(Mesh* mesh) {
    // Delete the VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &mesh->VBOIndex);

    // Delete the VAO
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mesh->VAO_ID);
    free(mesh);
}
