#ifndef GIRIGARA_RENDERER_QUAD_MESH_H
#define GIRIGARA_RENDERER_QUAD_MESH_H

#include <glad/glad.h>

class QuadMesh {
public:
    QuadMesh();
    ~QuadMesh();

    // 禁用拷貝（避免重複釋放顯存資源）
    QuadMesh(const QuadMesh&) = delete;
    QuadMesh& operator=(const QuadMesh&) = delete;

    // 支援移動語意
    QuadMesh(QuadMesh&& other) noexcept;
    QuadMesh& operator=(QuadMesh&& other) noexcept;

    // 綁定 VAO 並發出 6 個頂點的繪製調用
    void draw() const;

    GLuint getVAO() const { return m_vao; }
    GLuint getVBO() const { return m_vbo; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    void release();
};

#endif // GIRIGARA_RENDERER_QUAD_MESH_H
